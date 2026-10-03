#include <bergson/menu.h>
#include <bergson/viewport.h>

#include <gtk/gtk.h>

static void bgsi_open_scene_read(GObject* src, GAsyncResult* result, gpointer user){
    struct bgs_viewport_s* viewport = (struct bgs_viewport_s*)user;
    
    GtkFileDialog* dialog = GTK_FILE_DIALOG(src);
    GError* error = NULL;

    GFile* file = gtk_file_dialog_open_finish(dialog, result, &error);
    if(file != NULL){
        // success, open scene
        BVR_PRINT(g_file_get_path(file));
        //bvr_create_page(viewport->page, g_file_get_path(file));
        g_object_unref(file);
    } 
    else {
        g_printerr("Error while choosing a file: %s\n", error->message);
        g_error_free(error);
    }
}

static void bgsi_on_open_scene(GtkButton* button, gpointer user_data){
    struct bgs_menu_s* menu = user_data;
    struct bgs_viewport_s* viewport = (struct bgs_viewport_s*)menu->parent;

    GtkFileDialog* diag = gtk_file_dialog_new();
    gtk_file_dialog_open(
        diag,
        GTK_WINDOW(gtk_widget_get_ancestor(GTK_WIDGET(button), GTK_TYPE_WINDOW)),
        NULL,
        bgsi_open_scene_read,
        viewport
    );
}

static void bgsi_on_save_scene(GtkButton* button, gpointer user_data){
    struct bgs_menu_s* menu = user_data;
    struct bgs_viewport_s* viewport = (struct bgs_viewport_s*)menu->parent;
}

static void bgsi_on_quit(GtkButton* button, gpointer user_data){
    struct bgs_menu_s* menu = user_data;
}

static void bgsi_on_settings(GtkButton* button, gpointer user_data){
    struct bgs_menu_s* menu = user_data;
}

static void* bgsi_menu_add_item(GtkWidget* popover, 
    const char* name, GCallback clbk, gpointer user){

    BVR_ASSERT(popover);

    GtkWidget* button = gtk_button_new_with_mnemonic(name);
    BVR_ASSERT(button);

    GtkWidget* child = gtk_button_get_child(GTK_BUTTON(button));
    if(child){
        gtk_widget_set_halign(child, GTK_ALIGN_START);
    }

    g_signal_connect(button, "clicked", clbk, user);
    g_signal_connect_swapped(button, "clicked", G_CALLBACK(gtk_popover_popdown), popover);

    return button;
}

static void* bgsi_new_menu_button(const char* name, GtkWidget** popover, GtkWidget** box){
    BVR_ASSERT(name);
    BVR_ASSERT(popover);
    BVR_ASSERT(box);
    
    GtkWidget* menu = gtk_menu_button_new();
    BVR_ASSERT(menu);

    gtk_menu_button_set_label(GTK_MENU_BUTTON(menu), name);

    *popover = gtk_popover_new();
    *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    BVR_ASSERT(*popover && *box);

    gtk_popover_set_child(GTK_POPOVER(*popover), *box);
    gtk_menu_button_set_popover(GTK_MENU_BUTTON(menu), *popover);

    return menu;
}

void* bgs_create_menuitems(struct bgs_menu_s* menu, void* parent){
    BVR_ASSERT(menu);
    BVR_ASSERT(parent);

    GtkWidget* popover;
    GtkWidget* box;

    menu->parent = parent;

    // contains all buttons
    menu->menu_widget = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    // contains file popover buttons
    // button for : file open, file save, quit
    menu->file_widget = bgsi_new_menu_button("_File", &popover, &box);
    gtk_menu_button_set_use_underline(GTK_MENU_BUTTON(menu->file_widget), TRUE);
    // add the open submenu
    gtk_box_append(
        GTK_BOX(box),
        GTK_WIDGET(bgsi_menu_add_item(popover, "_Open", G_CALLBACK(bgsi_on_open_scene), menu))
    );

    // add the save submenu
    gtk_box_append(
        GTK_BOX(box),
        GTK_WIDGET(bgsi_menu_add_item(popover, "_Save", G_CALLBACK(bgsi_on_save_scene), menu))
    );

    // add the quit submenu
    gtk_box_append(
        GTK_BOX(box),
        GTK_WIDGET(bgsi_menu_add_item(popover, "_Quit", G_CALLBACK(bgsi_on_quit), menu))
    );

    gtk_box_append(GTK_BOX(menu->menu_widget), menu->file_widget);

    // contains edit popover buttons
    // button for : settings
    menu->edit_widget = NULL;
    
    // returns the menu widget
    return menu->menu_widget;
}

void bgs_destroy_menuitems(struct bgs_menu_s* menu){
    g_object_unref(G_MENU(menu->file_widget));
    g_object_unref(G_MENU(menu->edit_widget));
}