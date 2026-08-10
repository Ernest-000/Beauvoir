#include <bvr/bvr.h>

static bvr_book_t book;

int main(void){
    struct bvr_book_attributes_s book_infos = {0};
    book_infos.name = "COUCOU";
    book_infos.window_flags = BVR_WINDOW_DEFAULT;
    
    bvr_create_book_attributes(&book, &book_infos);

    while (true)
    {
        bvr_new_frame();

        if(BVR_CAN_QUIT()){
            break;
        }

        bvr_render();
    }
    
    bvr_destroy_book(&book);

    return 0;
}