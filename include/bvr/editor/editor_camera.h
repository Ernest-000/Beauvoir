#include <bvr/editor/editor.h>

#ifndef BVR_NO_NUKLEAR

#include <bvr/book.h>

struct bvr_free_camera_s {
    float forward_speed;
    float strafe_speed;
    float updown_speed;
    float rot_speed;
    float pitch_speed;

    float rotation, tilt, forward, updown, strafe;
};

BVR_H_FUNC void bvr_init_free_camera(bvr_book_t* book, struct bvr_free_camera_s* camera, float movement_speed, float rotation_speed){
    camera->forward_speed = movement_speed;
    camera->strafe_speed = movement_speed;
    camera->updown_speed = movement_speed;
    camera->rot_speed = rotation_speed;
    camera->pitch_speed = rotation_speed;
    camera->rotation = book->page->camera.transform.rotation[1];
    camera->tilt = book->page->camera.transform.rotation[2];
}

void bvr_update_free_camera(bvr_book_t* book, struct bvr_free_camera_s* camera, bvr_editor_t* editor){
    if(bvr_key_down(BVR_KEY_R)){
        camera->rotation = 0.0f;
        camera->tilt = 0.0f;
        book->page->camera.transform.rotation[2] = 0.0f;
        book->page->camera.transform.rotation[1] = 0.0f;
    }

    if(bvr_button_down(BVR_MOUSE_BUTTON_LEFT) && !editor->device.is_gui_hovered){
        camera->rotation += book->window.inputs.relative_motion[0] * camera->rot_speed * book->timer.delta_time;
        camera->tilt -= book->window.inputs.relative_motion[1] * camera->pitch_speed * book->timer.delta_time;

        book->page->camera.transform.rotation[2] = camera->rotation;
        book->page->camera.transform.rotation[1] = camera->tilt;
    }

    camera->forward = 0;
    camera->strafe = 0;

    camera->forward += bvr_axis_down(&book->window.inputs.axis.vertical) * camera->forward_speed * book->timer.delta_time;
    camera->strafe  -= bvr_axis_down(&book->window.inputs.axis.horizontal) * camera->strafe_speed * book->timer.delta_time;

    book->page->camera.transform.position[0] += camera->strafe;
    book->page->camera.transform.position[1] += camera->forward;

    book->page->camera.field_of_view.scale += book->window.inputs.scroll * camera->forward_speed * book->timer.delta_time;

}

#endif