#include "stdafx.h"
#include "uart.h"


Uart::Uart()
{
	isClosedPort = 1;
}


Uart::~Uart()
{
}

void Uart::OpenPort(std::string port_number) {
	try {
		ChoosePort(port_number);
		set_serial_params();
		set_timeouts();
	}
	catch (const ClosedPortError& e) {
		std::cerr << "Port is closed, please open the port ahead and try again" << std::endl;
		exit(1);
	}
	catch (...) {

		std::cerr << "Error in opening port" << std::endl;
		exit(1);
	}
}

void Uart::ClosedPort(HANDLE opend_port) {
	if (!CloseHandle(opend_port))
		throw ClosingPortError{};
	isClosedPort = 1;
}

void Uart::Write(BYTE c) {
	DWORD bytes_written;
	if (!WriteFile(hSerialPort, &c, 1, &bytes_written, NULL))
		throw UartWriteError{};
}

WORD Uart::Read(void) {
	unsigned long bytes_read;
	BYTE input_char;
	if (!ReadFile(hSerialPort, &input_char, 1, &bytes_read, NULL))
		throw UartReadError{};
	if (bytes_read == 0)
		return kNoData;
	return (WORD)input_char;
}

// It will raise Error when it has handleValueError
void Uart::ChoosePort(std::string port_number) {
	std::wstring wstr_port_number = L"";
	wstr_port_number.assign(port_number.begin(), port_number.end());
	wstr_port_number = L"\\\\.\\" + wstr_port_number;
	hSerialPort = CreateFile(wstr_port_number.c_str(),
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (hSerialPort == INVALID_HANDLE_VALUE)
		throw handleValueError{};

	// Open port
	isClosedPort = 0;
}

void Uart::set_serial_params(UartParams user_params) {
	if (isClosedPort) {
		throw ClosedPortError{};
	}
	serial_params = { 0 };
	if (!GetCommState(hSerialPort, &serial_params))
		throw GetCommStateError{};

	serial_params.DCBlength = sizeof(serial_params);
	serial_params.BaudRate = user_params.BaudRate;
	serial_params.ByteSize = user_params.ByteSize;
	serial_params.StopBits = user_params.StopBits;
	serial_params.Parity = user_params.Parity;

	if (!SetCommState(hSerialPort, &serial_params))
		throw SetCommStateError{};
}

void Uart::set_timeouts(TimeoutsParams user_timeouts) {
	if (isClosedPort) {
		throw ClosedPortError{};
	}
	timeouts = { 0 };
	timeouts.ReadIntervalTimeout = user_timeouts.ReadIntervalTimeout;
	timeouts.ReadTotalTimeoutConstant = user_timeouts.ReadTotalTimeoutConstant;
	timeouts.ReadTotalTimeoutMultiplier = user_timeouts.ReadTotalTimeoutMultiplier;
	timeouts.WriteTotalTimeoutConstant = user_timeouts.WriteTotalTimeoutConstant;
	timeouts.WriteTotalTimeoutMultiplier = user_timeouts.WriteTotalTimeoutMultiplier;
	if (!SetCommTimeouts(hSerialPort, &timeouts))
		throw SetCommTimeoutsError{};
}
