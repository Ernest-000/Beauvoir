#include <bergson/hierarchy.hpp>

#include <bvr/actors.h>

#include <gtk/gtk.h>

#define BGS_HIERARCHY_VISIBILITY_INDEX 0
#define BGS_HIERARCHY_NAME_INDEX 1

static void bgs_hierarchy_add_actor(struct bgs_hierarchy_s* hierarchy, struct bvr_actor_s* actor){
    GtkTreeIter iter;
    gtk_tree_store_append(GTK_TREE_STORE(hierarchy->store), &iter, NULL);
    gtk_tree_store_set(
        GTK_TREE_STORE(hierarchy->store),
        &iter,
        BGS_HIERARCHY_VISIBILITY_INDEX, actor->active,
        BGS_HIERARCHY_NAME_INDEX, actor->name.string,
        -1
    );

}

static void bgs_hierarchy_clear(struct bgs_hierarchy_s* hierarchy){
    gtk_tree_store_clear(GTK_TREE_STORE(hierarchy->store));
}

void bgs_hierarchy_row_activate (GtkTreeView* self, GtkTreePath* path, GtkTreeViewColumn* column, gpointer user_data) {
    struct bgs_hierarchy_s* hierarchy = (struct bgs_hierarchy_s*)user_data;
    
    int* indices = gtk_tree_path_get_indices(path);
    int index = indices[0];

    int i = 0;
    void* _actor;
    BVR_TABLE_FOR_EACH(&hierarchy->viewport->page->actors, _actor){    
        if(i++ == index){
            struct bvr_actor_s* actor = *((struct bvr_actor_s**)_actor);
            bgs_viewport_select_actor(hierarchy->viewport, actor);
        }    
    }
}

void* bgs_create_hierarchy(struct bgs_hierarchy_s* hierarchy, struct bgs_viewport_s* viewport){
    BVR_ASSERT(hierarchy);
    BVR_ASSERT(viewport);

    hierarchy->viewport = viewport;
    hierarchy->store = gtk_tree_store_new(
        2, G_TYPE_BOOLEAN, G_TYPE_STRING
    );

    hierarchy->treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(hierarchy->store));
    gtk_tree_view_set_activate_on_single_click(GTK_TREE_VIEW(hierarchy->treeview), TRUE);

    hierarchy->cell = gtk_cell_renderer_toggle_new();
    hierarchy->visible_column = gtk_tree_view_column_new_with_attributes(
        "Visible?",  GTK_CELL_RENDERER(hierarchy->cell),
        "active", BGS_HIERARCHY_VISIBILITY_INDEX,
        NULL
    );
    gtk_tree_view_append_column(GTK_TREE_VIEW(hierarchy->treeview), GTK_TREE_VIEW_COLUMN(hierarchy->visible_column));
    
    hierarchy->cell = gtk_cell_renderer_text_new();
    hierarchy->name_column = gtk_tree_view_column_new_with_attributes(
        "Name", GTK_CELL_RENDERER(hierarchy->cell),
        "text", BGS_HIERARCHY_NAME_INDEX,
        NULL
    );
    gtk_tree_view_append_column(GTK_TREE_VIEW(hierarchy->treeview), GTK_TREE_VIEW_COLUMN(hierarchy->name_column));

    g_signal_connect(GTK_TREE_VIEW(hierarchy->treeview), "row-activated", G_CALLBACK(bgs_hierarchy_row_activate), hierarchy);

    bgs_hierarchy_update_items(hierarchy);

    return hierarchy->treeview;
}

void bgs_hierarchy_update_items(struct bgs_hierarchy_s* hierarchy){
    BVR_ASSERT(hierarchy);

    BVR_PRINT("update");

    if(hierarchy->viewport == NULL || hierarchy->viewport->page == NULL){
        return;
    }

    bgs_hierarchy_clear(hierarchy);

    void* _actor;
    BVR_TABLE_FOR_EACH(&hierarchy->viewport->page->actors, _actor){
        struct bvr_actor_s* actor = *((struct bvr_actor_s**)_actor);

        bgs_hierarchy_add_actor(hierarchy, actor);
    }
}

void bgs_destroy_hierarchy(struct bgs_hierarchy_s* hierarchy){

}