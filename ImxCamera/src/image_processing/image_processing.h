#pragma once
#include <Windows.h>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>
#include "utility.h"

#include "python_plot.h"
#include "png_converter.h"
#include "png.h"
// Question:
// 1. 꼭 정해진 resolution에서만 해야 하는가?


const std::string output_dir = "output/";

const int kImageBufferSize = 8388608; // 8MB
//const int kCsiHorizontalResolution = 1514 * 2;
//const int kCsiVerticalResolution = 991;
const int kCsiHorizontalResolution = (1514 - 138) * 2;
const int kCsiVerticalResolution = 988;
const int kBmpHeaderSize = 54;

const std::string kPythonPlotFunction = "plot_image";
const std::string kPythonSaveNumpyFunction = "save_image_by_numpy";

// These constants are value for modifying CCD
// It is empirical value
const int kRightBlack = 138;
const int kRightGray = 41;
const int kLeftGray = 16;


const int kBlakcPixelVariation = 10;

// New version
const int kCenterBlackLine = 56;
const int kRightImageWidth = 147;
const int kTopBlack = 3;
const int kBottomBlack = 38;
const int kLeftOneLine = 1; // There are duplicated line at the left corner

// Without black area
const int kEffectiveImageHeight = 948;
const int kEffectiveImageWidth = (int)kCsiHorizontalResolution / 2 - kCenterBlackLine;

// There are horizontal black pixel region at the some point of image. Basically, the width of black pixel is 21
const int kBlackPixelWidth = 21;
// Left side of black pixel, there are gray pixel region including one black line at the left side. The width of this region is 20
const int kLeftGrayPixelWidth = 20;
// Right side of black pixel, there are also gray pixel reion simliar to left region. The width is 15
const int kRightGrayPixelWidth = 15;
// So, the total gray + black region width is 56 = kCenterBlackLine

// There are cut image with this length. It seems there are some problem in sync. 
// So, we need to cut this image and attach left side to main image part
//const int kChoppedImage = 147;


const BYTE kaBmpHeader[] = 
{ 0x42, 0x4d, 0xbc, 0xc5, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x36, 0x00, 0x00, 0x00, 0x28, 0x00,
0x00, 0x00, 0xfc, 0x03, 0x00, 0x00, 0x57, 0x02, 0x00, 0x00, 0x01, 0x00, 0x08, 0x00, 0x00, 0x00,	//offset 18 has the word holding the horizontal resolution, offset 22 has the word holding the vertical resolution
0x00, 0x00, 0xec, 0xf7, 0x1b, 0x00, 0xc3, 0x0e, 0x00, 0x00, 0xc3, 0x0e, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

const enum ImageType {
	RAW_IMAGE = 0,
	MODIFIED_IMAGE = 1,
	CUT_OFF_IMAGE,
	PIXEL_INFO,
};

const enum CollectedImageFormat {
	RAW_IMAGE_FORMAT = 0,
	PACKED_RAW_IMAGE_FORMAT = 1,
	PACKED_PNG_IMAGE_FORMAT = 2,
	CUT_OFF_IMAGE_FORMAT = 3,
	PIXEL_INFO_FORMAT = 4,
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
			modified_raw_data_size_(0),
			collected_image_total_length_(0),
			collected_image_type_(RAW_IMAGE_FORMAT),
			minimum_value_(255),
			kHaveImageDimension(0),
			image_width_from_png_(0),
			image_height_from_png_(0)
		{
			memcpy(bmp_header_, kaBmpHeader, sizeof(kaBmpHeader));
			original_width_ = kCsiHorizontalResolution / 2;
			original_height_ = kCsiVerticalResolution;
			pPythonPlot_ = new PythonPlot();
		};
		~ImageProcessing();

		void AssembleImageData(BYTE * image_source, WORD image_length);
		

		


		void set_image_size_(ImageType mode);
		void set_bmp_header_(void);

		void fill_bmp_image_data_(ImageType type);
		void write_bmp_to_file(void);
		void write_raw_data_to_file(ImageType type);

		void ImageModification(void);
		void UnpackImageData(void);

		void ChooseImageProcessOption(void);
		void ShowImageProcessOptions(void);

		void SaveInRawFormat(ImageType type);
		void SaveInBitmapImage(ImageType tpye);

		void SetImageTotalLength(unsigned int image_total_length);
		void InitImageBuffer(void);
		void SetImageType(CollectedImageFormat type);
		void SavePixelData();

		// It needs to be replaced by python extension code
		void PlotInPython(void);
		void SaveInNumpyFormat(void);

		void initImageProcessOption(void);

		BYTE isAssembleCompleted();

		std::string GetFileNameDayHourMinSec();

		void ReadImageData(std::string file_name);
		int ImageProcessing::GetBlackPixelHeightFromBottom(int horizontal_pixel);


		// Cutting image and collect correctly
		int GetBlackLineStartPoint();
		int GetChoppedImageOffset();
		int GetChoppedImageLength(int horizontal_offset);
		void ApplyCutEachSide();


	private:

		PythonPlot * pPythonPlot_;

		// Array containing default raw image data(2bytes per pixel)
		std::vector<BYTE> collected_image_buffer_;
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

		unsigned int image_width_from_png_;
		unsigned int image_height_from_png_;

		unsigned int modified_width_;
		unsigned int modified_height_;

		unsigned int original_width_;
		unsigned int original_height_;

		unsigned int collected_image_total_length_;
		CollectedImageFormat collected_image_type_;
		BYTE minimum_value_;
		int invalid_pixel_offset_;

		BYTE kHaveImageDimension;
};
