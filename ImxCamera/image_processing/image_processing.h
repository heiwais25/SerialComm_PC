#pragma once
#include "../utility/utility.h"
#include <Windows.h>
#include <iostream>
#include <string>
#include <fstream>

// Question:
// 1. 꼭 정해진 resolution에서만 해야 하는가?
// 2. root를 이 프로젝트에 포함시킬수있는가? histogram용으로
// 
// Todo:
// 1. 코드가 뭔가 굉장히 더럽다. 좀 청소하자
// 2. root로 저장할 것인지 아니면 bmp로 저장할 것인지 선택사항 만들기.

const std::string output_dir = "output/";

const int kImageBufferSize = 8388608; // 8MB
const int kCsiHorizontalResolution = 1514 * 2;
const int kCsiVerticalResolution = 991;
const int kBmpHeaderSize = 54;

// These constants are value for modifying CCD
// It is empirical value
const int kRightBlack = 138;
const int kRightGray = 41;
const int kLeftGray = 16;
const int kBottomBlack = 2;
const int kTopBlack = 13;

const BYTE kaBmpHeader[] = 
{ 0x42, 0x4d, 0xbc, 0xc5, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x36, 0x00, 0x00, 0x00, 0x28, 0x00,
0x00, 0x00, 0xfc, 0x03, 0x00, 0x00, 0x57, 0x02, 0x00, 0x00, 0x01, 0x00, 0x18, 0x00, 0x00, 0x00,	//offset 18 has the word holding the horizontal resolution, offset 22 has the word holding the vertical resolution
0x00, 0x00, 0xec, 0xf7, 0x1b, 0x00, 0xc3, 0x0e, 0x00, 0x00, 0xc3, 0x0e, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

const enum ImageType {
	DEFAULT_IMAGE = 0,
	MODIFIED_IMAGE = 1,
};

const enum ImageProcessOptionList {
	SAVE_DEFAULT_BITMAP = 0,
	SAVE_MODIFIED_BITMAP,
	SAVE_DEFAULT_RAW_DATA,
	SAVE_MODIFIED_RAW_DATA,
	PLOT_IN_PYTHON,

};

class ImageProcessing {
	public:
		ImageProcessing() 
			:default_raw_data_size_(kCsiHorizontalResolution * kCsiVerticalResolution),
			image_buffer_count_(0),
			modified_raw_data_size_(0)
		{
			memcpy(bmp_header_, kaBmpHeader, sizeof(kaBmpHeader));
		};
		~ImageProcessing();

		void AssembleImageData(BYTE * image_source, WORD image_length);
		void ApplyCutEachSide();

		


		void set_image_size_(ImageType mode);
		void set_bmp_header_(void);

		void fill_bmp_image_data_(ImageType type);
		void fill_modified_bmp_image_data_(void);

		void write_bmp_to_file(void);
		void write_raw_data_to_file(ImageType type);

		void ChooseImageProcessOption(void);
		void ShowImageProcessOptions(void);

		void SaveInRawFormat(ImageType type);
		void SaveInBitmapImage(ImageType tpye);

		// It needs to be replaced by python extension code
		void PlotInPython(void);

		void initImageProcessOption(void);

		BYTE isAssembleCompleted();

		std::string GetFileNameDayHourMinSec();

		void ReadImageData(std::string file_name);


	private:

		// Array containing default raw image data(2bytes per pixel)
		BYTE image_buffer_[kImageBufferSize]; 
		// Array containing modified raw image data excluding black and gray side(2bytes per pixel)
		BYTE modified_image_buffer_[kImageBufferSize]; 

		BYTE bmp_header_[kBmpHeaderSize];
		BYTE bmp_image_data_[kImageBufferSize];

		FILE * hWriteFile_;

		unsigned int bmp_file_size_;

		unsigned int image_buffer_count_;
		unsigned int default_raw_data_size_;
		unsigned int modified_raw_data_size_;

		unsigned int image_width_;
		unsigned int image_height_;
};
