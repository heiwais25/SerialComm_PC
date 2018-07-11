#include "stdafx.h"
#include "image_processing.h"


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
	collected_image_total_length_ = image_width_from_png_ * image_height_from_png_;
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
	collected_image_total_length_ = image_width_from_png_ * 4 / 3 * image_height_from_png_;
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
	switch (collected_image_type_) {
		case RAW_IMAGE_FORMAT:
			memcpy(image_buffer_, &collected_image_buffer_[0], collected_image_total_length_);
			// Anything to do
			break;

		case PIXEL_LOG_FORMAT:
			memcpy(image_buffer_, &collected_image_buffer_[0], collected_image_total_length_);
			kIsPixelLog = true;
			break;

		case CUT_OFF_IMAGE_FORMAT:
			memcpy(image_buffer_, &collected_image_buffer_[0], collected_image_total_length_);
			break;
			
		case PACKED_RAW_IMAGE_FORMAT:
			UnpackImageData();
			break;

		case PIXEL_INFO_FORMAT:
			memcpy(image_buffer_, &collected_image_buffer_[0], collected_image_total_length_);
			break;

		case PACKED_PNG_IMAGE_FORMAT:
			DecodeImageData(collected_image_buffer_);
			GetImageWidthHeight(image_width_from_png_, image_height_from_png_);
			// Choose proper imgType among several options(1. real img, 2. pixel info)
			// 1. Pixel info
			if (image_width_from_png_ == kEffectiveImageWidth * 3 || image_width_from_png_ == kEffectiveImageWidth * 4) {
				UnpackPixelInfo();
			}
			// 2. Image info
			else {
				UnpackImageData();
			}
			break;

		default:
			break;
	}
	cout << "Image modification finished" << endl;
}

void ImageProcessing::SetFileName(CameraParams cameraParams) {
	stringstream stream;
	stream << hex << cameraParams.adcMinimum;
	string strAdcMinimum = "0x" + stream.str();
	// Case 1 : save individual image
	if (cameraParams.numPictures == 1) {
		fileNameWithParam = "cds_" + to_string(cameraParams.cds) + "_vga_" + to_string(cameraParams.vga) + "_exp_" + \
			to_string(cameraParams.exposureTime) + "_led_" + to_string(cameraParams.ledTime) + "_adc_" + strAdcMinimum + "_";
	}
	// Case 2 : save pixel information
	else {
		string pixelInfoName;
		if (kPixelInfoOrder == 0) {
			kPixelInfoOrder++;
			pixelInfoName = "pixel_mean";
		}
		else {
			kPixelInfoOrder = 0;
			pixelInfoName = "pixel_std";
		}
		fileNameWithParam = pixelInfoName + "_cds_" + to_string(cameraParams.cds) + "_vga_" + to_string(cameraParams.vga) + "_exp_" + \
			to_string(cameraParams.exposureTime) + "_led_" + to_string(cameraParams.ledTime) + "_adc_" + strAdcMinimum + "_";
		kIsPixelLog = true;
	}

	// Check the file is already exist or not
	int fileCount = 0;
	while (1) {
		if (!isFileExist(output_dir + fileNameWithParam + to_string(fileCount))) {
			fileNameWithParam += to_string(fileCount);
			break;
		}
		/*else {
			while (1) {
				cout << "File is already existing" << endl;
			}
		}*/
		fileCount++;
	}
}


void ImageProcessing::SetFileName(string paramName, int paramVal, int expVal, int count) {
	string paramValStr = to_string(expVal);
	if (paramName == "adcMinimum") {
		stringstream stream;
		stream << hex << paramVal;
		paramValStr = "0x" + stream.str();
	}
	fileNameWithParam = "exp_" + to_string(expVal) + "_" + paramName + "_" + paramValStr + "_" + to_string(count);
	cout << "File name : " << fileNameWithParam << endl;
}

void ImageProcessing::UnpackPixelInfo() {
	if (image_width_from_png_ == kEffectiveImageWidth) return;
	int width = image_width_from_png_, height = image_height_from_png_;
	int oldCount = 0, newCount = 0;
	while (oldCount < width * height) {
		image_buffer_[newCount++] = collected_image_buffer_[oldCount++];
		if ((newCount + 1) % 4 == 0) {
			image_buffer_[newCount++] = 0;
		}

	}
	collected_image_total_length_ = newCount;
	cout << width << " x " << height << endl;
}



/* =======================================================================================================
	Description
	- It will save pixel log that recorded 12 pixel individually
=========================================================================================================*/
void ImageProcessing::SavePixelLog(void) {
	std::string file_name_str = GetFileNameDayHourMinSec();
	file_name_str = output_dir + file_name_str;
	if (fileNameWithParam.size() != 0) {
		file_name_str = output_dir + fileNameWithParam;
	}
	std::ofstream raw_file;
	raw_file.open(file_name_str, std::ios::binary);

	raw_file.write((const char *)image_buffer_, collected_image_total_length_);

	cout << "Saved Pixel Log : " << file_name_str << "\tFile size : " << collected_image_total_length_ << endl;
	raw_file.close();
}

void ImageProcessing::ChooseImageProcessOption(void) {
	if (kIsPixelLog) {
		SavePixelLog();
		fileNameWithParam = "";
		initImageProcessOption();
		kIsPixelLog = false;
		return;
	}

	if (fileNameWithParam.size() != 0) {
		// save raw data
		SaveInBitmapImage(MODIFIED_IMAGE);
		SaveInRawFormat(MODIFIED_IMAGE);
		fileNameWithParam = "";
		initImageProcessOption();
		return;
	}

	BYTE c;
	ShowImageProcessOptions();
	while (1) {
		c = GetOneCharKeyboardInput();
		if (c == 'x' || c == 'X') {
			initImageProcessOption();
			cout << "Exit" << endl;
			break;
		}

		if (isDecimalNumber(c)) {
			switch (c) {
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

				case '7':
					SaveInBitmapImage(CUT_OFF_IMAGE);
					break;

				case '8':
					SavePixelData();
					break;

				default:
					break;
			}
		}
	}
}

/* =========================================================================================
	Description
	- Save pixel data like mean and deviation
========================================================================================= */
void ImageProcessing::SavePixelData() {
	setImgSize(PIXEL_INFO);
	write_raw_data_to_file(PIXEL_INFO);
}

/*
	Save the image data by bitmap format to see clearly
	Due to easy approach, current bitmap is only using high 8bit
*/
void ImageProcessing::SaveInBitmapImage(ImgType type) {
	setImgSize(type);

	fill_bmp_image_data_(type);

	write_bmp_to_file();
}

/*
	To deal with data more in detail, it is necessary to save data by raw format.
	It will be used for other analysis tool
*/
void ImageProcessing::SaveInRawFormat(ImgType type) {
	setImgSize(type);

	write_raw_data_to_file(type);
}

// todo....
void ImageProcessing::PlotInPython(void) {
	Sleep(700 /* ms */);
	setImgSize(MODIFIED_IMAGE);
	pPythonPlot_->CopyRawImageData(modified_image_buffer_, modified_width_, modified_height_);
	pPythonPlot_->DrawPlot(kPythonPlotFunction);
}

void ImageProcessing::SaveInNumpyFormat(void) {
	Sleep(700 /* ms */);
	setImgSize(MODIFIED_IMAGE);
	pPythonPlot_->CopyRawImageData(modified_image_buffer_, modified_width_, modified_height_);
	pPythonPlot_->DrawPlot(kPythonSaveNumpyFunction);
}


/*
	Register image information to instance every time user set image processing option
*/

/* ===================================================================================================
	Description
	- Set image size for each image type mode
=================================================================================================== */
void ImageProcessing::setImgSize(ImgType mode) {
	// TODO : Change this modified width
	modified_width_ = kEffectiveImageWidth;

	// Height position can be different
	modified_height_ = kEffectiveImageHeight;

	// Not cut off version
	if (collected_image_total_length_ != kEffectiveImageWidth * kEffectiveImageHeight * 2) {
		if (mode == MODIFIED_IMAGE) {
			GetBlackLineStartPoint();
			image_width_ = modified_width_;
			image_height_ = modified_height_;
			modified_raw_data_size_ = image_width_ * 2 * image_height_;
			ApplyCutEachSide();
		}
		else {
			image_width_ = kCsiHorizontalResolution / 2;
			image_height_ = kCsiVerticalResolution;
		}
	}
	// Cut off version
	else {
		if (mode == MODIFIED_IMAGE) {
			memcpy(modified_image_buffer_, image_buffer_, collected_image_total_length_);
		}
		image_width_ = kEffectiveImageWidth;
		image_height_ = kEffectiveImageHeight;
		modified_raw_data_size_ = image_width_ * image_height_ * 2;
	}

	if (mode == PIXEL_INFO) {
		memcpy(modified_image_buffer_, image_buffer_, collected_image_total_length_);
		image_width_ = kEffectiveImageWidth;
		image_height_ = kEffectiveImageHeight;
		modified_raw_data_size_ = image_width_ * image_height_ * 4;
	}

}

/*
	Depending on the type of image type(default, modified), fill image data(high 8bit) to image buffer
*/
void ImageProcessing::fill_bmp_image_data_(ImgType type) {
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
	else if(type == CUT_OFF_IMAGE)
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

	cout << "FileSize = " << bmp_file_size_ << "\tWidth = " << image_width_ << "\tHeight = " << image_height_ << endl;
}

/*===============================================================================================================================================================================================================================================================

Name:			GetBlackLineStartPoint

Description:	There are invalid region somewhere in the image. It seems the starting point is not fixed. So, this function will find the invalid image region and return
			    the offset of invalid image region. The region is consist of gray + black + gray line and total width is 56

Parameters:		void

Returns:		(int)start offset of invalid image region

===============================================================================================================================================================================================================================================================*/
int ImageProcessing::GetBlackLineStartPoint(void) {
	setImgSize(RAW_IMAGE);
	ImgFixInfo imgFixInfo;
	bool kIsBlackLine = false;
	int black_pixel_count = 0;
	int original_image_width = image_width_;
	int black_pixel_start_pixel;
	int sample_vi = 400;

	minimum_value_ = 255;
	for (int hi = 0; hi < original_image_width; hi++) {
		BYTE high_byte = image_buffer_[hi * 2 + sample_vi * original_image_width + 1];

		if (high_byte < minimum_value_) {
			minimum_value_ = high_byte;
		}
	}
	cout << "Minimum value at 400th vertical line : " << (int)minimum_value_ << endl;

	for (int hi = 0; hi < original_image_width; hi++) {
		BYTE high_byte = image_buffer_[hi * 2 + sample_vi * original_image_width + 1];
		if (high_byte < minimum_value_ + kBlakcPixelVariation) { // 0x5 : Proper value indicating black(invalid) pixel
			kIsBlackLine = true;
			black_pixel_count++;
		}
		else {
			kIsBlackLine = false;
			black_pixel_count = 0;
		}
	
		//if (kIsBlackLine && high_byte >= 0x20) {
		if (kIsBlackLine && black_pixel_count == kBlackPixelWidth - 1) {
			kIsBlackLine = false;
			black_pixel_start_pixel = hi + 2 - kBlackPixelWidth;
			break;
		}
	}
	for (int i = 0; i<kBlackPixelWidth; i++)
		printf("%02x ", image_buffer_[(black_pixel_start_pixel + i) * 2 + sample_vi * original_image_width * 2 + 1]);
	printf("\n");

	// invalid_pixel_offset_ : Offset of gray+black pixel
	invalid_pixel_offset_ = black_pixel_start_pixel - kLeftGrayPixelWidth;
	cout << "Invalid pixel offset : " << invalid_pixel_offset_ << endl;
	imgFixInfo.blackLineHorizontalOffset = black_pixel_start_pixel;
	imgFixInfo.invalidPixelOffset = invalid_pixel_offset_;
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
	while (image_buffer_[horizontal_pixel + vi * kCsiHorizontalResolution + 1] > minimum_value_ + kBlakcPixelVariation)
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
		cout << "Calculated offset is " << start_offset << endl;
		cout << "Error on getting chopped image offset" << endl;
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
			if (high_byte < minimum_value_ + kBlakcPixelVariation) {
				while (image_buffer_[hi * 2 + 1 + vi * kCsiHorizontalResolution] < minimum_value_ + kBlakcPixelVariation) {
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
	int black_pixel_vertical_offset = GetBlackPixelHeightFromBottom((invalid_pixel_offset_ + kLeftGrayPixelWidth + 5)*2);
	cout << "Vertical offset : " << black_pixel_vertical_offset << endl;
	// 2. Calculate the vertical offset of chopped image
	//    Usually, the difference between both line is 13
	int chopped_image_vertical_offset = black_pixel_vertical_offset + 13;

	// 3. Calculate chopped image length
	int chopped_image_length = GetChoppedImageLength(chopped_image_horizontal_offset);

	cout << "Chopped Image Information--------------------------------------------------" << endl;
	cout << "Horizontal offset : " << chopped_image_horizontal_offset << endl;
	cout << "Vertical offset   : " << chopped_image_vertical_offset << endl;
	cout << "Image length      : " << chopped_image_length << endl;
	cout << "---------------------------------------------------------------------------" << endl;

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
			BYTE pixel_value = image_buffer_[(chopped_image_horizontal_offset + 2 + horizontal_count) + \
								(vertical_count + main_image_vertical_offset_bottom - 1)* kCsiHorizontalResolution];
			//printf("%x ", pixel_value);
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
	cout << "Which image process do you want to do" << endl;
	cout << "1) Save in bmp format(default)" << endl;
	cout << "2) Save in bmp format(modified)" << endl;
	cout << "3) Save in raw data(default)" << endl;
	cout << "4) Save in raw data(modified)" << endl;
	cout << "5) Draw by python(modified)" << endl;
	cout << "6) Save by numpy format 16bit(modified)" << endl;
	cout << "7) Save in bmp format(cut off)" << endl;
	cout << "8) Save pixel info" << endl;
	cout << "x) Go to previous menu" << endl;
}


void ImageProcessing::SetImageTotalLength(unsigned int image_total_length) {
	collected_image_total_length_ = image_total_length;
}

void ImageProcessing::InitImageBuffer() {
	SetImageTotalLength(0);
	collected_image_buffer_.clear();
}

void ImageProcessing::SetImgType(CollectedImageFormat type) {
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
		cout << "Please set image total length first" << endl;
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
	if (fileNameWithParam.size() != 0) {
		file_name_str = output_dir + fileNameWithParam + ".bmp";
	}
	bmp_file.open(file_name_str, std::ios::binary);

	bmp_file.write((const char *)bmp_image_data_, bmp_file_size_);

	cout << "Saved BMP image file name : " << file_name_str << endl;
	bmp_file.close();
}

/*
	Write raw data carried in image buffer to file
	file name is the format of day_hour_min_sec
*/
void ImageProcessing::write_raw_data_to_file(ImgType type) {
	std::string file_name_str = GetFileNameDayHourMinSec();
	file_name_str = output_dir + file_name_str;
	std::ofstream raw_file;
	if (fileNameWithParam.size() != 0) {
		file_name_str = output_dir + fileNameWithParam;
	}
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

	cout << "Saved Raw image data file name : " << file_name_str << "\tFile size : " << image_size << endl;
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
		std::cerr << "There are error in opening file" << endl;
		exit(1);
	};

	fseek(raw_input, 0, SEEK_END);
	file_size = ftell(raw_input);

	fseek(raw_input, 0, SEEK_SET);
	hex_count = fread(image_buffer_, file_size, 1, raw_input);
	cout << "Succesfully save image with size : " << file_size << endl;
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