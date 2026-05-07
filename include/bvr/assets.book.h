#pragma once

#include <bvr/config.h>
#include <bvr/common.h>
#include <bvr/scene.h>

void bvr_write_book_dataf(FILE* file, bvr_book_t* book);
void bvr_open_book_dataf(FILE* file, bvr_book_t* book);

BVR_H_FUNC void bvr_write_book(const char* path, bvr_book_t* book){
    FILE* file = fopen(path, "wb");
    bvr_write_book_dataf(file, book);
    fclose(file);
}

BVR_H_FUNC void bvr_open_book(const char* path, bvr_book_t* book){
    FILE* file = fopen(path, "rb");
    bvr_open_book_dataf(file, book);
    fclose(file);
}