#include "stdafx.h"
#include "device_controller.h"
DeviceController::~DeviceController(void) {

}

/*----------------------------------------------------------------------------------------------------------

Main stream

----------------------------------------------------------------------------------------------------------*/
void DeviceController::PickAndSendCommand(void) {
	BYTE c;
	ShowTestOptions();
	while (1) {
		if (c = GetOneCharKeyboardInput()) {
			if (c == 'x')break;
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
	init_comm_state();
	switch (c) {
		case '1':
			EchoTest();
			break;

		case '2':
			CameraTest();
			break;

		case '3':
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
	set_hSendingPacket_with_data(0x00, ECHO_TEST, 0x01, &c);
	SendPacket();
}

/*
	Do camera test including taking picture and some gain setting
*/
void DeviceController::CameraTest(void) {
	ShowCameraTestOption();
	ChooseCameraTestOption();
}

void DeviceController::ChooseCameraTestOption(void) {
	BYTE c;
	while (1) {
		c = GetOneCharKeyboardInput();
		if (isDecimalNumber(c))
			break;
	}
	switch (c) {
	case '1':
		SendCommand(CAMERA_POWER_ON);
		break;
	case '2':
		SendCommand(CAMERA_POWER_OFF);
		break;
	case '3':
		SendCommand(CAMERA_SHOW_GAIN_SETTING);
		break;

	case '4':
		SendCommand(CAMERA_CAPTURE);
		break;

	case '5':
		SendCommand(CAMERA_SEND_CAPTURED_IMAGE);
		break;

	case '6':
		SetCameraGainOption();
		break;
	}
}

void DeviceController::SetCameraGainOption(void) {
	ShowGainSettingOption();
	ChooseGainSettingOption();
}

void DeviceController::ChooseGainSettingOption(void) {
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
		
		case 'x':
			std::cout << "Exit choosing gain setting option" << std::endl;
			break;

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
			CollectImageData();
			break;

		default:
			printf("%x\n", hReceivedPacket.command);
			break;
	}
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
void DeviceController::CollectImageData() {

	if (image_piece_number_ != hReceivedPacket.number)
		std::cerr << "The order of data is packet is wr ong" << std::endl;
	else
		image_piece_number_++;

	// send this data to image processing class
	pImageProcessing_->AssembleImageData(hReceivedPacket.data, received_packet_data_length);

	if (pImageProcessing_->isAssembleCompleted())
		pImageProcessing_->ChooseImageProcessOption();
	else  
		SendCommand(CAMERA_SEND_NEXT_PACKET);
	
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
	set_hSendingPacket_with_data(0x00, CAMERA_SET_CDS_GAIN, 0x01, &c);
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
	set_hSendingPacket_with_data(0x00, CAMERA_SET_VGA_GAIN, 0x02, value);
	SendPacket();
}

/*
	Set VGA Gain value which will be affecting digital level of black level(0 ~ 1023)
*/
void DeviceController::SetBlackLevel(void) {
	std::cout << "Set digital level as a black signal(default option : 492)" << std::endl;
	int setting_value = getValueLowerThanMaximum(1024);
	unsigned char value[2];
	value[0] = setting_value & 0xff;
	value[1] = (setting_value >> 8) & 0xff;
	set_hSendingPacket_with_data(0x00, CAMERA_SET_BLACK_LEVEL, 0x02, value);
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

void DeviceController::ShowCameraTestOption() {
	SplitLine();
	std::cout << "Which camera option do you want to do" << std::endl;
	std::cout << "1) Camera power on" << std::endl;
	std::cout << "2) Camera power off" << std::endl;
	std::cout << "3) Show camera current gain setting" << std::endl;
	std::cout << "4) Camera capture image" << std::endl;
	std::cout << "5) Camera send captured image " << std::endl;
	std::cout << "6) Camera ADC setting " << std::endl;
	std::cout << "x) Back to previous menu" << std::endl;
}

void DeviceController::ShowGainSettingOption() {
	SplitLine();
	std::cout << "Which camera option do you want to do" << std::endl;
	std::cout << "1) Set CDS Gain" << std::endl;
	std::cout << "2) Set VGA Gain" << std::endl;
	std::cout << "3) Set Black Level" << std::endl;
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