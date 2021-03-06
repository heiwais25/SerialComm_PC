// ImxCamera.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

/* Todo
	image_processing working on...

	Comments
1. normal
	unit test
	class pointer를 넘겨준 것에 대해 const를 설정할 수 잇는가? 그냥 넘겨준 것과 포인터로 다룰 때의 차이점은?

*/

#include "stdafx.h"
#include "uart.h"
#include "device_controller.h"
#include "utility.h"
#include "image_processing.h"


void ChooseCommMode(void);
void UnitTest_ImageProcessing(void);


int main() {
	int mode = 2; // 1 : Image Processing Test // 2 : Communication with board
	if (mode == 1) {
		while (1)
			UnitTest_ImageProcessing(); 
	}
	else {
		std::string port_number = "COM4";
		PacketProtocol * hController = new DeviceController(port_number);

		//// Choose whether start communicate or other option
		ChooseCommMode();
		hController->StandByMode();
	}
	
	return 0;
}

void ChooseCommMode(void) {
	BYTE c;
	std::cout << "1) Start communication" << std::endl;
	std::cout << "x) Exit" << std::endl;

	while (1) {
		c = (BYTE)GetOneCharKeyboardInput();
		if (c == '1') {
			std::cout << "Start!" << std::endl;
			SplitLine();
			break;
		}
		else if (c == (BYTE)'x' || c == (BYTE)'X') {
			exit(1);
		}
		else {

		}

	}
}

/*
	When we want to test or set image processing class, use this class to check several functions
	What you need to do is put raw data from camera to the parameter of ReadImageData
*/
void UnitTest_ImageProcessing(void) {
	//std::string raw_file_name = "output/1414_3756";
	//std::string raw_file_name = "output/2008_5124";
	//std::string raw_file_name = "output/1420_0736";
	std::string raw_file_name = "output/1814_4627";
	ImageProcessing * pImgProcessing = new ImageProcessing();
	pImgProcessing->ReadImageData(raw_file_name);
	std::cout << std::endl;
	std::cout << "Offset : " << pImgProcessing->GetBlackLineStartPoint() << std::endl;
	pImgProcessing->ChooseImageProcessOption();
}