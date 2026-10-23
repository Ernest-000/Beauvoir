#pragma once

#include <bvr/config.h>
#include <bvr/common.h>
#include <bvr/actors.h>
#include <bvr/book.h>

#include <bergson/config.hpp>
#include <bergson/viewport.hpp>

#include <gtkmm.h>

namespace Bergson {
    
    class HierarchyColumnModel : public Gtk::TreeModelColumnRecord {
    public :
        HierarchyColumnModel(){
            add(mHash);
            add(mName);
        }

        Gtk::TreeModelColumn<int> mHash;
        Gtk::TreeModelColumn<Glib::ustring> mName;
    };
    
    class Hierarchy : public Gtk::ScrolledWindow {
    private :
        HierarchyColumnModel mColumnModel;
        
        Gtk::TreeView* mTreeView;
        Glib::RefPtr<Gtk::TreeStore> mTreeStore;
        
        void OnRowActivated(const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column);
    public :
        Hierarchy();
        ~Hierarchy();

        void AddActor(struct bvr_actor_s* actor);
        void SyncWithScene(bvr_page_t* page);
    };
}
