#include <bergson/terminal.hpp>

#include <gtk/gtk.h>

void* bgs_create_terminal(struct bgs_terminal_s* terminal, void* parent){
    BVR_ASSERT(terminal);

    terminal->parent = parent;
    terminal->container = gtk_scrolled_window_new();
    gtk_widget_set_halign(GTK_WIDGET(terminal->container), GTK_ALIGN_FILL);
    gtk_widget_set_valign(GTK_WIDGET(terminal->container), GTK_ALIGN_FILL);
    gtk_widget_set_hexpand(GTK_WIDGET(terminal->container), TRUE);
    gtk_widget_set_vexpand(GTK_WIDGET(terminal->container), TRUE);

    return terminal->container;
}

void bgs_destory_terminal(struct bgs_terminal_s* terminal){

}