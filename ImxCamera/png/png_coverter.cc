#include "stdafx.h"
#include "png_converter.h"


void read_data_memory(png_structp png_ptr, png_bytep data, png_size_t length){
	MEMORY_READER_STATE *f = (MEMORY_READER_STATE *)png_get_io_ptr(png_ptr);

	if (length > (f->bufsize - f->current_pos))
		png_error(png_ptr, "read error in read_data_memory (loadpng)");

	memcpy(data, f->buffer + f->current_pos, length);
	f->current_pos += length;
}

void DecodeImageData(std::vector<BYTE> & image_source, int & image_size) {
	png_bytep png_buffer_ptr = (png_bytep)&image_source[0];
	int number_to_check = 8;
	int is_png = !png_sig_cmp(png_buffer_ptr, 0, number_to_check);
	if (!is_png) {
		std::cout << "It is not png format" << std::endl;
		return;
	}

	// Create png pointer
	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr) {
		std::cout << "Error in make png_ptr" << std::endl;
		return;
	}
	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		std::cout << "Error in make info_ptr" << std::endl;
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		return;
	}

	png_infop end_info = png_create_info_struct(png_ptr);
	if (!end_info) {
		png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
		return;
	}

	// Set the custom read routine
	MEMORY_READER_STATE memory_reader_state;
	memory_reader_state.buffer = png_buffer_ptr;
	memory_reader_state.bufsize = (png_size_t)image_size;
	memory_reader_state.current_pos = number_to_check;
	png_set_read_fn(png_ptr, &memory_reader_state, read_data_memory);
	png_set_sig_bytes(png_ptr, number_to_check);

	// Start reading from info of png
	png_read_info(png_ptr, info_ptr);
	png_uint_32 width = 0, height = 0;
	int bitDepth = 0, colorType = -1;
	png_uint_32 retval = png_get_IHDR(png_ptr, info_ptr,
		&width,
		&height,
		&bitDepth,
		&colorType,
		NULL, NULL, NULL);

	if (retval != 1) {
		std::cout << "Error in reading png" << std::endl;
		return;
	}

	std::cout << "WIDTH x HEIGHT : " << width << " x " << height << std::endl;
	std::cout << "BITDEPTH : " << bitDepth << std::endl;
	std::cout << "COLOR TYPE : " << colorType << std::endl;
	png_read_update_info(png_ptr, info_ptr);
	// png_bytepp ? png_bytep ?
	png_bytepp read_row_pointers = (png_bytepp)png_malloc(png_ptr, sizeof(png_bytepp) * height);
	for (int i = 0; i < height; i++)
		read_row_pointers[i] = (png_bytep)png_malloc(png_ptr, width);

	png_set_rows(png_ptr, info_ptr, read_row_pointers);
	png_read_image(png_ptr, read_row_pointers);
	png_read_end(png_ptr, end_info);

	
	image_source.resize(width * height);
	for (int hi = 0; hi < height; hi++) {
		for (int wi = 0; wi < width; wi++) {
			image_source[wi + width * hi] = read_row_pointers[hi][wi];
		}
	}
	image_size = width * height;

	std::cout << "PNG TO BMP FIRST 100 PIXEL VALUE" << std::endl;;
	for (int i = 0; i<100; i++) {
		printf("%02x, ", read_row_pointers[0][i]);
	}
	printf("\n");

	// Finish the reading
	png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
	return;
}