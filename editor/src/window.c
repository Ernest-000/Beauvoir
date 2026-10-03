#include <bergson/window.h>

#include <gtk/gtk.h>

#define BGS_FILE_WLAYOUT "xml/wlayout.xml"

#define BGS_WLAYOUT "wlayout"
#define BGS_WVIEWPORT "wviewport"
#define BGS_ULAYOUT "ulayout"
#define BGS_CLAYOUT "clayout"
#define BGS_WMENU "wmenu"

static inline void bgs_builder_add(GtkBuilder* builder, const char* file){
    GError* error = NULL;
    if (!gtk_builder_add_from_file(builder, file, &error)) {
        g_printerr("Erreur chargement UI: %s\n", error->message);
        g_error_free(error);
    }
}

static void bgs_create_window_impl(GtkApplication* self, gpointer _window){
    bgs_window_t* window = (bgs_window_t*)_window;
    GtkBuilder* builder = NULL;
    BVR_ASSERT(window);

    window->window = gtk_window_new();
    gtk_window_set_application(GTK_WINDOW(window->window), GTK_APPLICATION(window->handle));
    gtk_window_set_title(GTK_WINDOW(window->window), window->name.string);
    gtk_window_set_default_size(
        GTK_WINDOW(window->window),
        window->width, window->height
    );

    builder = gtk_builder_new();
    
    // add files
    bgs_builder_add(builder, BGS_FILE_WLAYOUT);
    
    // link layout
    gtk_window_set_child(
        GTK_WINDOW(window->window), 
        GTK_WIDGET(gtk_builder_get_object(builder, BGS_WLAYOUT))
    );

    // link viewport
    gtk_box_append(
        GTK_BOX(gtk_builder_get_object(builder, BGS_WVIEWPORT)),
        GTK_WIDGET(bgs_create_viewport(&window->components.viewport, window))
    );

    // link menu
    gtk_box_append(
        GTK_BOX(gtk_builder_get_object(builder, BGS_WMENU)),
        GTK_WIDGET(bgs_create_menuitems(&window->components.menu, window))
    );

    g_object_unref(GTK_BUILDER(builder));
    
    gtk_window_present(GTK_WINDOW(window->window));
}
 
int bgs_create_window(bgs_window_t* window, const char* name, const uint16 width, const uint16 height, int flags){
    BVR_ASSERT(window);
    BVR_ASSERT(name);

    window->handle = NULL;
    window->window = NULL;
    window->gl = NULL;
    window->width = width;
    window->height = height;
    window->flags = flags;

    bvr_create_string(&window->name, name);

    // create a new instance
    window->handle = gtk_application_new(
        BGS_CLASS, G_APPLICATION_DEFAULT_FLAGS
    );
    BVR_ASSERT(window->handle);

    // create callback
    g_signal_connect(
        G_APPLICATION(window->handle), "activate", 
        G_CALLBACK(bgs_create_window_impl), window
    );

    // run
    return g_application_run(
        G_APPLICATION(window->handle), 0, NULL
    );
}

int bgs_destroy_window(bgs_window_t* window){
    BVR_ASSERT(window);

    bgs_destroy_viewport(&window->components.viewport);
    bgs_destroy_menuitems(&window->components.menu);

    gtk_window_destroy(GTK_WINDOW(window->window));
    g_object_unref(G_APPLICATION(window->handle));
}