#pragma once
#include "../comm_protocol/protocol.h"


class DeviceController :public Protocol {
public:
	DeviceController(Uart * user_hUart) :Protocol(user_hUart){};
	~DeviceController();


	virtual void PickAndSendCommand(void);
	virtual void DoCommand(void);

	void ShowOptions(void);
	void ControlDevice(BYTE c);


	void EchoTest();
	void CheckEchoTest();

private:
	
	/*Packet hSendingPacket;
	Packet hReceivedgPacket;*/
};
