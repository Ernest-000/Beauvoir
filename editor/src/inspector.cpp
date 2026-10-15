#include <bergson/inspector.hpp>
#include <bergson/viewport.hpp>

#include <gtk/gtk.h>

#define BGS_FILE_ILAYOUT "xml/ilayout.xml"

#define BGS_FIELD_GRID "ffields"

static void* bgs_inspector_add_field(struct bgs_inspector_field_s* field){
    BVR_ASSERT(field);

    GtkWidget* row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_widget_set_margin_start(row, 8);
    gtk_widget_set_margin_end(row, 8);
    gtk_widget_set_margin_top(row, 4);
    gtk_widget_set_margin_bottom(row, 4);

    GtkWidget* label = gtk_label_new(field->name);
    gtk_label_set_xalign(GTK_LABEL(label), 0.0f);
    gtk_widget_set_hexpand(label, TRUE);

    gtk_box_append(GTK_BOX(row), label);

    return row;
}

void* bgs_create_inspector(struct bgs_inspector_s* inspector, void* parent){
    BVR_ASSERT(inspector);
    BVR_ASSERT(parent);

    inspector->parent = parent;
    inspector->actor = NULL;

    inspector->store = gtk_list_box_new();
    gtk_list_box_set_selection_mode(GTK_LIST_BOX(inspector->store), GTK_SELECTION_NONE);
    gtk_widget_add_css_class(GTK_WIDGET(inspector->store), "rich-list");
    
    inspector->widget = gtk_scrolled_window_new();
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(inspector->widget), GTK_WIDGET(inspector->store));
    gtk_scrolled_window_set_propagate_natural_height(GTK_SCROLLED_WINDOW(inspector->widget), TRUE);
    gtk_widget_set_vexpand_set(GTK_WIDGET(inspector->widget), TRUE);

    bgs_inspector_set_actor(inspector, NULL);

    return inspector->widget;
}

void bgs_inspector_set_actor(struct bgs_inspector_s* inspector, struct bvr_actor_s* actor){
    BVR_ASSERT(inspector);

    inspector->actor = actor;
    if(inspector->actor == NULL){
        // no op
        return;
    }

    gtk_list_box_remove_all(GTK_LIST_BOX(inspector->store));

    struct bgs_inspector_field_s field;
    for (size_t f = 0; f < actor->vtable->field_count; f++)
    {
        struct bvr_fields_s* actor_field = &actor->vtable->ftable[f];
        field.name = actor_field->name;
        field.handle.origin = BVR_PHANDLE_FIELD;
        field.handle.pointer.field.self = actor;
        field.handle.pointer.field.field = *actor_field;

        // BVR_PRINTF("%s : %i", field.handle.pointer.field.field.name, field.handle.pointer.field.field.type);
        gtk_list_box_append(
            GTK_LIST_BOX(inspector->store),
            GTK_WIDGET(bgs_inspector_add_field(&field))
        );
    }
    
}

void bgs_destroy_inspector(struct bgs_inspector_s* inspector){

}