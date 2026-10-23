#pragma once

#include <bvr/config.h>
#include <bvr/common.h>
#include <bvr/actors.h>

#include <bergson/config.hpp>

#include <gtkmm.h>

namespace Bergson {
    using WidgetFactoryFunc = Gtk::Widget* (*)(void*);
    
    class Inspector : public Gtk::ScrolledWindow {
    private:
        Gtk::ListBox* mFieldList;
        struct bvr_actor_s* mSelectedActor;

        /**
         * @brief Create a new variable field to the inspector 
         */
        void AddField(struct bvr_fields_s* field);

        /**
         * @brief Create a new widget for the T typed field.
         */
        template<class T>
        static Gtk::Widget* CreateWidget(void* value);

        /**
         * @brief Create a new widget for a string typed field.
         */
        static Gtk::Widget* CreateStringWidget(void* string);

        /**
         * @brief Create a new widget for a vector typed field.
         */
        template<class T, int N>
        static Gtk::Widget* CreateVectorWidget(void* vec);
    
        /**
         * @brief Retrieve the correct widget function to create
         * a new field.
         */
        static WidgetFactoryFunc GetWidgetFactory(int type);
    public:
        Inspector();
        ~Inspector();

        void SetSelectedActor(struct bvr_actor_s* actor);
    };
}
