#pragma once
#include "../comm_protocol/protocol.h"
#include "../image_processing/image_processing.h"


class DeviceController :public Protocol {
public:
	DeviceController(Uart * user_hUart) :Protocol(user_hUart), image_piece_number_(1) {
		pImageProcessing_ = new ImageProcessing();
		
	};
	~DeviceController();

	virtual void PickAndSendCommand(void);
	
	void ControlDevice(BYTE c);
	void EchoTest(void);

	void CameraTest();
	void ChooseCameraTestOption(void);
	void CameraCaptureWithExposure(void);

	void SetCameraGainOption(void);
	void ChooseGainSettingOption(void);
	void SetCDSGain(void);
	void SetVGAGain(void);
	void SetBlackLevel(void);
	
	virtual void DoCommand(void);

	void CheckEchoTest();
	void CollectImageData(void);

	void ShowTestOptions(void);
	void ShowEchoTestOptions(void);
	void ShowCameraTestOption(void);
	void ShowExposureOption(void);
	void ShowGainSettingOption(void);
	void ShowCDSOption(void);

private:

	BYTE image_piece_number_;
	WORD received_packet_data_length;
	ImageProcessing * pImageProcessing_;
	
};
