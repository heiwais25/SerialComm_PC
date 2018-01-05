#pragma once
#ifndef __UART_PARAM__
#define __UART_PARAM__
#include <iostream>
#include <Windows.h>

const WORD kNoData = 0xFFFF;
struct UartParams {
	UartParams() :BaudRate(115200), ByteSize(8), StopBits(ONESTOPBIT), Parity(NOPARITY) {};

	UartParams operator=(const UartParams& right) {
		BaudRate = this->BaudRate + right.BaudRate;
		ByteSize = this->ByteSize + right.ByteSize;
		StopBits = this->StopBits + right.StopBits;
		Parity = this->Parity + right.Parity;
	}
	DWORD BaudRate;
	BYTE ByteSize;
	BYTE StopBits;
	BYTE Parity;
};

struct TimeoutsParams {
	TimeoutsParams() :
		ReadIntervalTimeout(50), ReadTotalTimeoutConstant(50), ReadTotalTimeoutMultiplier(1),
		WriteTotalTimeoutConstant(50), WriteTotalTimeoutMultiplier(1) {};
	DWORD ReadIntervalTimeout;
	DWORD ReadTotalTimeoutConstant;
	DWORD ReadTotalTimeoutMultiplier;
	DWORD WriteTotalTimeoutConstant;
	DWORD WriteTotalTimeoutMultiplier;
};

const UartParams default_serial_params;
const TimeoutsParams default_timeouts;
#endif