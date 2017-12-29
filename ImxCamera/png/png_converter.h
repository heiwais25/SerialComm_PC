#pragma once
#ifndef __PNG_CONVERTER__
#define __PNG_CONVERTER__
#include <iostream>
#include <vector>
#include <Windows.h>
#include "png.h"
#include "zlib.h"

typedef struct {
	png_bytep buffer;
	png_uint_32 bufsize;
	png_uint_32 current_pos;
} MEMORY_READER_STATE;

void DecodeImageData(std::vector<BYTE> & image_source, int & image_size);

#endif