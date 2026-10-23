#include <bergson/config.hpp>
#include <bergson/window.hpp>

using namespace Bergson;

int main(int argc, char** argv){
    auto application = Gtk::Application::create(BGS_CLASS);
    return application->make_window_and_run<Bergson::Window>(argc, argv);
}
