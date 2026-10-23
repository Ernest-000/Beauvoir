#pragma once

#include <bvr/config.h>
#include <bvr/common.h>

#include <bvr/window.h>
#include <bvr/book.h>
#include <bvr/graphics.h>

#include <gtkmm.h>

namespace Bergson {
    class Viewport : public Gtk::GLArea {
    private:
        // book holder
        bvr_book_t mBook;
        
        struct bvr_actor_s* mFocusedActor;
        struct bvr_page_s* mPage;

        bool OnTick(const Glib::RefPtr<Gdk::FrameClock>& clock);
        void OnCreateContext();
        void OnDestroyContext();
        bool OnDraw(const Glib::RefPtr<Gdk::GLContext>& context);
        void OnResize(int width, int height);

    public:
        Viewport();
        ~Viewport();  

        void LoadScene(const char* path);
        void SaveScene(const char* path);

        void SetFocusedActor(struct bvr_actor_s* actor);

        inline float GetWidth() const { return get_width() * get_scale_factor(); }
        inline float GetHeight() { return get_height() * get_scale_factor(); }

        inline bvr_page_t* GetCurrentPage() const { return mPage; }
    };
}
