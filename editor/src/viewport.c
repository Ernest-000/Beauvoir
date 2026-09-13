#include <bergon/viewport.h>

#include <gtk/gtk.h>

#include <bvr/assets.h>
#include <bvr/window.h>
#include <bvr/gl.h>

static void bgs_viewport_gl_init_impl(GtkGLArea* self){
    // we need to ensure that the GdkGLContext is set before calling GL API */    
    gtk_gl_area_make_current(self);
    
    // link eproxy binding to beauvoir's bindings
    if(!bvr_load_gl(bvr_load_proc)){
        BVR_PRINT("unable to load opengl functions.");
        return;
    }

    // error
    if(gtk_gl_area_get_error(self) != NULL){
        BVR_PRINTF(
            "failed to create a new gl context! %s", 
            gtk_gl_area_get_error(self)->message
        );
        return;
    }
}

static void bgs_viewport_gl_deinit_impl(GtkGLArea* self){
    // clear context
    gtk_gl_area_make_current(self);
}

static gboolean bgs_viewport_render_impl(GtkGLArea* area, GdkGLContext* context){
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    return TRUE;
}

static void bgs_viewport_resize_impl(GtkGLArea* area, int width, int height, gpointer user_data)
{
    glViewport(0, 0, width, height);
}

static gboolean bgs_viewport_tick_impl(GtkWidget* widget, GdkFrameClock* frame_clock, gpointer user_data){
    // we need to force-render the gl area on each frame
    gtk_gl_area_queue_render(GTK_GL_AREA(widget));

    return G_SOURCE_CONTINUE;
}


void* bgs_create_viewport(struct bgs_viewport_s* viewport){
    BVR_ASSERT(viewport);

    viewport->context = NULL;
    viewport->widget = gtk_gl_area_new();
    BVR_ASSERT(viewport->widget);

    // opengl parameters
    gtk_gl_area_set_required_version(GTK_GL_AREA(viewport->widget), 3, 2);
    gtk_gl_area_set_has_depth_buffer(GTK_GL_AREA(viewport->widget), TRUE);
    gtk_gl_area_set_has_stencil_buffer(GTK_GL_AREA(viewport->widget), TRUE);

    // size
    gtk_widget_set_hexpand(GTK_WIDGET(viewport->widget), TRUE);
    gtk_widget_set_vexpand(GTK_WIDGET(viewport->widget), TRUE);

    // callbacks
    g_signal_connect(GTK_WIDGET(viewport->widget), "realize", G_CALLBACK(bgs_viewport_gl_init_impl), NULL);
    g_signal_connect(GTK_WIDGET(viewport->widget), "unrealize", G_CALLBACK(bgs_viewport_gl_deinit_impl), NULL);
    g_signal_connect(GTK_WIDGET(viewport->widget), "resize", G_CALLBACK(bgs_viewport_resize_impl), NULL);
    g_signal_connect(
        GTK_WIDGET(viewport->widget), "render", 
        G_CALLBACK(bgs_viewport_render_impl),
        NULL
    );

    gtk_widget_add_tick_callback(GTK_WIDGET(viewport->widget), bgs_viewport_tick_impl, NULL, NULL);
    
    bvr_load_page(&viewport->page, "scene.json");

    {
        viewport->pipeline.rendering_pass.blending = BVR_BLEND_FUNC_ALPHA_ONE_MINUS;
        viewport->pipeline.rendering_pass.depth = BVR_DEPTH_FUNC_LESS;
        viewport->pipeline.rendering_pass.flags = 0;

        viewport->pipeline.gui_pass.blending = BVR_BLEND_FUNC_ALPHA_ADD;
        viewport->pipeline.gui_pass.depth = BVR_DEPTH_TEST_DISABLE;
        viewport->pipeline.gui_pass.flags = BVR_SCISSORS_ENABLE;

        viewport->pipeline.swap_pass.blending = BVR_BLEND_DISABLE;
        viewport->pipeline.swap_pass.depth = BVR_DEPTH_TEST_DISABLE;
        viewport->pipeline.swap_pass.flags = 0;

        BVR_IDENTITY_VEC3(viewport->pipeline.clear_color);
        viewport->pipeline.state.framebuffer = NULL;

        viewport->pipeline.command_count = 0;
        memset(&viewport->pipeline.commands, 0, sizeof(viewport->pipeline.commands));
    }

    return viewport->widget;
}

void bgs_destroy_viewport(struct bgs_viewport_s* viewport){
    g_object_unref(GTK_GL_AREA(viewport->widget));
}