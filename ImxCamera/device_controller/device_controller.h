#pragma once
#include "../comm_protocol/protocol.h"
#include "../image_processing/image_processing.h"

class DeviceController :public Protocol {
public:
	DeviceController(Uart * user_hUart) :Protocol(user_hUart), image_piece_number_(1)
	{
		pImageProcessing_ = new ImageProcessing();
	};
	~DeviceController();


	virtual void PickAndSendCommand(void);
	virtual void DoCommand(void);

	void ShowTestOptions(void);
	void ShowCameraTestOption(void);
	void ControlDevice(BYTE c);


	void EchoTest();

	void CameraTest();
	void CameraPowerOn(void);
	void CameraPowerOff(void);
	void ShowGainSetting(void);
	void SetCDSValue(void);
	void CaptureImage(void);
	void SendCapturedImage(void);
	void CollectImageData(void);

	void CheckEchoTest();

private:
	BYTE image_piece_number_;
	ImageProcessing * pImageProcessing_;
	/*Packet hSendingPacket;
	Packet hReceivedgPacket;*/
};
