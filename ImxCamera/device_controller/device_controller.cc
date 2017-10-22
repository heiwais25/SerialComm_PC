#include "stdafx.h"
#include "device_controller.h"
DeviceController::~DeviceController(void) {

}

void DeviceController::PickAndSendCommand(void) {
	BYTE c;
	ShowTestOptions();
	while (1) {
		if (c = GetOneCharKeyboardInput()) {
			if (c == 'x')break;
			ControlDevice(c);
			if (isWaitResponse()) {
				break; // It sends command, so let's wait for its response
			}
		}
	}
}

/*===================================================================================================================

Function that user want to use

===================================================================================================================*/
void DeviceController::ShowTestOptions() {
	std::cout << "Which command do you want to send" << std::endl;
	std::cout << "1) Echo test : send sample packet and get same one from device " << std::endl;
	std::cout << "2) Camera test : do camera test " << std::endl;
	std::cout << "x) Back to previous menu" << std::endl;
}

void DeviceController::ShowCameraTestOption() {
	SplitLine();
	std::cout << "Which camera option do you want to do" << std::endl;
	std::cout << "1) Camera power on" << std::endl;
	std::cout << "2) Camera power off" << std::endl;
	std::cout << "3) Show camera current gain setting" << std::endl;
	std::cout << "4) Camera capture image" << std::endl;
	std::cout << "5) Camera send captured image " << std::endl;
	std::cout << "x) Back to previous menu" << std::endl;
}

void DeviceController::ControlDevice(BYTE c) {
	init_comm_state();
	switch (c) {
		case '1':
			EchoTest();
			break;
		case '2':
			CameraTest();
			break;
	}
}

void DeviceController::EchoTest() {
	BYTE c;

	SplitLine();
	std::cout << "It will send one sample packet to device" << std::endl;
	std::cout << "Device will send same packet to PC" << std::endl;
	std::cout << "The purpose of this function is checking packet protocol work well" << std::endl;
	std::cout << "Enter one charactor" << std::endl;
	
	while (1) {
		if (c = GetOneCharKeyboardInput()) {
			break;
		}
	}
	set_hSendingPacket_with_data(0x00, ECHO_TEST, 0x01, &c);
	SendPacket();
}

void DeviceController::CameraTest(void) {
	ShowCameraTestOption();
	BYTE c;
	while (1) {
		c = GetOneCharKeyboardInput();
		if (isDecimalNumber(c)) {
			break;
		}
	}
	switch (c) {
		case '1':
			CameraPowerOn();
			break;
		case '2':
			CameraPowerOff();
			break;
		case '3':
			ShowGainSetting();
			break;

		case '4':
			CaptureImage();
			break;

		case '5':
			SendCapturedImage();
			break;
	}
}

void DeviceController::CameraPowerOn(void) {
	std::cout << "Camera power on" << std::endl;
	SendCommand(CAMERA_POWER_ON);
}

void DeviceController::CameraPowerOff(void) {
	std::cout << "Camera power off" << std::endl;
	SendCommand(CAMERA_POWER_OFF);
}

void DeviceController::ShowGainSetting(void) {
	std::cout << "Camera power off" << std::endl;
	SendCommand(CAMERA_SHOW_GAIN_SETTING);
}

void DeviceController::CaptureImage(void) {
	std::cout << "Camera take image" << std::endl;
	SendCommand(CAMERA_CAPTURE);
}

void DeviceController::SendCapturedImage(void) {
	std::cout << "Camera send captured image" << std::endl;
	SendCommand(CAMERA_SEND_CAPTURED_IMAGE);
}


void DeviceController::SetCDSValue(void) {

}

/*===================================================================================================================

Response after getting command from the device

===================================================================================================================*/

// Do specific action dending on command from device
// It would be signal about success or failure of communication
void DeviceController::DoCommand(void) {
	switch (hReceivedPacket.command) {
		case GOOD_COMMUNICATION: // In the case of device get packet successfully
			break;

		case ECHO_TEST:
			CheckEchoTest();
			break;

		case CAMERA_SEND_CAPTURED_IMAGE:
			CollectImageData();
			break;

		default:
			printf("%x\n", hReceivedPacket.command);
			// Nedd to add some function
			break;
	}
}

void DeviceController::CheckEchoTest() {
	WORD received_packet_length = (hReceivedPacket.length[1] << 8) + hReceivedPacket.length[0];
	std::cout << "Send" << "\t" << "Received" << std::endl;
	for (int i = 0; i < received_packet_length; i++) {
		std::cout << std::hex << hSendingPacket.data[i] << "\t" << std::hex << hReceivedPacket.data[i] << std::endl;
		if (hReceivedPacket.data[i] - hSendingPacket.data[i]) {
			std::cerr << "Echo test error" << std::endl;
		}
	}
	std::cout << "Echo test Finish" << std::endl;
}

// 1. 약 4kbyte씩 image data를 수신
// 2. image data를 수신하면 이를 ImageProcessing의 image buffer에 저장
// 3. image buffer가 해당 image size로 꽉 차면 저장하기
// 4. 아직 덜 왔으면 다음 data를 요청하기
void DeviceController::CollectImageData() {
	std::cout << "collecting..." << std::endl;
	// Check the packet number is continuous
	if (image_piece_number_ != hReceivedPacket.number)
		std::cerr << "The order of data is packet is wrong" << std::endl;
	else
		image_piece_number_++;

	BYTE received_packet_image_data[kPacketDataSize];
	WORD received_packet_data_length = (hReceivedPacket.length[1] << 8) + hReceivedPacket.length[0];


	// copy image data from packet to receiver
	memcpy(received_packet_image_data, hReceivedPacket.data, received_packet_data_length);

	// send this data to image processing class
	pImageProcessing_->AssembleImageData(received_packet_image_data, received_packet_data_length);

	// check the image is whether the end or not
	if (pImageProcessing_->isAssembleCompleted()) { // if end save it
		pImageProcessing_->SaveImageToBmp();
	}
	else { // else request next image data 
		SendCommand(CAMERA_SEND_NEXT_PACKET);
	}
}