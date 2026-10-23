#pragma once

#include <bvr/common.h>
#include <bvr/config.h>
#include <bvr/collections/string.h>

#include <gtkmm.h>

namespace Bergson
{
    class Window : public Gtk::Window
    {
    private:
        static inline Window* sInstance;
        
        uint16 mWidth;
        uint16 mHeight;
        bvr_string_t mTitle;

        Glib::RefPtr<Gtk::Builder> mBuilder;
        Glib::RefPtr<Gtk::CssProvider> mCss;
        Glib::RefPtr<Gtk::Widget> mLayout;
        
        Gtk::FlowBox* mMenuList;
        Gtk::GLArea* mViewport;
        Gtk::ScrolledWindow* mHierarchy;
        Gtk::ScrolledWindow* mInspector;
        
        bool BuilderAddNewFile(const char* file);
    public:
        static inline Window* GetInstance() noexcept {
            return sInstance;
        }

        // default constructor (so that the factory could create it)
        Window() : Window("Bergson", 1000, 800) {}
        Window(const char* title, const uint16 width, const uint16 height);
        ~Window();

        inline Gtk::FlowBox* GetMenuList() const { return mMenuList; }
        inline Gtk::GLArea* GetViewport() const { return mViewport; }
        inline Gtk::ScrolledWindow* GetHierarchy() const { return mHierarchy; }
        inline Gtk::ScrolledWindow* GetInspector() const { return mInspector; }
    };
} 
