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
#include "uart/uart.h"
#include "device_controller/device_controller.h"
#include "utility/utility.h"


void ChooseCommMode(void);

int main()
{
	
	Uart * pUart = new Uart();
	std::string port_number = "COM4";
	pUart->OpenPort(port_number);

	//Protocol * hController = new Protocol(pUart);
	Protocol * hController = new DeviceController(pUart);


	// Choose whether start communicate or other option
	ChooseCommMode();
	hController->StandByMode();


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