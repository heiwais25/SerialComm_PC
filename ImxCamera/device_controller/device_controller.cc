#include "stdafx.h"
#include "device_controller.h"


DeviceController::~DeviceController(void) {

}

void DeviceController::PickAndSendCommand(void) {
	//Protocol::PickAndSendCommand();
	BYTE c;
	std::cout << "Which command do you want to send" << std::endl;
	std::cout << "1) Echo test : send sample packet and get same one from device " << std::endl;
	std::cout << "2) Camera test : do camera test " << std::endl;
	std::cout << "x) Back to previous menu" << std::endl;
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

void DeviceController::ControlDevice(BYTE c) {
	init_comm_state();
	switch (c) {
		case '1':
			EchoTest();
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

// Do specific action dending on command from device
// It would be signal about success or failure of communication
void DeviceController::DoCommand(void) {
	switch (hReceivedPacket.command) {
		case ECHO_TEST:
			CheckEchoTest();
			break;
		default:
			printf("%x\n", hReceivedPacket.command);
			// Nedd to add some function
			break;
	}
}