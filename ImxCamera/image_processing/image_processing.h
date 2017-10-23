#pragma once
#include "../utility/utility.h"
#include <Windows.h>
#include <iostream>
#include <string>

// Question:
// 1. 꼭 정해진 resolution에서만 해야 하는가?
// 2. root를 이 프로젝트에 포함시킬수있는가? histogram용으로
// 
// Todo:
// 1. 코드가 뭔가 굉장히 더럽다. 좀 청소하자
// 2. root로 저장할 것인지 아니면 bmp로 저장할 것인지 선택사항 만들기.


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
		void write_bmp_to_file(void);

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
