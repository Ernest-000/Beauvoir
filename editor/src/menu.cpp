#include <bvr/common.h>
#include <bergson/menu.hpp>
#include <bergson/window.hpp>

using namespace Bergson;

MenuLayout::MenuLayout(){
  Gtk::MenuButton* fileButton = NewMenuButton("File", 2);
  AddPopoverItem(fileButton, "Open", []{
    // when the user open a new scene                 
  });
  AddPopoverItem(fileButton, "Save", []{
    // when the user save the current scene    
  });

  Gtk::MenuButton* editButton = NewMenuButton("Edit", 0);

  Gtk::MenuButton* winButton = NewMenuButton("Window", 0);

  insert(*fileButton, 0);
  insert(*editButton, 1);
  insert(*winButton, 2);

  set_can_focus(false);
}

MenuLayout::~MenuLayout(){
  
}

Gtk::MenuButton* MenuLayout::NewMenuButton(const char* string, const int submenuCount){
  // make sure to have a valid string 
  BVR_ASSERT(string);

  Gtk::MenuButton* button = Gtk::make_managed<Gtk::MenuButton>();
  BVR_ASSERT(button);
  button->set_use_underline(true);
  button->set_label(string);
  button->set_can_focus(false);

  Gtk::Popover* popover = Gtk::make_managed<Gtk::Popover>();
  Gtk::Box* box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL, submenuCount);
  BVR_ASSERT(popover && box);

  popover->set_child(*box);
  button->set_popover(*popover);

  return button;
}

template <typename F>
void MenuLayout::AddPopoverItem(Gtk::MenuButton *button, const char* string, F&& callback){
  BVR_ASSERT(button);
  BVR_ASSERT(string);

  // instantiate a new button
  Gtk::Button* nButton = Gtk::make_managed<Gtk::Button>();
  nButton->set_label(string);
  nButton->set_use_underline(true);

  // check for alignement
  Gtk::Widget* child = nButton->get_child();
  if(child){
    child->set_halign(Gtk::Align::START);
  }

  // add the button to the container
  Gtk::Box* container = (Gtk::Box*)button->get_popover()->get_child();
  container->append(*nButton);

  // link the callback
  nButton->signal_clicked().connect(callback);
  nButton->signal_clicked().connect([button]{
    button->get_popover()->popdown();                              
  });
  
}
