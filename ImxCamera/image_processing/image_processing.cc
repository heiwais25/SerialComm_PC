#include "stdafx.h"
#include "image_processing.h"

// Todo : 16bit로 저장하기


ImageProcessing::~ImageProcessing() {

}

// It will make the image array by assembling the image parts from the device
void ImageProcessing::AssembleImageData(BYTE * image_source, WORD image_length) {
	// Copy image from the device to image buffer
	memcpy(image_buffer_ + image_buffer_count_, image_source, image_length);

	// Count the image buffer
	image_buffer_count_ += image_length;
}

// After finishing assembling, save it to bmp image
void ImageProcessing::SaveImageToBmp() {
	// For now, it is possible to save only upper 8bits
	set_bmp_header_(kCsiHorizontalResolution / 2, kCsiVerticalResolution);

	// Copy header to bmp header data
	fill_bmp_image_data_();

	// Write bmp array into file
	write_bmp_to_file();

	// Need to initialize instance value
	image_buffer_count_ = 0;
	memset(image_buffer_, 0x00, sizeof(image_buffer_));
}

// Set bmp header array depending on the user setting
void ImageProcessing::set_bmp_header_(int image_width, int image_height) {
	bmp_file_size_ = (image_width * 3 + 2) * image_height + kBmpHeaderSize;
	bmp_image_width_ = image_width;
	bmp_image_height_ = image_height;

	// Set total file size
	bmp_header_[2] = static_cast<BYTE>(bmp_file_size_);
	bmp_header_[3] = static_cast<BYTE>(bmp_file_size_ >> 8);
	bmp_header_[4] = static_cast<BYTE>(bmp_file_size_ >> 16);
	bmp_header_[5] = static_cast<BYTE>(bmp_file_size_ >> 24);
	 
	// Set image width and heigt
	bmp_header_[18] = static_cast<BYTE>(bmp_image_width_);
	bmp_header_[19] = static_cast<BYTE>(bmp_image_width_ >> 8);
	bmp_header_[22] = static_cast<BYTE>(bmp_image_height_);
	bmp_header_[23] = static_cast<BYTE>(bmp_image_height_ >> 8);

	std::cout << "FileSize = " << bmp_file_size_ << "\tWidth = " << bmp_image_width_ << "\tHeight = " << bmp_image_height_ << std::endl;
}


void ImageProcessing::fill_bmp_image_data_(void) {
	memcpy(bmp_image_data_, bmp_header_, kBmpHeaderSize);
	int image_count = kBmpHeaderSize;
	int offset_value = 4 - (bmp_image_width_ * 3) % 4;
	int offset_count = 0;
	BYTE pixel_value;

	for (int vertical_count = 0; vertical_count < bmp_image_height_; vertical_count++) {
		for (int horizontal_count = 0; horizontal_count < bmp_image_width_; horizontal_count++) {
			pixel_value = image_buffer_[horizontal_count * 2 + vertical_count * bmp_image_width_ * 2];
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

// Write bmp image with bmp array
void ImageProcessing::write_bmp_to_file() {
	int current_time = getCurrentDayHourMinuteSec();
	std::string file_name_str = std::to_string(current_time) + std::string(".bmp");
	const char * file_name = file_name_str.c_str();

	if (!(hWriteFile_ = fopen(file_name, "wb"))) {
		std::cerr << "There are error in opening file" << std::endl;
		exit(1);
	}

	int write_count;
	write_count = fwrite(bmp_image_data_, 1, bmp_file_size_, hWriteFile_);
	std::cout << "Saved BMP image file name : " << file_name_str << std::endl;
	fclose(hWriteFile_);
}

BYTE ImageProcessing::isAssembleCompleted() {
	if (image_buffer_count_ == image_size_) 
		return true;
	else 
		return false;
}






