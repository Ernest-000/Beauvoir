#!/usr/bin/env bash
set -e

# Configuration
BVR_GENERATOR="Unix Makefiles"
BVR_CC="gcc"
BVR_CXX="g++"

BVR_ROOT_DIR="$(pwd)"
BVR_BUILD_DIR="$BVR_ROOT_DIR/build"
BVR_INCLUDE_DIR="$BVR_ROOT_DIR/include"
BVR_NUKLEAR_DIR="$BVR_ROOT_DIR/extern/Nuklear/src"

BVR_EXTERNAL_MODULES=("SDL" "Zlib" "Lpng" "json-c")

# Flags
BVR_CLEAR=false
BVR_SKIP_BIN=false
BVR_SKIP_SYNC=false

for arg in "$@"; do
    case "$arg" in
        --clear)
            BVR_CLEAR=true
            ;;
        --skip-binaries)
            BVR_SKIP_BIN=true
            ;;
        --skip-git-sync)
            BVR_SKIP_SYNC=true
            ;;  
    esac
done

# Git submodules
if [ "$BVR_SKIP_SYNC" = false ]; then
    git submodule deinit -f .
    git submodule update --init
    git submodule update --remote --merge
fi

# Clear command
if [ "$BVR_CLEAR" = false ]; then
    echo "Cleaning..."

    rm -rf "$BVR_ROOT_DIR/bin"
    rm -rf "$BVR_ROOT_DIR/lib"
    rm -rf "$BVR_ROOT_DIR/licenses"
    rm -rf "$BVR_ROOT_DIR/cmake"

    mkdir -p "$BVR_ROOT_DIR/bin"
    mkdir -p "$BVR_ROOT_DIR/lib"
fi

# Loop over external modules
if [ "$BVR_SKIP_BIN" = false ]; then
    for MOD in "${BVR_EXTERNAL_MODULES[@]}"; do
        MODULE_PATH="$BVR_ROOT_DIR/extern/$MOD"

        if [ -d "$MODULE_PATH" ]; then
            echo "$MODULE_PATH found!"

            BVR_MODULE_FLAGS=""

            rm -f "$BVR_BUILD_DIR/$MOD/CMakeCache.txt"

            cmake "$MODULE_PATH/CMakeLists.txt" \
                -G "$BVR_GENERATOR" \
                -B "$BVR_BUILD_DIR/$MOD" \
                -D CMAKE_INSTALL_PREFIX="$BVR_ROOT_DIR" \
                -D CMAKE_C_COMPILER="$BVR_CC" \
                $BVR_MODULE_FLAGS

            cmake --build "$BVR_BUILD_DIR/$MOD" --target install
        else
            echo "$MODULE_PATH not found!"
        fi
    done
fi

# Nuklear
NK_HEADER="$BVR_NUKLEAR_DIR/HEADER.md"
NK_FOOTER="$BVR_NUKLEAR_DIR/LICENSE $BVR_NUKLEAR_DIR/CHANGELOG $BVR_NUKLEAR_DIR/CREDITS"

NK_PRIV1="$BVR_NUKLEAR_DIR/nuklear_internal.h
$BVR_NUKLEAR_DIR/nuklear_math.c
$BVR_NUKLEAR_DIR/nuklear_util.c
$BVR_NUKLEAR_DIR/nuklear_color.c
$BVR_NUKLEAR_DIR/nuklear_utf8.c
$BVR_NUKLEAR_DIR/nuklear_buffer.c
$BVR_NUKLEAR_DIR/nuklear_string.c
$BVR_NUKLEAR_DIR/nuklear_draw.c
$BVR_NUKLEAR_DIR/nuklear_vertex.c"

NK_PRIV2="$BVR_NUKLEAR_DIR/nuklear_font.c
$BVR_NUKLEAR_DIR/nuklear_input.c
$BVR_NUKLEAR_DIR/nuklear_style.c
$BVR_NUKLEAR_DIR/nuklear_context.c
$BVR_NUKLEAR_DIR/nuklear_pool.c
$BVR_NUKLEAR_DIR/nuklear_page_element.c
$BVR_NUKLEAR_DIR/nuklear_table.c
$BVR_NUKLEAR_DIR/nuklear_panel.c
$BVR_NUKLEAR_DIR/nuklear_window.c
$BVR_NUKLEAR_DIR/nuklear_popup.c
$BVR_NUKLEAR_DIR/nuklear_contextual.c
$BVR_NUKLEAR_DIR/nuklear_menu.c
$BVR_NUKLEAR_DIR/nuklear_layout.c
$BVR_NUKLEAR_DIR/nuklear_tree.c
$BVR_NUKLEAR_DIR/nuklear_group.c
$BVR_NUKLEAR_DIR/nuklear_list_view.c
$BVR_NUKLEAR_DIR/nuklear_widget.c
$BVR_NUKLEAR_DIR/nuklear_text.c
$BVR_NUKLEAR_DIR/nuklear_image.c
$BVR_NUKLEAR_DIR/nuklear_9slice.c
$BVR_NUKLEAR_DIR/nuklear_button.c
$BVR_NUKLEAR_DIR/nuklear_toggle.c
$BVR_NUKLEAR_DIR/nuklear_selectable.c
$BVR_NUKLEAR_DIR/nuklear_slider.c
$BVR_NUKLEAR_DIR/nuklear_knob.c
$BVR_NUKLEAR_DIR/nuklear_progress.c
$BVR_NUKLEAR_DIR/nuklear_scrollbar.c
$BVR_NUKLEAR_DIR/nuklear_text_editor.c
$BVR_NUKLEAR_DIR/nuklear_edit.c
$BVR_NUKLEAR_DIR/nuklear_property.c
$BVR_NUKLEAR_DIR/nuklear_chart.c
$BVR_NUKLEAR_DIR/nuklear_color_picker.c
$BVR_NUKLEAR_DIR/nuklear_combo.c
$BVR_NUKLEAR_DIR/nuklear_tooltip.c"

NK_EXTERN="$BVR_NUKLEAR_DIR/stb_rect_pack.h
$BVR_NUKLEAR_DIR/stb_truetype.h"

NK_PUBLIC="$BVR_NUKLEAR_DIR/nuklear.h"

python3 "$BVR_NUKLEAR_DIR/build.py" --macro NK --intro "$NK_HEADER" --pub "$NK_PUBLIC" --priv1 "$NK_PRIV1" --extern "$NK_EXTERN" --priv2 "$NK_PRIV2" --outro "$NK_FOOTER" > "$BVR_INCLUDE_DIR/nuklear.h"

# Auto clear-up unused SDL folders
[ -d "$BVR_ROOT_DIR/licenses" ] && rm -rf "$BVR_ROOT_DIR/licenses"
[ -d "$BVR_ROOT_DIR/cmake" ] && rm -rf "$BVR_ROOT_DIR/cmake"