#include <bergson/hierarchy.hpp>
#include <bergson/viewport.hpp>
#include <bergson/window.hpp>

#include <bvr/collections/table.h>

using namespace Bergson;

Hierarchy::Hierarchy(){

  // create the column prototype
  mTreeStore = Gtk::TreeStore::create(mColumnModel);
  
  mTreeView = Gtk::make_managed<Gtk::TreeView>();
  mTreeView->set_model(mTreeStore);
  mTreeView->set_activate_on_single_click(true);
  mTreeView->append_column("Hash", mColumnModel.mHash);
  mTreeView->append_column("Name", mColumnModel.mName);
  mTreeView->signal_row_activated().connect(
    sigc::mem_fun(*this, &Hierarchy::OnRowActivated)
  );

  set_child(*mTreeView);
  set_policy(Gtk::PolicyType::AUTOMATIC, Gtk::PolicyType::AUTOMATIC);
}

Hierarchy::~Hierarchy(){
  
}

void Hierarchy::AddActor(struct bvr_actor_s *actor){
  BVR_ASSERT(actor);

  Gtk::TreeModel::iterator iter = mTreeStore->append();
  Gtk::TreeRow row = *iter;
  row[mColumnModel.mHash] = actor->hash;
  row[mColumnModel.mName] = Glib::ustring(actor->name.string);
}

void Hierarchy::SyncWithScene(bvr_page_t *page){
  BVR_ASSERT(page);

  mTreeStore->clear();

  void* actorRaw;
  BVR_TABLE_FOR_EACH(&page->actors, actorRaw){
    struct bvr_actor_s* actor = *((struct bvr_actor_s**)actorRaw);
    AddActor(actor);
  }
}

void Hierarchy::OnRowActivated(const Gtk::TreeModel::Path &path, Gtk::TreeViewColumn *column){
  Gtk::TreeModel::iterator iter = mTreeStore->get_iter(path);

  // when invalid iter
  if(!iter){
    return;
  }

  Viewport* viewport = (Viewport*)Window::GetInstance()->GetViewport();
  Gtk::TreeRow row = *iter;
  Glib::ustring name = row[mColumnModel.mName];

  struct bvr_actor_s** actor = (struct bvr_actor_s**) bvr_table_get(
    &viewport->GetCurrentPage()->actors,
    name.c_str()
  );

  if(actor){
    viewport->SetFocusedActor(*actor);  
  }
}
