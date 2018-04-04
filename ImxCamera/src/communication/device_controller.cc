#include "stdafx.h"
#include "device_controller.h"



void DeviceController::PickAndSendCommand(void) {
	BYTE c;
	ShowTestOptions();
	while (1) {

		if (c = GetOneCharKeyboardInput()) {
			if (c == 'x')
				break;

			ControlDevice(c);

			if (isWaitResponse()) 
				break; // It sends command, so let's wait for its response
		}
	}
}

/*----------------------------------------------------------------------------------------------------------

Sending operation

----------------------------------------------------------------------------------------------------------*/
/*
	Depending on user input, it will send command or do some process
*/
void DeviceController::ControlDevice(BYTE c) {
	// init_comm_state();
	IndicatePacketState(PACKET_EMPTY);
	switch (c) {
		case '1':
			EchoTest();
			break;

		case '2':
			CameraTest();
			break;

		case '3':
			// In future?
			break;
	}
}

/*
	The purpose of this test is checking communication with PC and device is well
	It will send one character and device will check that and reply same character
*/
void DeviceController::EchoTest() {
	BYTE c;
	ShowEchoTestOptions();	
	while (1) {
		if (c = GetOneCharKeyboardInput())
			break;
	}
	SetSendingPacketInfo(0x00, ECHO_TEST, 0x01, &c);
	SendPacket();
}



/*
	Do camera test including taking picture and some gain setting
*/
void DeviceController::CameraTest(void) {
	ShowCameraOperationOption();
	ChooseCameraOperation();
}

void DeviceController::DoControlCamera(void) {
	ShowControlCameraOption();
	BYTE c = GetOneChar();
	switch (c) {
	case '1':
		SendCommand(CAMERA_POWER_ON);
		break;

	case '2':
		SendCommand(CAMERA_POWER_OFF);
		break;

	case '3':
		SendCommand(CAMERA_CAPTURE);
		break;

	case '4':
		ChangeCameraOption();
		break;

	case '5':
		SendCommand(CAMERA_CAPTURE_AND_TRANSMIT);
		break;

	case '6':
		captureContinuously();
		break;

	case 'x':
		break;

	default:
		break;
	}
}



/*===============================================================================================================================================================================================================================================================

Name			:	captureContinuously

Description		:	Read camera parameter from paramSet file and send it to main board

Parameters		:

Returns			:	void

Date			:

===============================================================================================================================================================================================================================================================*/
void DeviceController::captureContinuously() {
	// 1. Read camera params from paramSet
	int paramNum = 8;
	if (paramsQueue.empty()) {
		paramsQueue = getParamFromCSV("paramSet/paramSetInfo.prn", paramNum);
		if (paramsQueue.empty()) {
			cout << "No data in the paramSet" << endl;
			return;
		}

		if (paramsQueue.front().size() != paramNum) {
			cout << "Wrong reading function" << endl;
			return;
		}
		paramsToSendNum = paramsQueue.size();
	}
	

	// 2. Copy params to currentParam
	auto params = paramsQueue.front();
	currentParams.cds = params["cds"];
	currentParams.vga = params["vga"];
	currentParams.blackLevel = params["blackLevel"];
	currentParams.exposureTime = params["exposureTime"];
	currentParams.ledTime = params["ledTime"];
	currentParams.adcMinimum = params["adcMinimum"];

	auto const pCurrentParams = reinterpret_cast<BYTE*>(&currentParams);
	vector<BYTE> paramToSend(pCurrentParams, pCurrentParams + sizeof(currentParams));
	SetSendingPacketInfo(0, CAMERA_SEND_CAMERA_PARAMS, paramToSend);
	SendPacket();
	saveLastCameraParam(params["mode"], params["count"]);
}


void DeviceController::DoImageTransmission(void) {
	ShowImageTransmissionOption();
	BYTE c = GetOneChar();
	switch (c) {
		case '1':
			SendCommand(CAMERA_SEND_CAPTURED_IMAGE);
			break;

		case '2':
			SendCommand(CAMERA_SEND_PACKED_DATA);
			break;

		case '3':
			SendCommand(CAMERA_SEND_PNG);
			break;

		case '4':
			SendCommand(CAMERA_SEND_CUT_OFF_IMAGE);
			break;

		case '5':
			SendCommand(CAMERA_SEND_PACKED_PNG_CUT_OFF_IMAGE);
			break;

		case '6':
			SendCommand(COMMAND_CAMERA_SEND_LAST_ACTIVATED_MODE);
			break;

		case '7':
			SendCommand(CAMERA_SEND_PIXEL_MEAN);
			break;

		case '8':
			SendCommand(CAMERA_SEND_PIXEL_DEV);
			break;

		case '9':
			SendCommand(CAMERA_SEND_PIXEL_LOG);
			break;

		case 'x':
			break;

		default:
			break;
	}
}

/*===============================================================================================================================================================================================================================================================

Name			:	DoImageProcess

Description		:	

Parameters		:	

Returns			:	void

Date			:	

===============================================================================================================================================================================================================================================================*/
void DeviceController::DoImageProcess(void) {	
	ShowImageProcessOption();
	BYTE c = GetOneChar();
	switch (c) {
		case '1':
			SendCommand(PACK_IMAGE);
			break;

		case '2':
			SendCommand(PACK_CONVERT_PNG);
			break;

		case '3':
			SendCommand(CUT_IMAGE);
			break;

		case '4':
			SendCommand(PACK_CUT_OFF_CONVERT_PNG);
			break;

		case '5':
			SendCommand(SAVE_IMAGE);
			break;

		case '6':
			SetImageToRead();
			break;

		case '7':
			SendCommand(SHOW_STORED_IMAGE_LIST);
			break;

		case '8':
			SendCommand(DELETE_LAST_NODE);
			break;

		case '9':
			SendCommand(CONVERT_MEAN_PNG);
			break;

		case 'a':
			SendCommand(CONVERT_DEV_PNG);
			break;

		case 'x':
			break;

		default:
			break;
	}
}

void DeviceController::DoControlLED() {
	ShowLEDOption();
	BYTE c;
	while (1) {
		if (c = GetOneCharKeyboardInput())
			break;
	}
	switch (c) {
		case '1':
			SendCommand(TURN_ON_LED);
			break;

		case '2':
			SendCommand(TURN_OFF_LED);
			break;

		case '3':
			BlankLED();
			break;

		case 'x':
			break;

		default:
			break;
	}
}

void DeviceController::BlankLED(void) {
	cout << "Blank the LED with fixed period" << endl;
	int setting_value = getValueLowerThanMaximum(4095);
	unsigned char value[2];
	value[0] = setting_value & 0xff;
	value[1] = (setting_value >> 8) & 0xff;
	SetSendingPacketInfo(0x00, BLANK_LED, 0x02, value);
	SendPacket();
}


/*
	It will choose operation mode among 3 main operaion modes
	1. Control camera
	2. Image transmission
	3. Image process
*/
void DeviceController::ChooseCameraOperation(void) {
	BYTE c = GetOneChar();
	switch (c) {
		case '1':
			DoControlCamera();
			break;

		case '2':
			DoImageTransmission();
			break;

		case '3':
			DoImageProcess();
			break;

		case '4':
			DoControlLED();

		case 'x':
			return;
	}
}

// Fine setting of camera option list. These options are affecting when we capture the image




void DeviceController::SetImageToRead() {
	cout << "Select the position of image stored at EMMC" << endl;
	int setting_value = getValueLowerThanMaximum(1024);
	SendShortValue(READ_IMAGE, setting_value);
}





/*----------------------------------------------------------------------------------------------------------

Receiving Operation

----------------------------------------------------------------------------------------------------------*/
/*
	When we got command from device, this function will choose how PC will respond
*/
void DeviceController::DoCommand(void) {
	received_packet_data_length = (hReceivedPacket.length[1] << 8) + hReceivedPacket.length[0];
	switch (hReceivedPacket.command) {
		case GOOD_COMMUNICATION: 
			break;

		case ECHO_TEST:
			CheckEchoTest();
			break;

		case CAMERA_SEND_PIXEL_LOG:
			pImageProcessing_->SetImgType(PIXEL_LOG_FORMAT);
			pImageProcessing_->SetImageTotalLength(received_packet_data_length);
			pImageProcessing_->AssembleImageData(hReceivedPacket.data, received_packet_data_length);
			pImageProcessing_->ImageModification();
			pImageProcessing_->ChooseImageProcessOption();
			pImageProcessing_->InitImageBuffer();
			break;

		case CAMERA_SEND_CAPTURED_IMAGE: // Because the image packet is seperated, we need to collect this image pieces
		case CAMERA_SEND_PACKED_DATA:
		case CAMERA_SEND_PNG:
		case CAMERA_SEND_CUT_OFF_IMAGE:
		case CAMERA_SEND_PIXEL_MEAN:
		case CAMERA_SEND_PIXEL_DEV:
			CollectImageData();
			break;

		default:
			printf("%x\n", hReceivedPacket.command);
			break;
	}
}



void DeviceController::CheckEchoTest() {
	cout << "Send" << "\t" << "Received" << endl;
	for (int i = 0; i < received_packet_data_length; i++) {
		cout << std::hex << hSendingPacket.data[i] << "\t" << std::hex << hReceivedPacket.data[i] << endl;
		if (hReceivedPacket.data[i] - hSendingPacket.data[i]) {
			std::cerr << "Echo test error" << endl;
		}
	}
	cout << "Echo test Finish" << endl;
}


/* ===================================================================================================
	Description
	- Due to captured image is trasmitted sepearate packet, we need to collect image data and make it original one
	- To do this, we will use ImageProcessing class
	- steps	
		1) Check packet order(number)
		2) AssembleImageData
		3) If the transmission is finished, processing image else request next frame
	- This function will be continued until the image transmission is finished
=================================================================================================== */
void DeviceController::CollectImageData(void) {
	// 1. Get packet number
	unsigned short packet_number = (hReceivedPacket.number[1] << 8) + hReceivedPacket.number[0];
	static unsigned int data_total_length = 0;
	static unsigned int received_total_length_ = 0;
	static int fileCount = 0;
	if (image_piece_number_ != packet_number) {
		std::cerr << "The order of data is packet is wrong" << endl;
		// ERROR HANDLING WHEN THE DATA IS CRASHED
		return;
	}
	else
		image_piece_number_++;

	// First packet will include image information
	if (packet_number == 0) {
		BYTE * data = hReceivedPacket.data;
		data_total_length = (data[3] << 24) + (data[2] << 16) + (data[1] << 8) + data[0];
		auto img_format = GetCorrectImgFormat(data_total_length, hReceivedPacket.command);

		// The image is taken by sending camera parameters
		if (paramsQueue.empty() == 0) {
			cout << "count : " << lastCameraParam.count;
			pImageProcessing_->SetFileName(lastCameraParam.paramName, lastCameraParam.paramVal, currentParams.exposureTime, fileCount++);
			if (fileCount == lastCameraParam.count) {
				fileCount = 0;
				paramsQueue.pop();
			}
		}

		pImageProcessing_->SetImgType(img_format);
		pImageProcessing_->SetImageTotalLength(data_total_length);
		system("cls");
	}

	// 2. Just collect image data
	else {
		pImageProcessing_->AssembleImageData(hReceivedPacket.data, received_packet_data_length);
	}

	// 3. If collection is finished, transform image data
	if (pImageProcessing_->isAssembleCompleted()) {
		pImageProcessing_->ImageModification();
		pImageProcessing_->ChooseImageProcessOption();
		pImageProcessing_->InitImageBuffer();
		image_piece_number_ = received_total_length_ = 0;
		system("cls");
		if (paramsQueue.empty() == 0) {
			Sleep(1000);
			captureContinuously();
		}

		return;
	}

	else {
		received_total_length_ += received_packet_data_length;
		cout << std::fixed;
		cout.precision(2);
		cout << ((double)(packet_number * kPacketDataSize) / (double)data_total_length)  * 100. << "%" << endl;
		DrawPercentageArrow(received_total_length_, data_total_length);
		SendCommand(CAMERA_SEND_NEXT_PACKET);
	}
}



/* ===================================================================================================
	Description
	- Among the option of camera setting, choose one
=================================================================================================== */
void DeviceController::ChangeCameraOption(void) {
	ShowFineSettingList();
	BYTE c = GetOneChar();
	switch (c) {
	case '1':
		SetCDSGain();
		break;

	case '2':
		SetVGAGain();
		break;

	case '3':
		SetBlackLevel();
		break;

	case '4':
		SetExposureTime();
		break;

	case '5':
		SetLEDPeriod();
		break;

	case '6':
		SetNumberOfPicture();
		break;

	case '7':
		SetADCMinimum();
		break;

	case 'x':
		cout << "Exit choosing gain setting option" << endl;
		return;

	default:
		break;
	}
}

/* ===================================================================================================
	Description
	- Set CDS Gain value which will be affecting to analog gain(0dB ~ 18dB)
	- Default is 6dB
=================================================================================================== */
void DeviceController::SetCDSGain(void) {
	ShowCDSOption();
	BYTE c = GetOneChar();
	if (c < 0x31 || c > 0x39) {
		cout << "Input correct menu" << endl;
		return;
	}
	c -= 0x31;
	SendShortValue(CAMERA_SET_CDS_GAIN, c);
}


/* ===================================================================================================
	Description
	- Set VGA Gain value which will be affecting to analog gain(0 ~ 1023)
=================================================================================================== */
void DeviceController::SetVGAGain(void) {
	cout << "VGA setting will increase analog signal from CCD sensor in detail(default option : 255)" << endl;
	SendShortValue(CAMERA_SET_VGA_GAIN, getValueLowerThanMaximum(1024));
}

/* ===================================================================================================
	Description
	- Set Black level value which will affect digital level of black level(0 ~ 4095)
	- The black level is the value of black signal between each frame
	=================================================================================================== */
void DeviceController::SetBlackLevel(void) {
	cout << "Set digital level as a black signal(default option : 492)" << endl;
	SendShortValue(CAMERA_SET_BLACK_LEVEL, getValueLowerThanMaximum(4095));
}

/* ===================================================================================================
	Description
	- Set the exposure time of camera
	- The maximum time is 3000 * (1/30s) = 100s
=================================================================================================== */
void DeviceController::SetExposureTime(void) {
	cout << "Set Exposure Time (default is 1 : 1/30s)" << endl;
	SendShortValue(CAMERA_SET_EXPOSURE_TIME, getValueLowerThanMaximum(3000));
}

/* ===================================================================================================
	Description
	- It will set the period of one LED blink when we capture the image
=================================================================================================== */
void DeviceController::SetLEDPeriod(void) {
	cout << "Set LED Period time" << endl;
	SendShortValue(CAMERA_SET_LED_PERIOD, getValueLowerThanMaximum(4095));
}

/* ===================================================================================================
	Description
	- Set the minimum level of ADC which is the standard for digitizing the CCD input
=================================================================================================== */
void DeviceController::SetADCMinimum(void) {
	cout << "Set the minimum level of ADC" << endl;
	cout << "Candidates : 0x5020(20512), 0x6030(24624)" << endl;
	cout << "Condition (High bytes = Low bytes + 0x30)" << endl;
	int setting_value = getValueLowerThanMaximum(28480);
	if ((setting_value >> 8) != ((setting_value & 0xff) + 0x30)) {
		cout << "High byte needs to be Low byte + 0x30" << endl;
		return;
	}
	SendShortValue(CAMERA_SET_ADC_MINIMUM, setting_value);
}


/* ===================================================================================================
	Description
	- It will set the number of pictures to take
	- It will be necessary to calculate the mean and deviation of each pixel or image
=================================================================================================== */
void DeviceController::SetNumberOfPicture(void) {
	cout << "Set Number of pictures to capture" << endl;
	int setting_value = getValueLowerThanMaximum(4095);
	SendShortValue(CAMERA_SET_NUMBER_OF_PICTURES, setting_value);
}



/*===================================================================================================================

Helper functions

===================================================================================================================*/
void DeviceController::ShowTestOptions() {
	cout << "Which command do you want to send" << endl;
	cout << "1) Echo test : send sample packet and get same one from device " << endl;
	cout << "2) Camera test : do camera test " << endl;
	cout << "x) Back to previous menu" << endl;
}

void DeviceController::ShowEchoTestOptions() {
	SplitLine();
	cout << "It will send one sample packet to device" << endl;
	cout << "Device will send same packet to PC" << endl;
	cout << "The purpose of this function is checking packet protocol work well" << endl;
	cout << "Enter one charactor" << endl;
}

void DeviceController::ShowCameraOperationOption() {
	SplitLine();
	cout << "Which camera option do you want to do" << endl;
	cout << "1) Control camera" << endl;
	cout << "2) Image Transmission" << endl;
	cout << "3) Image process" << endl;
	cout << "4) Control LED" << endl;
}

void DeviceController::ShowExposureOption(void) {
	SplitLine();
	cout << "It will set the exposure time to capture" << endl;
	for (int i = 1; i < 10; i++)
		cout << i << ") " << i << "sec" << endl;
	cout << "x) Back to previous menu" << endl;
}

void DeviceController::ShowFineSettingList() {
	SplitLine();
	cout << "Which camera option do you want to do" << endl;
	cout << "1) Set CDS Gain" << endl;
	cout << "2) Set VGA Gain" << endl;
	cout << "3) Set Black Level" << endl;
	cout << "4) Set Exposure Time" << endl;
	cout << "5) Set LED Period" << endl;
	cout << "6) Set number of pictures to capture" << endl;
	cout << "7) Set minimum level of ADC" << endl;
	cout << "x) Back to previous menu" << endl;

}

void DeviceController::ShowCDSOption(void) {
	SplitLine();
	cout << "CDS will increase analog signal from CCD sensor(default option : 3)" << endl;
	cout << "1) 0dB" << endl;
	cout << "2) 3dB" << endl;
	cout << "3) 6dB" << endl;
	cout << "4) 9dB" << endl;
	cout << "5) 12dB" << endl;
	cout << "6) 15dB" << endl;
	cout << "7) 18dB" << endl;
}

void DeviceController::ShowControlCameraOption(void) {
	SplitLine();
	cout << "1) Camera Power On" << endl;
	cout << "2) Camera Power Off" << endl;
	cout << "3) Camera capture image" << endl;
	cout << "4) Camera set camera option" << endl;
	cout << "5) Camera capture image at current setting" << endl;
	cout << "6) Send camera parameter which is set in cameraParam file" << endl;
	cout << "x) Go to previous menu" << endl;
}

void DeviceController::ShowImageTransmissionOption(void) {
	SplitLine();
	cout << "1) Send raw image format" << endl;
	cout << "2) Send packed image format" << endl;
	cout << "3) Send packed png format" << endl;
	cout << "4) Send cut off image" << endl;
	cout << "5) Send packed cut off png image" << endl;
	cout << "6) Send last activated mode" << endl;
	cout << "7) Send Image pixel mean" << endl;
	cout << "8) Send Image pixel dev" << endl;
	cout << "9) Send pixel save information" << endl;
	cout << "x) Go to previous menu" << endl;
}

void DeviceController::ShowImageProcessOption(void) {
	SplitLine();
	cout << "1) Pack raw image data" << endl;
	cout << "2) Convert packed image to png " << endl;
	cout << "3) Cut image edges" << endl;
	cout << "4) Convert packed cut off image edges to png" << endl;
	cout << "5) Save image data to emmc" << endl;
	cout << "6) Read image data from emmc" << endl;
	cout << "7) Show stored image list" << endl;
	cout << "8) Delete last node" << endl;
	cout << "9) Convert pixel mean to png" << endl;
	cout << "a) Convert pixel dev to png" << endl;
	cout << "x) Go to previous menu" << endl;
}

void DeviceController::ShowLEDOption(void) {
	SplitLine();
	cout << "1) Turn on the LED" << endl;
	cout << "2) Turn off the LED" << endl;
	cout << "3) Blank the LED" << endl;
	cout << "x) Go to previous menu" << endl;

}



/*
	Send short values with certain command (2bytes)
*/
void DeviceController::SendShortValue(BYTE command, int value) {
	vector<BYTE> value_vec(2);
	value_vec[0] = value & 0xff;
	value_vec[1] = (value >> 8) & 0xff;
	SetSendingPacketInfo(0x00, command, value_vec);
	SendPacket();
}


/* ===================================================================================================
	Description
	- It will choose correct imgFormat depending on the dataTotalLength sent from device
	Return
	- (CollectedImageFormat) imgFormat
=================================================================================================== */
CollectedImageFormat DeviceController::GetCorrectImgFormat(unsigned int dataTotalLength, BYTE command) {
	CollectedImageFormat imgFormat;
	if (dataTotalLength == RAW_IMG_TOTAL_LENGTH)
		imgFormat = RAW_IMAGE_FORMAT;
	else if (dataTotalLength == PACKED_RAW_IMG_TOTAL_LENGTH)
		imgFormat = PACKED_RAW_IMAGE_FORMAT;
	else if (dataTotalLength == CUT_OFF_IMG_TOTAL_LENGTH)
		imgFormat = CUT_OFF_IMAGE_FORMAT;
	else if (dataTotalLength == PIXEL_INFO_TOTAL_LENGTH)
		imgFormat = PIXEL_INFO_FORMAT;
	else
		imgFormat = PACKED_PNG_IMAGE_FORMAT;
	
	return imgFormat;
}


void DeviceController::initCameraParams() {
	currentParams.cds = 2;
	currentParams.vga = 15;
	currentParams.blackLevel = 0;
	currentParams.adcMinimum = 0x5020;
	currentParams.ledTime = 0;
	currentParams.exposureTime = 1;
}


vector<BYTE> DeviceController::copyParamsToVector(CameraParams params) {
	vector<BYTE> retVec(sizeof(params));
	retVec[0] = params.cds & 0xff;
	retVec[1] = (params.cds >> 8) & 0xff;
	retVec[2] = params.vga & 0xff;
	retVec[3] = (params.vga >> 8) & 0xff;
	retVec[4] = params.blackLevel & 0xff;
	retVec[5] = (params.blackLevel >> 8) & 0xff;
	retVec[6] = params.exposureTime & 0xff;
	retVec[7] = (params.exposureTime >> 8) & 0xff;
	retVec[8] = params.ledTime & 0xff;
	retVec[9] = (params.ledTime >> 8) & 0xff;
	retVec[10] = params.adcMinimum & 0xff;
	retVec[11] = (params.adcMinimum >> 8) & 0xff;
	return retVec;
}

void DeviceController::saveLastCameraParam(int mode, int count) {
	if (mode < 0 || mode > 5) {
		cout << mode << " is not valid option" << endl;
		return;
	}
	if (mode == 0) lastCameraParam.paramName = "cds";
	else if (mode == 1) lastCameraParam.paramName = "vga";
	else if (mode == 2) lastCameraParam.paramName = "blackLevel";
	else if (mode == 3) lastCameraParam.paramName = "exposureTime";
	else if (mode == 4) lastCameraParam.paramName = "ledTime";
	else if (mode == 5) lastCameraParam.paramName = "adcMinimum";
	lastCameraParam.paramVal = paramsQueue.front()[lastCameraParam.paramName];
	lastCameraParam.count = count;
	return;
}