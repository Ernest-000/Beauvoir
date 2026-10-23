#include <bergson/inspector.hpp>

namespace  {
  
  template<class T>
  T InternalGenericClamp(double value){
    if constexpr (std::is_floating_point_v<T>) {
      return static_cast<T>(value);
    } else {
      constexpr double low = static_cast<double>(std::numeric_limits<T>::lowest());
      constexpr double high = static_cast<double>(std::numeric_limits<T>::max());
  
      if(value >= high) return std::numeric_limits<T>::max();
      if(value <= low) return std::numeric_limits<T>::lowest();
      return static_cast<T>(value);
    }
  }
}

using namespace Bergson;

Inspector::Inspector(){
  mSelectedActor = nullptr;

  // create list
  mFieldList = Gtk::make_managed<Gtk::ListBox>();
  mFieldList->set_selection_mode(Gtk::SelectionMode::NONE);
  mFieldList->add_css_class("rich-list");

  set_child(*mFieldList);
  
  set_propagate_natural_height(true);
  set_vexpand(true);
}

Inspector::~Inspector(){
  
}

void Inspector::SetSelectedActor(struct bvr_actor_s *actor){
  mSelectedActor = actor;

  mFieldList->remove_all();

  // avoid null actor, nothing displayed
  if(mSelectedActor == nullptr){
    return;
  }

  for (int f = 0; f < actor->vtable->field_count; f++) {
    AddField(&actor->vtable->ftable[f]);
  }
}

void Inspector::AddField(struct bvr_fields_s *field){
  Gtk::Box* row = Gtk::make_managed<Gtk::Box>();
  row->set_orientation(Gtk::Orientation::VERTICAL);
  row->set_spacing(12);
  row->set_margin_start(8);
  row->set_margin_end(8);
  row->set_margin_top(4);
  row->set_margin_bottom(4);

  Gtk::Label* name = Gtk::make_managed<Gtk::Label>(field->name);
  name->set_xalign(0.0f);
  name->set_hexpand(true);

  // unsafe pointer
  void* pointer = reinterpret_cast<uint8*>(mSelectedActor) + field->offset;
   
  Gtk::Widget* value = nullptr;
  WidgetFactoryFunc func = GetWidgetFactory(field->type);
  if(func){
    value = func(pointer);
  } else {
    value = Gtk::make_managed<Gtk::Label>("unknown");
    value->add_css_class("dim-label");
  }
  
  row->append(*name);
  row->append(*value);  

  mFieldList->append(*row);
}

template<class T>
Gtk::Widget* Inspector::CreateWidget(void* _value){
  T* value = static_cast<T*>(_value);

  if constexpr (std::is_same_v<T, bool>){
    Gtk::Switch* sw = Gtk::make_managed<Gtk::Switch>();

    sw->set_valign(Gtk::Align::CENTER);
    sw->set_active(*value);
    sw->property_active().signal_changed().connect([sw, value](){
      *value = sw->get_active();
    });

    return sw;
    
  } else {
    constexpr bool isFloat = std::is_floating_point_v<T>;
    constexpr double lo = static_cast<double>(std::numeric_limits<T>::lowest());
    constexpr double hi = static_cast<double>(std::numeric_limits<T>::max());

    Glib::RefPtr<Gtk::Adjustment> adj = Gtk::Adjustment::create(
      static_cast<double>(*value), lo, hi,
      isFloat ? 0.1 : 1.0,    // step
      isFloat ? 1.0 : 10.0    // page
    );

    Gtk::SpinButton* spin = Gtk::make_managed<Gtk::SpinButton>(
      adj, isFloat ? 0.1 : 1.0, isFloat ? 3 : 0
    );
    
    spin->set_numeric(true);

    // avoid scrolling
    Glib::RefPtr<Gtk::EventControllerScroll> controller = Gtk::EventControllerScroll::create();
    controller->set_propagation_phase(Gtk::PropagationPhase::CAPTURE);
    controller->signal_scroll().connect([controller](double x, double y){
        controller->signal_scroll().emission_stop();
        return 0;
      }, false
    );

    spin->add_controller(controller);
    
    // value changing callback
    spin->signal_value_changed().connect([spin, value](){
      *value = InternalGenericClamp<T>(spin->get_value());
    });

    return spin;
  }
}

Gtk::Widget* Inspector::CreateStringWidget(void* _string){
  bvr_string_t* string = reinterpret_cast<bvr_string_t*>(_string);
  
  Gtk::Entry* entry = Gtk::make_managed<Gtk::Entry>();
  entry->set_text(string->string);
  entry->signal_changed().connect([entry, string]() {
      bvr_overwrite_string(string, entry->get_text().c_str(), entry->get_text_length());
  });

  return entry;
}

template<class T, int N>
Gtk::Widget* Inspector::CreateVectorWidget(void *vec){
  Gtk::Box* box = Gtk::make_managed<Gtk::Box>(
    Gtk::Orientation::HORIZONTAL, N
  );

  for (int i = 0; i < N; i++) {
    // create a new entry for each vector component
    box->append(*CreateWidget<T>(static_cast<T*>(vec) + i));
  }

  return box;
}

WidgetFactoryFunc Inspector::GetWidgetFactory(int type){
  switch(type){
    case BVR_INT8: return &CreateWidget<int8>;
    case BVR_INT16: return &CreateWidget<int16>;
    case BVR_INT32: return &CreateWidget<int32>;
    case BVR_INT64: return &CreateWidget<int64>;
    case BVR_UNSIGNED_INT8: return &CreateWidget<uint8>;
    case BVR_UNSIGNED_INT16: return &CreateWidget<uint16>;
    case BVR_UNSIGNED_INT32: return &CreateWidget<uint32>;
    case BVR_UNSIGNED_INT64: return &CreateWidget<uint64>;
    case BVR_FLOAT: return &CreateWidget<float>;
    case BVR_DOUBLE: return &CreateWidget<double>;
    case BVR_STRING: return &CreateStringWidget;
    case BVR_VEC2: return &CreateVectorWidget<float, 2>;
    case BVR_VEC3: return &CreateVectorWidget<float, 3>;
    case BVR_VEC4: return &CreateVectorWidget<float, 4>;
    default: return nullptr;
  }
}
