#include <bvr/config.h>
#include <bvr/common.h>

#include <bergson/config.hpp>

#include <gtkmm.h>

namespace Bergson {    
    class MenuLayout : public Gtk::FlowBox
    {
    private:
        Gtk::MenuButton* NewMenuButton(const char* string, const int submenuCount);

        template <typename F>
        void AddPopoverItem(Gtk::MenuButton* button, const char* string, F&& callback);
    public:
        MenuLayout();
        ~MenuLayout();    
    };
}   
