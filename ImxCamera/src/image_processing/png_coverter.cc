#include "stdafx.h"
#include "png_converter.h"

static int img_width_ = 0;
static int img_height_ = 0;

void read_data_memory(png_structp png_ptr, png_bytep data, png_size_t length){
	MEMORY_READER_STATE *f = (MEMORY_READER_STATE *)png_get_io_ptr(png_ptr);

	if (length > (f->bufsize - f->current_pos))
		png_error(png_ptr, "read error in read_data_memory (loadpng)");

	memcpy(data, f->buffer + f->current_pos, length);
	f->current_pos += length;
}

void DecodeImageData(std::vector<BYTE> & image_source) {
	int imgSize = image_source.size();
	png_bytep png_buffer_ptr = (png_bytep)&image_source[0];
	int number_to_check = 8;
	int is_png = !png_sig_cmp(png_buffer_ptr, 0, number_to_check);
	if (!is_png) {
		cout << "It is not png format" << endl;
		return;
	}

	// Create png pointer
	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr) {
		cout << "Error in make png_ptr" << endl;
		return;
	}
	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		cout << "Error in make info_ptr" << endl;
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
	memory_reader_state.bufsize = (png_size_t)imgSize;
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

	img_width_ = width;
	img_height_ = height;

	if (retval != 1) {
		cout << "Error in reading png" << endl;
		return;
	}

	cout << "WIDTH x HEIGHT : " << width << " x " << height << endl;
	cout << "BITDEPTH : " << bitDepth << endl;
	cout << "COLOR TYPE : " << colorType << endl;
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

	cout << "PNG TO BMP FIRST 100 PIXEL VALUE" << endl;;
	for (int i = 0; i<100; i++) {
		printf("%02x, ", read_row_pointers[0][i]);
	}
	printf("\n");

	// Finish the reading
	png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
	return;
}

void GetImageWidthHeight(unsigned int & width, unsigned int & height) {
	width = img_width_;
	height = img_height_;
}