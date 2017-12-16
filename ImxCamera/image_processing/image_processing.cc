#include "stdafx.h"
#include "image_processing.h"

// Todo : 16bit로 저장하기


ImageProcessing::~ImageProcessing() {

}

/*----------------------------------------------------------------------------------------------------------

	Main stream

----------------------------------------------------------------------------------------------------------*/

/*
	After filling image data by using "AssembleImageData" or "ReadImageData", use this function
	It will help you to use several image processing.
	Each option is divided two kinds (Default, Modified)
	1) Default : including sensor black area at each side
	2) Modified : excluding sensor black area
*/
void ImageProcessing::ChooseImageProcessOption(void) {
	BYTE c;
	ShowImageProcessOptions();
	while (1) {

		c = GetOneCharKeyboardInput();
		if (c == 'x' || c == 'X') {
			initImageProcessOption();
			std::cout << "Exit" << std::endl;
			break;
		}

		if (isDecimalNumber(c)) {
			switch (c)
			{
			case '1':
				SaveInBitmapImage(DEFAULT_IMAGE);
				break;

			case '2':
				SaveInBitmapImage(MODIFIED_IMAGE);
				break;

			case '3':
				SaveInRawFormat(DEFAULT_IMAGE);
				break;

			case '4':
				SaveInRawFormat(MODIFIED_IMAGE);
				break;

			case '5':
				PlotInPython();
				break;

			case '6':
				SaveInNumpyFormat();
				break;

			default:
				break;
			}
		}
	}
}

/*
	Save the image data by bitmap format to see clearly
	Due to easy approach, current bitmap is only using high 8bit
*/
void ImageProcessing::SaveInBitmapImage(ImageType type) {
	set_image_size_(type);

	fill_bmp_image_data_(type);

	write_bmp_to_file();
}

/*
	To deal with data more in detail, it is necessary to save data by raw format.
	It will be used for other analysis tool
*/
void ImageProcessing::SaveInRawFormat(ImageType type) {
	set_image_size_(type);

	write_raw_data_to_file(type);
}

// todo....
void ImageProcessing::PlotInPython(void) {
	Sleep(700 /* ms */);
	set_image_size_(MODIFIED_IMAGE);
	pPythonPlot_->CopyRawImageData(modified_image_buffer_, modified_width_, modified_height_);
	pPythonPlot_->DrawPlot(kPythonPlotFunction);
}

void ImageProcessing::SaveInNumpyFormat(void) {
	Sleep(700 /* ms */);
	set_image_size_(MODIFIED_IMAGE);
	pPythonPlot_->CopyRawImageData(modified_image_buffer_, modified_width_, modified_height_);
	pPythonPlot_->DrawPlot(kPythonSaveNumpyFunction);
}

/*
	Register image information to instance every time user set image processing option
*/
void ImageProcessing::set_image_size_(ImageType mode) {
	// TODO : Change this modified width
	//int modified_width = kCsiHorizontalResolution / 2 - (kRightBlack + kRightGray + kLeftGray);
	modified_width_ = kCsiHorizontalResolution / 2 - (kCenterBlackLine);
	//int modified_height = kCsiVerticalResolution - (kBottomBlack + kTopBlack);
	modified_height_ = kCsiVerticalResolution - (kBottomBlack + kTopBlack);

	if (mode == MODIFIED_IMAGE) { // Apply modification
		image_width_ = modified_width_;
		image_height_ = modified_height_;
		modified_raw_data_size_ = modified_width_ * 2 * modified_height_;
		ApplyCutEachSide();
	}
	else { // default mode
		image_width_ = kCsiHorizontalResolution / 2;
		image_height_ = kCsiVerticalResolution;
	}
}

/*
	Depending on the type of image type(default, modified), fill image data(high 8bit) to image buffer
*/
void ImageProcessing::fill_bmp_image_data_(ImageType type) {
	set_bmp_header_();
	memcpy(bmp_image_data_, bmp_header_, kBmpHeaderSize);

	// Offset need to inserted to the end of each horizontal line to make the number of horizontal bytes to be multiplies of 4
	// If total horizontal bytes are 4542, we need to add 2 to make 4546 which is multiplies of 4
	int offset_value = ((image_width_ * 3) % 4) ? 4 - (image_width_ * 3) % 4 : 0;
	int offset_count = 0;

	BYTE pixel_value;
	BYTE * pImgData;

	if (type == DEFAULT_IMAGE)
		pImgData = image_buffer_;
	else
		pImgData = modified_image_buffer_;

	int image_count = kBmpHeaderSize;
	for (unsigned int vertical_count = 0; vertical_count < image_height_; vertical_count++) {
		for (unsigned int horizontal_count = 0; horizontal_count < image_width_; horizontal_count++) {
			pixel_value = pImgData[horizontal_count * 2 + vertical_count * image_width_ * 2 + 1];
			bmp_image_data_[image_count + offset_count] = pixel_value;
			bmp_image_data_[image_count + offset_count + 1] = pixel_value;
			bmp_image_data_[image_count + offset_count + 2] = pixel_value;
			image_count += 3; // Fill 3 pixel with same value every interation
		}
		for (int i = 0; i < offset_value; i++) {
			bmp_image_data_[image_count + offset_count] = 0x00;
			offset_count += 1;
		}
	}
}

// Set bmp header array depending on the user setting following bmp structures
void ImageProcessing::set_bmp_header_(void) {
	int offset_value = 4 - (image_width_ * 3) % 4;
	bmp_file_size_ = (image_width_ * 3 + offset_value) * image_height_ + kBmpHeaderSize;

	// Set total file size
	bmp_header_[2] = static_cast<BYTE>(bmp_file_size_);
	bmp_header_[3] = static_cast<BYTE>(bmp_file_size_ >> 8);
	bmp_header_[4] = static_cast<BYTE>(bmp_file_size_ >> 16);
	bmp_header_[5] = static_cast<BYTE>(bmp_file_size_ >> 24);

	// Set image width and heigt
	bmp_header_[18] = static_cast<BYTE>(image_width_);
	bmp_header_[19] = static_cast<BYTE>(image_width_ >> 8);
	bmp_header_[22] = static_cast<BYTE>(image_height_);
	bmp_header_[23] = static_cast<BYTE>(image_height_ >> 8);

	std::cout << "FileSize = " << bmp_file_size_ << "\tWidth = " << image_width_ << "\tHeight = " << image_height_ << std::endl;
}

// Because we get the raw data from camera, there are black and gray area which is not operating parts
// To deal with pixels operating correctly, I applied the cutting each side gray and black
void ImageProcessing::ApplyCutEachSide(void) {
	// TODO(1217)
	// Do we need to find center black line and fix automatically or it is fixed pattern?
	// After fix this problem, I need to make image showing what this function do
	// It seems there are duplicated line at horizontal 146,147 pixel. We need to take another sample picture

	// 1. Cut down side 
	int vertical_offset_bottom = kBottomBlack;

	// 2. Move right part of image to left side
	
	int raw_image_width = image_width_ + kCenterBlackLine;
	int start_offset = (raw_image_width - kRightImageWidth) * 2; // Horizontal start point of right valid image

	for (int vertical_count = 0; vertical_count < image_height_; vertical_count++) {

		// Copy right valid image first to new image buffer 
		for (int horizontal_count = 0; horizontal_count < raw_image_width  * 2 - start_offset; horizontal_count++) {
			BYTE pixel_value = image_buffer_[(horizontal_count + start_offset) + (vertical_count + vertical_offset_bottom)* kCsiHorizontalResolution];
			modified_image_buffer_[horizontal_count + vertical_count * image_width_ * 2] = pixel_value;
		}

		// Copy left valid image next to right image
		for (int horizontal_count = 0; horizontal_count  < start_offset - kCenterBlackLine * 2; horizontal_count++) {
			BYTE pixel_value = image_buffer_[(horizontal_count) + (vertical_count + vertical_offset_bottom)* kCsiHorizontalResolution];
			modified_image_buffer_[kRightImageWidth * 2 + horizontal_count + vertical_count * image_width_ * 2] = pixel_value;
		}
	}

	// 3. Cut right side which is not interested in




	

	/*BYTE pixel_value;
	int horizontal_offset_by_sensor = kLeftGray;
	int vertical_offset_by_sensor = kBottomBlack;
	for(unsigned int vertical_count = 0; vertical_count < image_height_; vertical_count++) {
		for(unsigned int horizontal_count = 0; horizontal_count < image_width_ * 2; horizontal_count++) {
			pixel_value = image_buffer_[(horizontal_offset_by_sensor * 2 + horizontal_count) +
				(vertical_count + vertical_offset_by_sensor) * kCsiHorizontalResolution];

			modified_image_buffer_[horizontal_count + vertical_count * image_width_ * 2] = pixel_value;
		}
	}*/
}

/*--------------------------------------------------------------------------------------------------------------

Helper function

--------------------------------------------------------------------------------------------------------------*/
void ImageProcessing::ShowImageProcessOptions(void) {
	std::cout << "Which image process do you want to do" << std::endl;
	std::cout << "1) Save in bmp format(default)" << std::endl;
	std::cout << "2) Save in bmp format(modified)" << std::endl;
	std::cout << "3) Save in raw data(default)" << std::endl;
	std::cout << "4) Save in raw data(modified)" << std::endl;
	std::cout << "5) Draw by python(modified)" << std::endl;
	std::cout << "6) Save by numpy format 16bit(modified)" << std::endl;
	std::cout << "x) Go to previous menu" << std::endl;
}

/*
	It will make the image array by assembling the image parts from the device
*/
void ImageProcessing::AssembleImageData(BYTE * image_source, WORD image_length) {
	memcpy(image_buffer_ + image_buffer_count_, image_source, image_length);
	image_buffer_count_ += image_length;
}

/*
	Write bmp image data carried in image buffer to file
	file name is the format of day_hour_min_sec.bmp
*/
void ImageProcessing::write_bmp_to_file(void) {
	int current_time = getCurrentDayHourMinuteSec();
	std::string file_name_str = std::to_string(current_time) + std::string(".bmp");
	file_name_str.insert(2, "_");
	file_name_str.insert(5, "_");
	file_name_str.insert(8, "_");
	file_name_str = output_dir + file_name_str;

	std::ofstream bmp_file;
	bmp_file.open(file_name_str, std::ios::binary);

	bmp_file.write((const char *)bmp_image_data_, bmp_file_size_);

	std::cout << "Saved BMP image file name : " << file_name_str << std::endl;
	bmp_file.close();
}

/*
	Write raw data carried in image buffer to file
	file name is the format of day_hour_min_sec
*/
void ImageProcessing::write_raw_data_to_file(ImageType type) {
	std::string file_name_str = GetFileNameDayHourMinSec();
	file_name_str = output_dir + file_name_str;
	std::ofstream raw_file;
	raw_file.open(file_name_str, std::ios::binary);

	const char * pRawData;
	unsigned int image_size;
	if (type == DEFAULT_IMAGE) {
		pRawData = (const char *)image_buffer_;
		image_size = default_raw_data_size_;
	}
	else {
		pRawData = (const char *)modified_image_buffer_;
		image_size = modified_raw_data_size_;
	}
	raw_file.write(pRawData, image_size);

	std::cout << "Saved Raw image data file name : " << file_name_str << "\tFile size : " << image_size << std::endl;
	raw_file.close();
}


/*
	Checking whether the assembling image pieces from device finish or not
*/
BYTE ImageProcessing::isAssembleCompleted() {
	if (image_buffer_count_ == default_raw_data_size_)
		return true;
	else
		return false;
}

/*
	When finish the image processing flow, init each parameter which is used for assembling
*/
void ImageProcessing::initImageProcessOption(void) {
	image_buffer_count_ = 0;
	memset(image_buffer_, 0x00, sizeof(image_buffer_));
	//pPythonPlot_->FinalizePythonThread();
}

/*--------------------------------------------------------------------------------------------------------------

This function is used for unit test

--------------------------------------------------------------------------------------------------------------*/

/*
	It will be used to test image processing function by reading certain raw file.
	The raw file format should be hex array
	It is recommended if you want to fix the how much you should cut each side.
*/
void ImageProcessing::ReadImageData(std::string file_name) {
	FILE * raw_input;
	BYTE c;
	int hex_count = 0;
	int vertical_count = 0;
	int file_size;
	if (!(raw_input = fopen(file_name.c_str(), "rb"))) {
		std::cerr << "There are error in opening file" << std::endl;
		exit(1);
	};

	fseek(raw_input, 0, SEEK_END);
	file_size = ftell(raw_input);

	fseek(raw_input, 0, SEEK_SET);
	hex_count = fread(image_buffer_, file_size, 1, raw_input);
	std::cout << "Succesfully save image with size : " << file_size << std::endl;
	for (int i = 0; i < 20; i++) {
		printf("%02x\t", image_buffer_[i]);
	}
}

std::string ImageProcessing::GetFileNameDayHourMinSec(void) {
	int current_time = getCurrentDayHourMinuteSec();
	std::string file_name_str = std::to_string(current_time);
	file_name_str.insert(4, "_");
	return file_name_str;
}