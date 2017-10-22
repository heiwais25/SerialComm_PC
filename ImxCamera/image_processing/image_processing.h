#pragma once
#include "../utility/utility.h"
#include <Windows.h>
#include <iostream>
#include <string>
// 일단은 이 함수는 기본적으로 정해진 Resolution에 대해서 작동하는 걸로 만들었다.
// 재활용성은 다음에 다 만들면 다시 하자

const int kImageBufferSize = 8388608; // 8MB
const int kCsiHorizontalResolution = 1514 * 2;
const int kCsiVerticalResolution = 991;
const int kBmpHeaderSize = 54;

const BYTE kaBmpHeader[] = 
{ 0x42, 0x4d, 0xbc, 0xc5, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x36, 0x00, 0x00, 0x00, 0x28, 0x00,
0x00, 0x00, 0xfc, 0x03, 0x00, 0x00, 0x57, 0x02, 0x00, 0x00, 0x01, 0x00, 0x18, 0x00, 0x00, 0x00,	//offset 18 has the word holding the horizontal resolution, offset 22 has the word holding the vertical resolution
0x00, 0x00, 0xec, 0xf7, 0x1b, 0x00, 0xc3, 0x0e, 0x00, 0x00, 0xc3, 0x0e, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

class ImageProcessing {
	public:
		ImageProcessing() 
			:image_size_(kCsiHorizontalResolution * kCsiVerticalResolution),
			image_buffer_count_(0)
		{
			memcpy(bmp_header_, kaBmpHeader, sizeof(kaBmpHeader));
		};
		~ImageProcessing();

		void AssembleImageData(BYTE * image_source, WORD image_length);

		void set_bmp_header_(int image_width, int image_height);
		void fill_bmp_image_data_(void);
		void write_to_file(void);

		void SaveImageToBmp();

		BYTE isAssembleCompleted();

	private:

		BYTE image_buffer_[kImageBufferSize];
		BYTE bmp_header_[kBmpHeaderSize];
		BYTE bmp_image_data_[kImageBufferSize];

		FILE * hWriteFile_;

		unsigned int bmp_file_size_;
		unsigned int bmp_image_width_;
		unsigned int bmp_image_height_;

		unsigned int image_buffer_count_;
		unsigned int image_size_;
};
