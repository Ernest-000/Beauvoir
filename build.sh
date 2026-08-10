#!/usr/bin/env bash
set -euo pipefail

# Avaible parameters :
#   --clear: clear all caches and building files
#   --force: force building
#   --skip-binaries: skip cmake building pass
#   --skip-git: skip git sync pass
#   --skip-nk: skip nuklear header generation

# CMake building system
BVR_GENERATOR="Unix Makefiles"
BVR_CC="gcc"
BVR_CXX="g++"
BVR_USE_CCACHE=false

# path variables
BVR_ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BVR_BUILD_DIR="/tmp/beauvoir-build"
BVR_INCLUDE_DIR="${BVR_ROOT_DIR}/include"
BVR_NUKLEAR_DIR="${BVR_ROOT_DIR}/extern/Nuklear/src"

# Third-party lib list 
BVR_EXTERNAL_MODULES=("Zlib" "json-c")

# flags
BVR_CLEAR=false
BVR_FORCE=false
BVR_SKIP_BIN=false
BVR_SKIP_SYNC=false
BVR_SKIP_NK=false

for arg in "$@"; do
    case "$arg" in
        --clear)            BVR_CLEAR=true ;;
        --force)            BVR_FORCE=true ;;
        --skip-binaries)    BVR_SKIP_BIN=true ;;
        --skip-git)         BVR_SKIP_SYNC=true ;;
        --skip-nk)          BVR_SKIP_NK=true ;;
    esac
done

# clear all cmake and user's cache
bvr_clear(){
    echo "Cleaning..."

    rm -rf "${BVR_ROOT_DIR}/bin"
    rm -rf "${BVR_ROOT_DIR}/lib"
    rm -rf "${BVR_ROOT_DIR}/licenses"
    rm -rf "${BVR_ROOT_DIR}/cmake"
    rm -rf "${BVR_BUILD_DIR}"

    mkdir -p "${BVR_ROOT_DIR}/bin"
    mkdir -p "${BVR_ROOT_DIR}/lib"

    return 0
}

# clear post build folders
bvr_post_build(){
    rm -rf "${BVR_BUILD_DIR}"
    rm -rf "${BVR_ROOT_DIR}/licenses"
    rm -rf "${BVR_ROOT_DIR}/cmake"
}

# sync and update git submodules
bvr_git_sync(){
    # maximum retries
    local submodules_max_retries=3

    echo "Syncing git submodules..."

    # check for .gitmodules
    if [ ! -f "${BVR_ROOT_DIR}/.gitmodules" ]; then
        echo "No .gitmodules file found, skipping submodule sync."
        return 0
    fi

    # keep remote URLs
    git submodule sync --recursive

    # safe deinit
    git submodule deinit --all -f

    # tries multiple times to update init all modules
    local attempt=1 
    local success=false
    while [ "$attempt" -le "$submodules_max_retries" ]; do
        echo "Initializing submodules (attempt ${attempt}/${submodules_max_retries})..."
        if git submodule update --init --recursive; then
            success=true
            break
        fi

        echo "Submodule init/update failed, retrying..." >&2
        attempt=$((attempt + 1))
        sleep 2
    done

    # print error if needed
    if [ "$success" = false ]; then
        echo "Error: could not initialize submodules after ${submodules_max_retries} attempts." >&2
        return 1
    fi

    local failed_modules=()

    # try to get latest release
    while read -r key path; do
        name="${key#submodule.}"
        name="${name%.path}"
        module_dir="${BVR_ROOT_DIR}/${path}"

        echo "Updating submodule: ${name} (${path})"

        (
            cd "${module_dir}" || exit 1
            git fetch --tags --force >/dev/null 2>&1

            latest_tag="$(git tag --list --sort=-v:refname | head -n1)"

            # find latest with the tag
            if [ -n "${latest_tag}" ]; then
                echo "Checking out latest release tag: ${latest_tag}"
                git checkout --detach "${latest_tag}"
            else
                echo "No tags found, falling back to remote branch head"
                default_branch="$(git symbolic-ref --short HEAD 2>/dev/null || echo main)"
                git checkout "${default_branch}"
                git pull --ff-only
            fi
        ) || failed_modules+=("${name}")
    done < <(git config -f "${BVR_ROOT_DIR}/.gitmodules" --get-regexp '^submodule\..*\.path$')

    # print failed modules
    if [ "${#failed_modules[@]}" -gt 0 ]; then
        echo "Warning: the following submodules failed to update: ${failed_modules[*]}" >&2
    fi

    return 0
}

bvr_build_module() {
    # max parallel jobs
    local BVR_JOBS=4

    # mod variables
    local MOD="$1"
    local MOD_PATH="${BVR_ROOT_DIR}/extern/${MOD}"
    local MOD_BUILD_DIR="${BVR_BUILD_DIR}/${MOD}"
    local MOD_TMP="${BVR_BUILD_DIR}/install"
    local MOD_HASH_FILE="${BVR_BUILD_DIR}/.bvr-mod-hash"

    # check for missing module
    [ -d "${MOD_PATH}" ] || { echo "Missing module: ${MOD_PATH}"; return 1; }

    # get version hash
    local SRC_HASH 
    SRC_HASH="$(git -C "${MOD_PATH}" rev-parse HEAD 2>/dev/null || echo "nogit")"

    # skip building if everything is up to date
    if [ -f "${MOD_HASH_FILE}" ] && [ "$(cat "${MOD_HASH_FILE}")" = "${SRC_HASH}" ] && [ "$BVR_FORCE" = false ] \
       && [ -d "${MOD_TMP}/lib" ]; then
        echo "Module ${MOD} unchanged (${SRC_HASH:0:8}), skipping rebuild."
    else
        echo "Building module ${MOD}"

        # generic flags
        local BVR_MODULE_FLAGS="-DBUILD_SHARED_LIBS=OFF -DCMAKE_PLATFORM_NO_VERSIONED_SONAME=ON -DCMAKE_POSITION_INDEPENDENT_CODE=ON"

        # module specific flags
        case "$MOD" in
            Zlib)   BVR_MODULE_FLAGS="${BVR_MODULE_FLAGS} -DZLIB_BUILD_EXAMPLES=OFF" ;;
            json-c) BVR_MODULE_FLAGS="${BVR_MODULE_FLAGS} -DBUILD_SHARED_LIBS=ON" ;;
        esac

        # enable ccache
        if [ "$BVR_USE_CCACHE" = true ]; then
            BVR_MODULE_FLAGS="${BVR_MODULE_FLAGS} -DCMAKE_C_COMPILER_LAUNCHER=ccache -DCMAKE_CXX_COMPILER_LAUNCHER=ccache"
        fi

        # use --fresh for the the first build of the module 
        local FRESH_FLAG="--fresh"
        [ -f "${MOD_BUILD_DIR}/CMakeCache.txt" ] && FRESH_FLAG=""

        cmake ${FRESH_FLAG} \
            -S "${MOD_PATH}" \
            -G "${BVR_GENERATOR}" \
            -B "${MOD_BUILD_DIR}" \
            -DCMAKE_INSTALL_PREFIX="${MOD_TMP}" \
            -DCMAKE_C_COMPILER="${BVR_CC}" \
            -DCMAKE_BUILD_TYPE=Release \
            ${BVR_MODULE_FLAGS} \
            || return 1

        cmake --build "${MOD_BUILD_DIR}" --target install --parallel "${BVR_JOBS}" \
            || return 1
    fi

    # check for lib files
    if [ -d "${MOD_TMP}/lib" ]; then
        for ext in "*.so" "*.a" "*.dylib"; do
            find "${MOD_TMP}/lib" -maxdepth 1 -name "$ext" \
                -exec cp -f {} "${BVR_ROOT_DIR}/lib/" \;
        done
    fi

    # copy include fils
    if [ -d "${MOD_TMP}/include" ]; then
        cp -r "${MOD_TMP}/include/." "${BVR_INCLUDE_DIR}/"
    fi
}

bvr_merge_nuklear(){
    local NK_HEADER="${BVR_NUKLEAR_DIR}/HEADER.md"
    local NK_FOOTER="${BVR_NUKLEAR_DIR}/LICENSE ${BVR_NUKLEAR_DIR}/CHANGELOG ${BVR_NUKLEAR_DIR}/CREDITS"
    local NK_PRIV1="${BVR_NUKLEAR_DIR}/nuklear_internal.h ${BVR_NUKLEAR_DIR}/nuklear_math.c ${BVR_NUKLEAR_DIR}/nuklear_util.c ${BVR_NUKLEAR_DIR}/nuklear_color.c ${BVR_NUKLEAR_DIR}/nuklear_utf8.c ${BVR_NUKLEAR_DIR}/nuklear_buffer.c ${BVR_NUKLEAR_DIR}/nuklear_string.c ${BVR_NUKLEAR_DIR}/nuklear_draw.c ${BVR_NUKLEAR_DIR}/nuklear_vertex.c"
    local NK_PRIV2="${BVR_NUKLEAR_DIR}/nuklear_font.c ${BVR_NUKLEAR_DIR}/nuklear_input.c ${BVR_NUKLEAR_DIR}/nuklear_style.c ${BVR_NUKLEAR_DIR}/nuklear_context.c ${BVR_NUKLEAR_DIR}/nuklear_pool.c ${BVR_NUKLEAR_DIR}/nuklear_page_element.c ${BVR_NUKLEAR_DIR}/nuklear_table.c ${BVR_NUKLEAR_DIR}/nuklear_panel.c ${BVR_NUKLEAR_DIR}/nuklear_window.c ${BVR_NUKLEAR_DIR}/nuklear_popup.c ${BVR_NUKLEAR_DIR}/nuklear_contextual.c ${BVR_NUKLEAR_DIR}/nuklear_menu.c ${BVR_NUKLEAR_DIR}/nuklear_layout.c ${BVR_NUKLEAR_DIR}/nuklear_tree.c ${BVR_NUKLEAR_DIR}/nuklear_group.c ${BVR_NUKLEAR_DIR}/nuklear_list_view.c ${BVR_NUKLEAR_DIR}/nuklear_widget.c ${BVR_NUKLEAR_DIR}/nuklear_text.c ${BVR_NUKLEAR_DIR}/nuklear_image.c ${BVR_NUKLEAR_DIR}/nuklear_9slice.c ${BVR_NUKLEAR_DIR}/nuklear_button.c ${BVR_NUKLEAR_DIR}/nuklear_toggle.c ${BVR_NUKLEAR_DIR}/nuklear_selectable.c ${BVR_NUKLEAR_DIR}/nuklear_slider.c ${BVR_NUKLEAR_DIR}/nuklear_knob.c ${BVR_NUKLEAR_DIR}/nuklear_progress.c ${BVR_NUKLEAR_DIR}/nuklear_scrollbar.c ${BVR_NUKLEAR_DIR}/nuklear_text_editor.c ${BVR_NUKLEAR_DIR}/nuklear_edit.c ${BVR_NUKLEAR_DIR}/nuklear_property.c ${BVR_NUKLEAR_DIR}/nuklear_chart.c ${BVR_NUKLEAR_DIR}/nuklear_color_picker.c ${BVR_NUKLEAR_DIR}/nuklear_combo.c ${BVR_NUKLEAR_DIR}/nuklear_tooltip.c"
    local NK_EXTERN="${BVR_NUKLEAR_DIR}/stb_rect_pack.h ${BVR_NUKLEAR_DIR}/stb_truetype.h"
    local NK_PUBLIC="${BVR_NUKLEAR_DIR}/nuklear.h"

    local NK_OUT="${BVR_INCLUDE_DIR}/nuklear.h"
    local NK_STAMP="${BVR_BUILD_DIR}/.nuklear"
    local NK_TMP
    
    # check for versioning
    local NK_HASH
    NK_HASH="$(git -C "${BVR_ROOT_DIR}/extern/Nuklear" rev-parse HEAD 2>/dev/null || echo "nogit")"

    # skip when up to date
    if [ -f "${NK_STAMP}" ] && [ "$(cat "${NK_STAMP}")" = "${NK_HASH}" ] && [ -f "${NK_OUT}" ]; then
        echo "nuklear.h up to date (${NK_HASH:0:8}), skipping regeneration."
        return 0
    fi

    echo "Generating nuklear.h..."
    
    NK_TMP="$(mktemp "${BVR_INCLUDE_DIR}/.nuklear.h.XXXXXX")"

    if python3 "${BVR_NUKLEAR_DIR}/build.py" \
        --macro NK \
        --intro  "${NK_HEADER}" \
        --pub    "${NK_PUBLIC}" \
        --priv1  "${NK_PRIV1}" \
        --extern "${NK_EXTERN}" \
        --priv2  "${NK_PRIV2}" \
        --outro  "${NK_FOOTER}" \
        > "${NK_TMP}"
    then
        mv "${NK_TMP}" "${NK_OUT}"
        echo "${NK_HASH}" > "${NK_STAMP}"
    else
        rm -f "${NK_TMP}"
        echo "Error: nuklear.h generation failed." >&2
        return 1
    fi
}

if [ "$BVR_SKIP_SYNC" = false ]; then
    bvr_git_sync || {
        echo "Submodules sync had errors"
    }
fi

if [ "$BVR_CLEAR" = true ]; then
    bvr_clear ||  {
        echo "clearing stage had errors"
    }
fi

if [ "$BVR_SKIP_BIN" = false ]; then
    BVR_MODULE_PIDS=()
    BVR_MODULE_LOGS=()

    # loop through all modules
    for MOD in "${BVR_EXTERNAL_MODULES[@]}"; do
        LOG="${BVR_BUILD_DIR}/${MOD}.log"
        mkdir -p "${BVR_BUILD_DIR}"
        ( bvr_build_module "$MOD" > "${LOG}" 2>&1 ) &
        BVR_MODULE_PIDS+=($!)
        BVR_MODULE_LOGS+=("${MOD}:${LOG}")
    done

    BVR_MODULE_FAILED=false
    for i in "${!BVR_MODULE_PIDS[@]}"; do
        if ! wait "${BVR_MODULE_PIDS[$i]}"; then
            echo "Error: build failed for ${BVR_MODULE_LOGS[$i]}" >&2
            BVR_MODULE_FAILED=true
        fi
    done

    # print logs when failed
    for entry in "${BVR_MODULE_LOGS[@]}"; do
        echo "----- ${entry} -----"
        cat "${entry#*:}"
    done

    if [ "$BVR_MODULE_FAILED" = true ]; then
        echo "One or more external modules failed to build." >&2
        exit 1
    fi
fi

if [ "$BVR_SKIP_BIN" = false ]; then
    bvr_merge_nuklear || {
        echo "Nuklear header gen had errors"
    }
fi

bvr_post_build