#include <BVR/editor/editor.h>
#include <BVR/scene.h>

struct bvr_free_camera_s {
    float forward_speed;
    float strafe_speed;
    float updown_speed;
    float rot_speed;
    float pitch_speed;

    float rotation, tilt, forward, updown, strafe;
};

void bvr_init_free_camera(bvr_book_t* book, struct bvr_free_camera_s* camera, float movement_speed, float rotation_speed);
void bvr_update_free_camera(bvr_book_t* book, struct bvr_free_camera_s* camera, bvr_editor_t* editor);