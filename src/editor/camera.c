#include <BVR/editor/camera.h>
#include <BVR/window.h>

void bvr_init_free_camera(bvr_book_t* book, struct bvr_free_camera_s* _camera_controller, float movement_speed, float rotation_speed){
    _camera_controller->forward_speed = movement_speed;
    _camera_controller->strafe_speed = movement_speed;
    _camera_controller->updown_speed = movement_speed;
    _camera_controller->rot_speed = rotation_speed;
    _camera_controller->pitch_speed = rotation_speed;

    _camera_controller->rotation = book->page.camera.transform.rotation[1];
    _camera_controller->tilt = book->page.camera.transform.rotation[2];
}

void bvr_update_free_camera(bvr_book_t* book, struct bvr_free_camera_s* _camera_controller, bvr_editor_t* editor){
    if(bvr_key_down(BVR_KEY_R)){
        _camera_controller->rotation = 0.0f;
        _camera_controller->tilt = 0.0f;
        book->page.camera.transform.rotation[2] = 0.0f;
        book->page.camera.transform.rotation[1] = 0.0f;
    }

    if(bvr_button_down(BVR_MOUSE_BUTTON_LEFT) && !editor->device.is_gui_hovered){
        _camera_controller->rotation += book->window.inputs.relative_motion[0] * _camera_controller->rot_speed * book->timer.delta_timef;
        _camera_controller->tilt -= book->window.inputs.relative_motion[1] * _camera_controller->pitch_speed * book->timer.delta_timef;

        book->page.camera.transform.rotation[2] = _camera_controller->rotation;
        book->page.camera.transform.rotation[1] = _camera_controller->tilt;
    }

    _camera_controller->forward = 0;
    _camera_controller->strafe = 0;

    _camera_controller->forward += bvr_axis_down(&book->window.inputs.axis.vertical) * _camera_controller->forward_speed * book->timer.delta_timef;
    _camera_controller->strafe  -= bvr_axis_down(&book->window.inputs.axis.horizontal) * _camera_controller->strafe_speed * book->timer.delta_timef;

    book->page.camera.transform.position[0] += _camera_controller->strafe;
    book->page.camera.transform.position[1] += _camera_controller->forward;

    book->page.camera.field_of_view.scale += book->window.inputs.scroll * _camera_controller->forward_speed * book->timer.delta_timef;
}