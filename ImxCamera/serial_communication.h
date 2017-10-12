#pragma once
#include <Windows.h>
class SerialCommunication
{
public:
	SerialCommunication();
	~SerialCommunication();
	virtual void Write(BYTE c) = 0;
	virtual WORD Read(void) = 0;
};

