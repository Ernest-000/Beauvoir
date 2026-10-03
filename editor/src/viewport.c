#include <bergson/viewport.h>

#include <gtk/gtk.h>

#include <bvr/assets.h>
#include <bvr/actors.h>
#include <bvr/window.h>
#include <bvr/gl.h>

static struct bgs_viewport_s* __viewport;

void static bgs_error_callback(GLenum source, GLenum type, GLuint id,
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

static void bgs_viewport_gl_init_impl(GtkGLArea* self){
    // we need to ensure that the GdkGLContext is set before calling GL API */    
    gtk_gl_area_make_current(self);
    
    // error
    if(gtk_gl_area_get_error(self) != NULL){
        BVR_PRINTF(
            "failed to create a new gl context! %s", 
            gtk_gl_area_get_error(self)->message
        );
        return;
    }

    // link eproxy binding to beauvoir's bindings
    if(!bvr_load_gl(bvr_load_proc)){
        BVR_PRINT("unable to load opengl functions.");
        return;
    }

    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(bgs_error_callback, NULL);

    bgs_viewport_load_page(NULL, "scene.json");
}

static void bgs_viewport_gl_deinit_impl(GtkGLArea* self){
    // clear context
    gtk_gl_area_make_current(self);
}

static gboolean bgs_viewport_render_impl(GtkGLArea* area, GdkGLContext* context){
    bvr_book_t* book = BVR_INSTANCE();
    int scale = gtk_widget_get_scale_factor(GTK_WIDGET(area));
    glViewport(
        0, 0,
        gtk_widget_get_width(GTK_WIDGET(area)) * scale,
        gtk_widget_get_height(GTK_WIDGET(area)) * scale
    );

    // clear
    glClearColor(.5, 0.1, 0.1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    bvr_pipeline_state_enable(&book->graphics.rendering_pass);
    
    // draw everything that is queued
    bvr_flush();
    
    // clear drawing queue
    book->graphics.command_count = 0;

    return TRUE;
}

static void bgs_viewport_resize_impl(GtkGLArea* area, int width, int height, gpointer user_data)
{
    glViewport(0, 0, width, height);

    // overwrite 
    BVR_CAMERA()->ortho.width = width;
    BVR_CAMERA()->ortho.height = height;
}

static gboolean bgs_viewport_tick_impl(GtkWidget* widget, GdkFrameClock* frame_clock, gpointer user_data){
    // we need to force-render the gl area on each frame
    gtk_gl_area_queue_render(GTK_GL_AREA(widget));

    struct bgs_viewport_s* viewport = (struct bgs_viewport_s*)user_data;
    if(!viewport || !viewport->page){
        return G_SOURCE_CONTINUE;
    }

    bvr_update_camera(&viewport->page->camera);

    struct bvr_actor_s** p_actor;
    BVR_TABLE_FOR_EACH(&viewport->page->actors, p_actor){
        struct bvr_actor_s* actor = *p_actor;

        if(actor->active){
            BVR_ACTOR_DRAW((bvr_static_mesh_t*)actor, BVR_DRAWMODE_TRIANGLES);
        }
    }

    return G_SOURCE_CONTINUE;
}


void* bgs_create_viewport(struct bgs_viewport_s* viewport, void* parent){
    BVR_ASSERT(viewport);

    __viewport = viewport;
    viewport->context = NULL;
    viewport->parent = parent;
    viewport->widget = gtk_gl_area_new();
    BVR_ASSERT(viewport->widget);

    // opengl parameters
    gtk_gl_area_set_required_version(GTK_GL_AREA(viewport->widget), 3, 3);
    gtk_gl_area_set_has_depth_buffer(GTK_GL_AREA(viewport->widget), TRUE);
    gtk_gl_area_set_has_stencil_buffer(GTK_GL_AREA(viewport->widget), TRUE);
    gtk_gl_area_set_use_es(GTK_GL_AREA(viewport->widget), FALSE);

    // size
    gtk_widget_set_hexpand(GTK_WIDGET(viewport->widget), TRUE);
    gtk_widget_set_vexpand(GTK_WIDGET(viewport->widget), TRUE);

    // callbacks
    g_signal_connect(GTK_WIDGET(viewport->widget), "realize", G_CALLBACK(bgs_viewport_gl_init_impl), viewport);
    g_signal_connect(GTK_WIDGET(viewport->widget), "unrealize", G_CALLBACK(bgs_viewport_gl_deinit_impl), viewport);
    g_signal_connect(GTK_WIDGET(viewport->widget), "resize", G_CALLBACK(bgs_viewport_resize_impl), viewport);
    g_signal_connect(
        GTK_WIDGET(viewport->widget), "render", 
        G_CALLBACK(bgs_viewport_render_impl),
        viewport
    );

    gtk_widget_add_tick_callback(GTK_WIDGET(viewport->widget), bgs_viewport_tick_impl, viewport, NULL);

    {
        struct bvr_book_attributes_s battributes = {0};
        battributes.name = "landscape demo";
        battributes.window_width = 800;
        battributes.window_height = 800;
        battributes.window_flags = BVR_WINDOW_NONE;

        bvr_create_book_attributes(&viewport->book, &battributes);

        viewport->page = bvr_enable_page(0);
        BVR_ASSERT(viewport->page);
    }

    return viewport->widget;
}

int bgs_viewport_load_page(struct bgs_viewport_s* viewport, const char* path){
    if(viewport == NULL){
        viewport = __viewport;
    }

    if(path == NULL){
        BVR_PRINT("invalid path");
        return BVR_FALSE;
    }

    if(!viewport->page){
        return BVR_FALSE;
    }

    bvr_create_page(viewport->page, path);
    return BVR_TRUE;
}

void bgs_destroy_viewport(struct bgs_viewport_s* viewport){
    g_object_unref(GTK_GL_AREA(viewport->widget));
}