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
	BYTE c;
	while (1) {
		if (c = GetOneCharKeyboardInput())
			break;
	}
	switch (c)
	{
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
			SetCameraFineSetting();
			break;

		case 'x':
			break;

		default:
			break;
	}
}

void DeviceController::DoImageTransmission(void) {
	ShowImageTransmissionOption();
	BYTE c;
	while (1) {
		if (c = GetOneCharKeyboardInput())
			break;
	}
	switch (c)
	{
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
	BYTE c;
	while (1) {
		if (c = GetOneCharKeyboardInput())
			break;
	}
	switch (c)
	{
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
	std::cout << "Blank the LED with fixed period" << std::endl;
	int setting_value = getValueLowerThanMaximum(4095);
	unsigned char value[2];
	value[0] = setting_value & 0xff;
	value[1] = (setting_value >> 8) & 0xff;
	SetSendingPacketInfo(0x00, BLANK_LED, 0x02, value);
	SendPacket();
}



void DeviceController::ChooseCameraOperation(void) {
	/*
		It will choose operation mode among 3 main operaion modes
		1. Control camera
		2. Image transmission
		3. Image process
	*/
	BYTE c;
	while (1) {
		if (c = GetOneCharKeyboardInput())
			break;
	}
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




void DeviceController::SetImageToRead() {
	std::cout << "Select the position of image stored at EMMC" << std::endl;
	int setting_value = getValueLowerThanMaximum(1024);
	unsigned char value[2];
	value[0] = setting_value & 0xff;
	value[1] = (setting_value >> 8) & 0xff;
	SetSendingPacketInfo(0x00, READ_IMAGE, 0x02, value);
	SendPacket();
}
//
//void DeviceController::CameraCaptureWithExposure(void) {
//	ShowExposureOption();
//	BYTE c;
//	while (1) {
//		c = GetOneCharKeyboardInput();
//		if (c == 'x') {
//			std::cout << "Exit" << std::endl;
//			return;
//		}
//		if (isDecimalNumber(c)) {
//			c = toDecimalNumber(c);
//			break;
//		}
//	}
//	SetSendingPacketInfo(0x00, CAMERA_CAPTURE_LONG_EXPOSURE, 0x01, &c);
//	SendPacket();
//}

void DeviceController::SetCameraFineSetting(void) {
	ShowCameraFineSettingOption();
	ChooseCameraFineSetting();
}

void DeviceController::ChooseCameraFineSetting(void) {
	BYTE c;
	while (1) {
		if (c = GetOneCharKeyboardInput())
			break;
	}
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

		case 'x':
			std::cout << "Exit choosing gain setting option" << std::endl;
			return;

		default:
			break;
	}
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

		case CAMERA_SEND_CAPTURED_IMAGE: // Because the image packet is seperated, we need to collect this image pieces
		case CAMERA_SEND_PACKED_DATA:
		case CAMERA_SEND_PNG:
		case CAMERA_SEND_CUT_OFF_IMAGE:
			CollectImageData();
			break;

		default:
			printf("%x\n", hReceivedPacket.command);
			break;
	}
}


void DeviceController::SetLEDPeriod(void) {
	// Get LED period value
	std::cout << "Set one LED pulse period" << std::endl;
	int setting_value = getValueLowerThanMaximum(4095);

	// Send command with value
	unsigned char value[2];
	value[0] = setting_value & 0xff;
	value[1] = (setting_value >> 8) & 0xff;
	SetSendingPacketInfo(0x00, CAMERA_SET_LED_PERIOD, 0x02, value);
	SendPacket();
}


void DeviceController::CheckEchoTest() {
	std::cout << "Send" << "\t" << "Received" << std::endl;
	for (int i = 0; i < received_packet_data_length; i++) {
		std::cout << std::hex << hSendingPacket.data[i] << "\t" << std::hex << hReceivedPacket.data[i] << std::endl;
		if (hReceivedPacket.data[i] - hSendingPacket.data[i]) {
			std::cerr << "Echo test error" << std::endl;
		}
	}
	std::cout << "Echo test Finish" << std::endl;
}

/*
	Due to captured image is trasmitted sepearate packet, we need to collect image data and make it original one
	To do this, we will use ImageProcessing class
	steps	1) Check packet order(number)
			2) AssembleImageData
			3) If the transmission is finished, processing image else request next frame
	This function will be continued until the image transmission is finished
*/
void DeviceController::CollectImageData(void) {
	// 1. Get packet number
	unsigned short packet_number = (hReceivedPacket.number[1] << 8) + hReceivedPacket.number[0];
	static unsigned int data_total_length = 0;
	static unsigned int received_total_length_ = 0;
	if (image_piece_number_ != packet_number) {

		std::cerr << "The order of data is packet is wrong" << std::endl;
		// ERROR HANDLING WHEN THE DATA IS CRASHED
		return;
	}
	else
		image_piece_number_++;

	// First packet will include image information
	if (packet_number == 0) {
		BYTE * data = hReceivedPacket.data;

		
		data_total_length = (data[3] << 24) + (data[2] << 16) + (data[1] << 8) + data[0];

		CollectedImageFormat img_format;
		int raw_total_length = kCsiHorizontalResolution * kCsiVerticalResolution;
		int cut_off_total_lentgh = kEffectiveImageWidth * kEffectiveImageHeight * 2;
		if (data_total_length == raw_total_length)
			img_format = RAW_IMAGE_FORMAT;
		else if (data_total_length == raw_total_length * 3 / 4)
			img_format = PACKED_RAW_IMAGE_FORMAT;
		else if (data_total_length == cut_off_total_lentgh) {
			img_format = CUT_OFF_IMAGE_FORMAT;
		}
		else
			img_format = PACKED_PNG_IMAGE_FORMAT;

		pImageProcessing_->SetImageType(img_format);
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
		return;
	}
	else {
		received_total_length_ += received_packet_data_length;
		std::cout << std::fixed;
		std::cout.precision(2);
		std::cout << ((double)(packet_number * kPacketDataSize) / (double)data_total_length)  * 100. << "%" << std::endl;
		DrawPercentageArrow(received_total_length_, data_total_length);
		SendCommand(CAMERA_SEND_NEXT_PACKET);
	}
}


/*===============================================================================================================================================================================================================================================================

Name:				SetCameraParamsValue

Description:		It will set camera params including cds, vga, black level and exposure time

Parameters:			void

Returns:			void

Date:				2018-01-02

===============================================================================================================================================================================================================================================================*/
void DeviceController::SetCameraParamsValue(void) {

}



/*
	Set CDS Gain value which will be affecting to analog gain(0dB ~ 18dB)
*/
void DeviceController::SetCDSGain(void) {
	BYTE c;
	ShowCDSOption();
	while (1) {
		c = GetOneCharKeyboardInput();
		if (isDecimalNumber(c)) {
			c = toDecimalNumber(c);
			if (c <= 7)
				break;
			else {
				std::cout << "Choose in the option " << std::endl;
				ShowCDSOption();
				continue;
			}
		}
	}
	c -= 1;
	SetSendingPacketInfo(0x00, CAMERA_SET_CDS_GAIN, 0x01, &c);
	SendPacket();
}

/*
	Set VGA Gain value which will be affecting to analog gain(0 ~ 1023)
*/
void DeviceController::SetVGAGain(void) {
	std::cout << "VGA setting will increase analog signal from CCD sensor in detail(default option : 255)" << std::endl;
	int setting_value = getValueLowerThanMaximum(1024);
	unsigned char value[2];
	value[0] = setting_value & 0xff;
	value[1] = (setting_value >> 8) & 0xff;
	SetSendingPacketInfo(0x00, CAMERA_SET_VGA_GAIN, 0x02, value);
	SendPacket();
}

/*
	Set VGA Gain value which will be affecting digital level of black level(0 ~ 1023)
*/
void DeviceController::SetBlackLevel(void) {
	std::cout << "Set digital level as a black signal(default option : 492)" << std::endl;
	int setting_value = getValueLowerThanMaximum(4095);
	unsigned char value[2];
	value[0] = setting_value & 0xff;
	value[1] = (setting_value >> 8) & 0xff;
	SetSendingPacketInfo(0x00, CAMERA_SET_BLACK_LEVEL, 0x02, value);
	SendPacket();
}

void DeviceController::SetExposureTime(void) {
	std::cout << "Set Exposure Time (default is 1 : 1/30s)" << std::endl;
	int setting_value = getValueLowerThanMaximum(3000);
	unsigned char value[2];
	value[0] = setting_value & 0xff;
	value[1] = (setting_value >> 8) & 0xff;
	SetSendingPacketInfo(0x00, CAMERA_SET_EXPOSURE_TIME, 0x02, value);
	SendPacket();
}


/*===================================================================================================================

Helper functions

===================================================================================================================*/
void DeviceController::ShowTestOptions() {
	std::cout << "Which command do you want to send" << std::endl;
	std::cout << "1) Echo test : send sample packet and get same one from device " << std::endl;
	std::cout << "2) Camera test : do camera test " << std::endl;
	std::cout << "x) Back to previous menu" << std::endl;
}

void DeviceController::ShowEchoTestOptions() {
	SplitLine();
	std::cout << "It will send one sample packet to device" << std::endl;
	std::cout << "Device will send same packet to PC" << std::endl;
	std::cout << "The purpose of this function is checking packet protocol work well" << std::endl;
	std::cout << "Enter one charactor" << std::endl;
}

void DeviceController::ShowCameraOperationOption() {
	SplitLine();
	std::cout << "Which camera option do you want to do" << std::endl;
	std::cout << "1) Control camera" << std::endl;
	std::cout << "2) Image Transmission" << std::endl;
	std::cout << "3) Image process" << std::endl;
	std::cout << "4) Control LED" << std::endl;
}

void DeviceController::ShowExposureOption(void) {
	SplitLine();
	std::cout << "It will set the exposure time to capture" << std::endl;
	for (int i = 1; i < 10; i++)
		std::cout << i << ") " << i << "sec" << std::endl;
	std::cout << "x) Back to previous menu" << std::endl;
}

void DeviceController::ShowCameraFineSettingOption() {
	SplitLine();
	std::cout << "Which camera option do you want to do" << std::endl;
	std::cout << "1) Set CDS Gain" << std::endl;
	std::cout << "2) Set VGA Gain" << std::endl;
	std::cout << "3) Set Black Level" << std::endl;
	std::cout << "4) Set Exposure Time" << std::endl;
	std::cout << "5) Set LED period" << std::endl;
	std::cout << "x) Back to previous menu" << std::endl;

}

void DeviceController::ShowCDSOption(void) {
	SplitLine();
	std::cout << "CDS will increase analog signal from CCD sensor(default option : 2)" << std::endl;
	std::cout << "1) 0dB" << std::endl;
	std::cout << "2) 3dB" << std::endl;
	std::cout << "3) 6dB" << std::endl;
	std::cout << "4) 9dB" << std::endl;
	std::cout << "5) 12dB" << std::endl;
	std::cout << "6) 15dB" << std::endl;
	std::cout << "7) 18dB" << std::endl;
}

void DeviceController::ShowControlCameraOption(void) {
	SplitLine();
	std::cout << "1) Camera Power On" << std::endl;
	std::cout << "2) Camera Power Off" << std::endl;
	std::cout << "3) Camera capture image" << std::endl;
	std::cout << "4) Camera set camera option" << std::endl;
	std::cout << "x) Go to previous menu" << std::endl;
}

void DeviceController::ShowImageTransmissionOption(void) {
	SplitLine();
	std::cout << "1) Send raw image format" << std::endl;
	std::cout << "2) Send packed image format" << std::endl;
	std::cout << "3) Send packed png format" << std::endl;
	std::cout << "4) Send cut off image" << std::endl;
	std::cout << "5) Send packed cut off png image" << std::endl;
	std::cout << "6) Send last activated mode" << std::endl;
	std::cout << "x) Go to previous menu" << std::endl;
}

void DeviceController::ShowImageProcessOption(void) {
	SplitLine();
	std::cout << "1) Pack raw image data" << std::endl;
	std::cout << "2) Convert packed image to png " << std::endl;
	std::cout << "3) Cut image edges" << std::endl;
	std::cout << "4) Convert packed cut off image edges to png" << std::endl;
	std::cout << "5) Save image data to emmc" << std::endl;
	std::cout << "6) Read image data from emmc" << std::endl;
	std::cout << "7) Show stored image list" << std::endl;
	std::cout << "8) Delete last node" << std::endl;
	std::cout << "x) Go to previous menu" << std::endl;
}

void DeviceController::ShowLEDOption(void) {
	SplitLine();
	std::cout << "1) Turn on the LED" << std::endl;
	std::cout << "2) Turn off the LED" << std::endl;
	std::cout << "3) Blank the LED" << std::endl;
	std::cout << "x) Go to previous menu" << std::endl;

}