#pragma once
#include "packet_protocol.h"
#include "image_processing.h"
#include "utility.h"

const enum CAMERA_PARAMS {
	CDS_GAIN = 0,
	VGA_GAIN,
	BLACK_LEVEL,
	EXPOSRUE_TIME,
};


class DeviceController : public PacketProtocol {
public:
	DeviceController(Uart * user_hUart) : PacketProtocol(user_hUart), image_piece_number_(0) {
		pImageProcessing_ = new ImageProcessing();
	};

	DeviceController(std::string port_number) : PacketProtocol(port_number), image_piece_number_(0) {
		pImageProcessing_ = new ImageProcessing();
	};
	~DeviceController() {};

	virtual void PickAndSendCommand(void);
	
	void ControlDevice(BYTE c);
	void EchoTest(void);

	void CameraTest();
	void ChooseCameraTestOption(void);
	void CameraCaptureWithExposure(void);

	void SetCameraParamsValue(void);


	void SetCameraGainOption(void);
	void ChooseGainSettingOption(void);
	void SetCDSGain(void);
	void SetVGAGain(void);
	void SetBlackLevel(void);
	void SetExposureTime(void);

	virtual void DoCommand(void);

	void CheckEchoTest();
	void CollectImageData(void);
	void CollectPackedImageData(void);
	void CollectPNGImageData(void);


	void ShowTestOptions(void);
	void ShowEchoTestOptions(void);
	void ShowCameraTestOption(void);
	void ShowExposureOption(void);
	void ShowGainSettingOption(void);
	void ShowCDSOption(void);

private:

	WORD image_piece_number_;
	WORD received_packet_data_length;
	
	ImageProcessing * pImageProcessing_;
	
};
