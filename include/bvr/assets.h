#pragma once

#include <bvr/config.h>
#include <bvr/common.h>

#include <bvr/book.h>

/**
 * @brief load a page from a json file
 */
int bvr_load_pagef(bvr_page_t* page, FILE* file);