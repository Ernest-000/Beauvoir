#include <bergson/window.hpp>
#include <bergson/config.hpp>
#include <bergson/menu.hpp>
#include <bergson/viewport.hpp>
#include <bergson/hierarchy.hpp>
#include <bergson/inspector.hpp>

#define BGS_FILE_WLAYOUT "xml/wlayout.xml"
#define BGS_FILE_CSS "xml/style.css"

#define BGS_WLAYOUT "wlayout"
#define BGS_WVIEWPORT "wviewport"
#define BGS_ULAYOUT "ulayout"
#define BGS_CLAYOUT "clayout"
#define BGS_WHIERARCHY "whierarchy"
#define BGS_WINSPECTOR "winspector"
#define BGS_WMENU "wmenu"

using namespace Bergson;

Window::Window(const char* title, const uint16 width, const uint16 height){

    // overwrite current instance
    Window::sInstance = this;
    
    mWidth = width;
    mHeight = height;

    bvr_create_string(&mTitle, title);
    
    set_title(title);
    set_default_size(width, height);

    // create css
    mCss = Gtk::CssProvider::create();
    mCss->load_from_path(BGS_FILE_CSS);

    // load css
    get_style_context()->add_provider(mCss, GTK_STYLE_PROVIDER_PRIORITY_USER);

    // create builder
    mBuilder = Gtk::Builder::create();
    BVR_ASSERT(BuilderAddNewFile(BGS_FILE_WLAYOUT));

    // apply layout
    mLayout = mBuilder->get_object<Widget>(BGS_WLAYOUT);
    BVR_ASSERT(mLayout);
    
    // add default layout
    set_child(*((Widget*)mLayout.get()));

    // create the menu layout
    mMenuList = Gtk::make_managed<MenuLayout>();
    mViewport = Gtk::make_managed<Viewport>();
    mHierarchy = Gtk::make_managed<Hierarchy>();
    mInspector = Gtk::make_managed<Inspector>();
    // ((Hierarchy*)mHierarchy)->SyncWithScene(((Viewport*)mViewport)->GetCurrentPage());

    // add the menu bar
    Gtk::Box* menuContainer = mBuilder->get_widget<Gtk::Box>(BGS_WMENU);
    BVR_ASSERT(menuContainer);
    menuContainer->append(*mMenuList);

    // add the viewport
    Gtk::Box* viewportContainer = mBuilder->get_widget<Gtk::Box>(BGS_WVIEWPORT);
    BVR_ASSERT(viewportContainer);
    viewportContainer->append(*mViewport);
    viewportContainer->set_hexpand(true);
    viewportContainer->set_vexpand(true);

    // default size?
    viewportContainer->set_size_request(500, 500);

    Gtk::Box* hierarchyContainer = mBuilder->get_widget<Gtk::Box>(BGS_WHIERARCHY);
    BVR_ASSERT(hierarchyContainer);
    hierarchyContainer->append(*mHierarchy);

    Gtk::Box* inspectorContainer = mBuilder->get_widget<Gtk::Box>(BGS_WINSPECTOR);
    BVR_ASSERT(inspectorContainer);
    inspectorContainer->append(*mInspector);
}

Window::~Window(){
    // no op
}

bool Window::BuilderAddNewFile(const char* file){
    BVR_FILE_EXISTS(file);

    if(!mBuilder->add_from_file(file)){
        BVR_PRINTF("failed to add a new file from '%s' to the builder.", file);
        return false;
    }
    
    return true;
}
