#pragma once
#include <iostream>
#include <Windows.h>
#include <string>
#include <sstream>
#include <exception>
#include <stdexcept>
#include <typeinfo>

#include "../serial_communication.h"
#include "uart_params.h"



class Uart : SerialCommunication
{
public:


	Uart();
	~Uart();

	void OpenPort(std::string port_number);
	void ClosedPort(HANDLE opend_port);

	void Write(BYTE c);
	WORD Read(void);

	void ChoosePort(std::string port_number);
	void set_serial_params(UartParams user_params = default_serial_params);
	void set_timeouts(TimeoutsParams user_timeouts = default_timeouts);

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
