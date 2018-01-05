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

/*===============================================================================================================================================================================================================================================================

Name:			UnpackImageData

Description:	

Parameters:		void

Returns:

Date:			2017-12-29

===============================================================================================================================================================================================================================================================*/
void ImageProcessing::UnpackImageData(void) {
	int flag_even_odd = 0;
	int pixel_count = -1;
	char check_one_pixel = 0;
	int count = 0;
	int unpacked_image_length = collected_image_total_length_ % 3 == 0 ? (collected_image_total_length_ * 4) / 3 :
											((collected_image_total_length_ - 2) * 4) / 3 + 2;
	for (int i = 0; i < unpacked_image_length; i++) {
		if (check_one_pixel % 2 == 0) {
			check_one_pixel = 0;
			pixel_count++;
			flag_even_odd = pixel_count % 2 == 0 ? 0 : 1;
		}
		if (flag_even_odd) { // odd
			image_buffer_[i] = check_one_pixel ? collected_image_buffer_[count++] : (collected_image_buffer_[count - 2] & 0x0f) << 4;
		}
		else { // even
			image_buffer_[i] = check_one_pixel ? collected_image_buffer_[count++] : collected_image_buffer_[count++] & 0xf0;
		}
		check_one_pixel++;
	}
}


/*===============================================================================================================================================================================================================================================================

Name:			ImageModification

Description:	Depending on the image type, it will modify collected image format to original version(8bit)

Parameters:		void

Returns:

Date:			2017-12-29

===============================================================================================================================================================================================================================================================*/
void ImageProcessing::ImageModification(void) {
	int image_buffer_size = 0;
	switch (collected_image_type_)
	{
		case RAW_IMAGE_FORMAT:
			memcpy(image_buffer_, &collected_image_buffer_[0], collected_image_total_length_);
			// Anything to do
			break;

		case PACKED_RAW_IMAGE_FORMAT:
			UnpackImageData();
			break;

		case PACKED_PNG_IMAGE_FORMAT:
			image_buffer_size = collected_image_total_length_;
			DecodeImageData(collected_image_buffer_, image_buffer_size);
			collected_image_total_length_ = image_buffer_size;
			UnpackImageData();
			break;

		default:
			break;
	}
	std::cout << "Image modification finished" << std::endl;
}

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
					SaveInBitmapImage(RAW_IMAGE);
					break;

				case '2':
					SaveInBitmapImage(MODIFIED_IMAGE);
					break;

				case '3':
					SaveInRawFormat(RAW_IMAGE);
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
	modified_width_ = kCsiHorizontalResolution / 2 - (kCenterBlackLine);

	// Height position can be different
	modified_height_ = kEffectiveImageHeight;


	// modified_height_ = kCsiVerticalResolution;

	if (mode == MODIFIED_IMAGE) { // Apply modification
		GetBlackLineStartPoint();
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
	//int offset_value = ((image_width_ * 3) % 4) ? 4 - (image_width_ * 3) % 4 : 0;
	int offset_value = (image_width_ % 4) ? 4 - (image_width_) % 4 : 0;
	int offset_count = 0;

	BYTE pixel_value;
	BYTE * pImgData;

	if (type == RAW_IMAGE)
		pImgData = image_buffer_;
	else
		pImgData = modified_image_buffer_;

	int image_count = kBmpHeaderSize;

	// Fill first format of b/w image
	for (int i = 0; i < 256; i++) {
		bmp_image_data_[image_count++] = (unsigned char)i;
		bmp_image_data_[image_count++] = (unsigned char)i;
		bmp_image_data_[image_count++] = (unsigned char)i;
		bmp_image_data_[image_count++] = (unsigned char)0;
	}

	for (unsigned int vertical_count = 0; vertical_count < image_height_; vertical_count++) {
		for (unsigned int horizontal_count = 0; horizontal_count < image_width_; horizontal_count++) {
			pixel_value = pImgData[horizontal_count * 2 + vertical_count * image_width_ * 2 + 1];
			bmp_image_data_[image_count + offset_count] = pixel_value;
			image_count += 1; // Fill 3 pixel with same value every interation
		}
		for (int i = 0; i < offset_value; i++) {
			bmp_image_data_[image_count + offset_count] = 0x00;
			offset_count += 1;
		}
	}
}

// Set bmp header array depending on the user setting following bmp structures
void ImageProcessing::set_bmp_header_(void) {
	int offset_value = (image_width_ % 4) ? 4 - (image_width_ * 3) % 4 : 0;
	bmp_file_size_ = (image_width_ + offset_value) * image_height_ + kBmpHeaderSize + 1024;

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

/*===============================================================================================================================================================================================================================================================

Name:			GetBlackLineStartPoint

Description:	There are invalid region somewhere in the image. It seems the starting point is not fixed. So, this function will find the invalid image region and return
			    the offset of invalid image region. The region is consist of gray + black + gray line and total width is 56

Parameters:		void

Returns:		(int)start offset of invalid image region

===============================================================================================================================================================================================================================================================*/
int ImageProcessing::GetBlackLineStartPoint(void) {
	set_image_size_(RAW_IMAGE);
	
	bool kIsBlackLine = false;
	int black_pixel_count = 0;
	int original_image_width = image_width_;
	int black_pixel_start_pixel;
	int sample_vi = 400;

	for (int hi = 0; hi < original_image_width; hi++) {
		BYTE high_byte = image_buffer_[hi * 2 + sample_vi * original_image_width + 1];
		//if (high_byte < 0x5) { // 0x5 : Proper value indicating black(invalid) pixel
		if (high_byte < 0x5) { // 0x5 : Proper value indicating black(invalid) pixel
			kIsBlackLine = true;
			black_pixel_count++;
		}

		if (kIsBlackLine && high_byte >= 0x5) {
			kIsBlackLine = false;
			if (black_pixel_count == kBlackPixelWidth - 1) {
				// Sometime, the last pixel value in black line is larger than 0x5
				// Because first 20 pixel value should be 0, so it is okay to check first 20 pixels
				black_pixel_start_pixel = hi + 1 - kBlackPixelWidth;
				break;
			}
			else {
				black_pixel_count = 0;
			}
		}
	}

	// invalid_pixel_offset_ : Offset of gray+black pixel
	invalid_pixel_offset_ = black_pixel_start_pixel - kLeftGrayPixelWidth;
	return black_pixel_start_pixel;
}

/*===============================================================================================================================================================================================================================================================

Name:			GetBlackPixelHeightFromBottom

Description:	Calculate how many black pixel exist from bottom

Parameters:		horizontal pixel : the horizontal offset we want to calculate the black pixel height
		
Returns:		(int) height

===============================================================================================================================================================================================================================================================*/
int ImageProcessing::GetBlackPixelHeightFromBottom(int horizontal_pixel) {
	int vi = 0;
	while (image_buffer_[horizontal_pixel + vi * kCsiHorizontalResolution + 1] > 0x5) 
		vi++;
	return vi;
}

/*===============================================================================================================================================================================================================================================================

Name:			GetChoppedImageOffset

Description:

Parameters:

Returns:

===============================================================================================================================================================================================================================================================*/
int ImageProcessing::GetChoppedImageOffset() {
	int start_offset = (invalid_pixel_offset_ + kCenterBlackLine) * 2;
	if (start_offset / 2 > original_width_) {
		std::cout << "Calculated offset is " << start_offset << std::endl;
		std::cout << "Error on getting chopped image offset" << std::endl;
		return -1;
	}

	if (start_offset == original_width_ * 2)
		start_offset = 0;
	return start_offset;
}

/*===============================================================================================================================================================================================================================================================

Name:			GetChoppedImageOffset

Description:

Parameters:

Returns:

===============================================================================================================================================================================================================================================================*/
int ImageProcessing::GetChoppedImageLength(int horizontal_offset) {
	int black_pixel_length_from_bottom;
	int chopped_image_length;
	bool kIsBlackStart = false;
	if (horizontal_offset == 0) {
		for (int hi = horizontal_offset; hi < original_width_ * 2; hi++) {
			int vi = 0;
			BYTE high_byte = image_buffer_[hi * 2 + 1];
			if (high_byte < 0x5) {
				while (image_buffer_[hi * 2 + 1 + vi * kCsiHorizontalResolution] < 0x5) {
					vi++;
				}
				if (kIsBlackStart && vi != black_pixel_length_from_bottom) {
					chopped_image_length = hi;
					break;
				}
				else {
					kIsBlackStart = true;
					black_pixel_length_from_bottom = vi;
				}
			}
		}
	}
	else {
		chopped_image_length = (original_width_ - horizontal_offset / 2);
	}
	return chopped_image_length;
}

/*===============================================================================================================================================================================================================================================================

Name:			ApplyCutEachSide

Description:	This function is required because the position of valid image is fluctuate little bit
				In this function, it will find invalid line area, chopped image and main image position and re-arrange them to correct image

Parameters:		void

Returns:		void

===============================================================================================================================================================================================================================================================*/
void ImageProcessing::ApplyCutEachSide(void) {
	// Calculate chopped image offset
	int chopped_image_horizontal_offset = GetChoppedImageOffset();

	// 1. Find the vertical offset of first black line
	int black_pixel_vertical_offset = GetBlackPixelHeightFromBottom((invalid_pixel_offset_ + kLeftGrayPixelWidth)*2);

	// 2. Calculate the vertical offset of chopped image
	//    Usually, the difference between both line is 13
	int chopped_image_vertical_offset = black_pixel_vertical_offset + 13;

	// 3. Calculate chopped image length
	int chopped_image_length = GetChoppedImageLength(chopped_image_horizontal_offset);

	std::cout << "Chopped Image Information--------------------------------------------------" << std::endl;
	std::cout << "Horizontal offset : " << chopped_image_horizontal_offset << std::endl;
	std::cout << "Vertical offset   : " << chopped_image_vertical_offset << std::endl;
	std::cout << "Image length      : " << chopped_image_length << std::endl;
	std::cout << "---------------------------------------------------------------------------" << std::endl;

	// 4. Set main image offset information
	// Usually, the difference of the vertical offset between main image and chopped image is 1
	int main_image_horizontal_offset = chopped_image_horizontal_offset == 0 ? (chopped_image_length) * 2 : 0;
	int main_image_vertical_offset = chopped_image_vertical_offset + 1;
	int main_image_vertical_offset_bottom = main_image_vertical_offset;
	int main_image_vertical_offset_top = main_image_vertical_offset + kEffectiveImageHeight;
	int main_image_length = chopped_image_horizontal_offset == 0 ? invalid_pixel_offset_ - chopped_image_length : invalid_pixel_offset_;
	
	for (int vertical_count = 0; vertical_count < kEffectiveImageHeight; vertical_count++) {
		// Copy right image from invalid region first to new image buffer 
		for (int horizontal_count = 0; horizontal_count < chopped_image_length * 2; horizontal_count++) {
			BYTE pixel_value = image_buffer_[(chopped_image_horizontal_offset + horizontal_count) + \
								(vertical_count + main_image_vertical_offset_bottom - 1)* kCsiHorizontalResolution];
			modified_image_buffer_[horizontal_count + vertical_count * image_width_ * 2] = pixel_value;
		}

		// Copy left valid image next to right image
		for (int horizontal_count = 0; horizontal_count  < main_image_length * 2; horizontal_count++) {
			BYTE pixel_value = image_buffer_[(horizontal_count + main_image_horizontal_offset) + \
									(vertical_count + main_image_vertical_offset_bottom)* kCsiHorizontalResolution];
			modified_image_buffer_[chopped_image_length * 2 + horizontal_count + vertical_count * image_width_ * 2] = pixel_value;
		}
	}
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


void ImageProcessing::SetImageTotalLength(unsigned int image_total_length) {
	collected_image_total_length_ = image_total_length;
}

void ImageProcessing::InitImageBuffer() {
	SetImageTotalLength(0);
	collected_image_buffer_.clear();
}

void ImageProcessing::SetImageType(CollectedImageFormat type) {
	collected_image_type_ = type;
}

/*===============================================================================================================================================================================================================================================================

Name:				AssembleImageData

Description:		It will make the image array by assembling the image parts from the device

Parameters:			BYTE * image_source 
					WORD image_length

Returns:			void

Date:				2017-12-29

===============================================================================================================================================================================================================================================================*/
void ImageProcessing::AssembleImageData(BYTE * image_source, WORD image_length) {
	if (collected_image_total_length_ == 0) {
		std::cout << "Please set image total length first" << std::endl;
		return;
	}
	std::vector<BYTE> temp_vec(&image_source[0], &image_source[image_length]);
	collected_image_buffer_.insert(collected_image_buffer_.end(), temp_vec.begin(), temp_vec.end());
	//memcpy(collected_image_buffer_ + image_buffer_count_, image_source, image_length);
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
	if (type == RAW_IMAGE) {
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
	//if (image_buffer_count_ == default_raw_data_size_)
	if (image_buffer_count_ == collected_image_total_length_)
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