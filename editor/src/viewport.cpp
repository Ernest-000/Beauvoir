#include <bergson/viewport.hpp>
#include <bergson/hierarchy.hpp>
#include <bergson/window.hpp>
#include <bergson/inspector.hpp>

#include <bvr/actors.h>
#include <bvr/gl.h>

using namespace Bergson;

// define default stream size
#define BGS_DEFAULT_STREAM_SIZE (sizeof(struct bvr_actor_s) * 1024)

void static bgs_ViewportGLCallback(GLenum source, GLenum type, GLuint id,
   GLenum severity, GLsizei length, const GLchar* message, const void* userParam){
    
    if(severity == GL_DEBUG_SEVERITY_NOTIFICATION){
        return;
    }

    char src[25];
    char error[25];

    switch (source)
    {
        case GL_DEBUG_SOURCE_API:
            BVR_STRCPY(src, "API", 4);
            break;

        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            BVR_STRCPY(src, "WINDOW SYSTEM", 14);
            break;

        case GL_DEBUG_SOURCE_SHADER_COMPILER:
            BVR_STRCPY(src, "SHADERS", 8);
            break;
        
        case GL_DEBUG_SOURCE_THIRD_PARTY:
            BVR_STRCPY(src, "THIRD PARTY", 12);
            break;
        
        case GL_DEBUG_SOURCE_APPLICATION:
            BVR_STRCPY(src, "APPLICATION", 12);
            break;
        
        case GL_DEBUG_SOURCE_OTHER:
            BVR_STRCPY(src, "OTHER", 6);
            break;
    };
    
    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:
            BVR_STRCPY(error, "fatal error", 12);
            BVR_PRINTF("catch a new %s (%i) from OGL %s! '%s'", error, id, src, message);
            BVR_ASSERT(0);
            break;

        default:
            BVR_STRCPY(error, "unknown", 6);
            BVR_PRINTF("catch a new %s (%i) from OGL %s! '%s'", error, id, src, message);
            break;
        
    };

}

Viewport::Viewport(){
  mFocusedActor = nullptr;
  mPage = nullptr;
  
  // gl version
  set_required_version(3, 2);
  set_has_depth_buffer(true);
  set_has_stencil_buffer(true);
  set_use_es(false);

  // size
  set_hexpand(true);
  set_vexpand(true);

  set_halign(Gtk::Align::FILL);
  set_valign(Gtk::Align::FILL);
  
  // add the tick callback
  add_tick_callback(
    sigc::mem_fun(*this, &Viewport::OnTick)
  );

  signal_realize().connect(sigc::mem_fun(*this, &Viewport::OnCreateContext));
  signal_unrealize().connect(sigc::mem_fun(*this, &Viewport::OnDestroyContext));
  signal_render().connect(sigc::mem_fun(*this, &Viewport::OnDraw), false);
  signal_resize().connect(sigc::mem_fun(*this, &Viewport::OnResize));

  // create a default book
  std::string name = std::string("default-editor");

  mBook = bvr_book_t();
  
  struct bvr_book_attributes_s config = {0};
  config.name = (char*)name.c_str();
  config.window_width = 500;
  config.window_height = 500;
  config.window_flags = BVR_WINDOW_NONE;

  bvr_create_book_attributes(&mBook, &config);

  mPage = bvr_enable_page(0);
  BVR_ASSERT(mPage);
}

Viewport::~Viewport(){
  
}
  
void Viewport::OnCreateContext(){
  make_current();

  if(has_error()){
    BVR_PRINT("failed to create a new gl context!");
    return;
  }

  if(!bvr_load_gl(bvr_load_proc)){
    BVR_PRINT("failed to load open gl functions!");
    return;
  }

  glEnable(GL_DEBUG_OUTPUT);
  glDebugMessageCallback(bgs_ViewportGLCallback, NULL);

  LoadScene("scene.json");
}

void Viewport::OnDestroyContext(){
  make_current();
}

bool Viewport::OnDraw(const Glib::RefPtr<Gdk::GLContext> &context){
  glViewport(0, 0, GetWidth(), GetHeight());

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // flush and enable pipeline state
  //
  bvr_pipeline_state_enable(&mBook.graphics.rendering_pass);
  bvr_flush();

  mBook.graphics.command_count = 0;
  
  return true;
}

void Viewport::OnResize(int width, int height){
  glViewport(0, 0, width, height);
  bvr_camera_resize(BVR_CAMERA(), width, height);
}

bool Viewport::OnTick(const Glib::RefPtr<Gdk::FrameClock> &clock){
  queue_render();

  bvr_update_camera(BVR_CAMERA());

  // raw pointer reference
  void* _actor;
  BVR_TABLE_FOR_EACH(&mPage->actors, _actor){
    struct bvr_actor_s* actor = *(struct bvr_actor_s**)_actor;
    if(actor->active){
      BVR_ACTOR_DRAW((bvr_static_mesh_t*)actor, BVR_DRAWMODE_TRIANGLES);
    }
  }

  // G_SOURCE_CONTINUE
  return true;
}

void Viewport::SetFocusedActor(struct bvr_actor_s *actor){
  Inspector* inspector = (Inspector*)Window::GetInstance()->GetInspector(); 
  inspector->SetSelectedActor(actor);

  mFocusedActor = actor;
}

void Viewport::LoadScene(const char *path){
  BVR_FILE_EXISTS(path);

  if(mPage->actors.count){
    // destroy previous page
    bvr_destroy_page(mPage);
  }

  bvr_create_page(mPage, path);

  // camera must be overwritten for controls
  bvr_create_persp_camera(
    &mPage->camera,
    GetWidth(),
    GetHeight(),
    0.001f,
    1000.0f,
    90.0f
  );

  Hierarchy* hierarchy = (Hierarchy*)Window::GetInstance()->GetHierarchy();
  if(hierarchy){
    hierarchy->SyncWithScene(mPage);
  }
}

void Viewport::SaveScene(const char *path){
  BVR_ASSERT(0 || "Not implemented");
}
