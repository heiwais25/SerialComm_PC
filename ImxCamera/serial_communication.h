#pragma once
#include <Windows.h>

// The basic function will be Write and read
class SerialCommunication {
	public:
		SerialCommunication();
		~SerialCommunication();
		virtual void Write(BYTE c) = 0;
		virtual WORD Read(void) = 0;
};

