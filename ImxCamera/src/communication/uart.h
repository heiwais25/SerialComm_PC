#pragma once
#include <iostream>
#include <Windows.h>
#include <string>
#include <sstream>
#include <exception>
#include <stdexcept>
#include <typeinfo>
#include "uart_params.h"
#include "serial_communication.h"

class Uart : SerialCommunication {
	public:
		Uart() : isClosedPort(1) {};
		~Uart() {};
		void OpenPort(std::string port_number);
		void ClosePort(HANDLE opend_port);
		void Write(BYTE c);
		WORD Read(void);

		void ChoosePort(std::string port_number);
		void SetSerialParams(UartParams user_params = default_serial_params);
		void SetTimeOuts(TimeoutsParams user_timeouts = default_timeouts);

	private:
		HANDLE hSerialPort;
		DCB serial_params;
		COMMTIMEOUTS timeouts;
		int isClosedPort;
};


struct handleValueError {};
struct CommStateError {};
struct GetCommStateError : CommStateError {};
struct SetCommStateError : CommStateError {};
struct SetCommTimeoutsError {};

struct UartWriteError {};
struct UartReadError {};
struct ClosingPortError {};
struct ClosedPortError {};
