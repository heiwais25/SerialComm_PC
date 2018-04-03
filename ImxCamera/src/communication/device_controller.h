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

typedef struct CameraParamsType {
	uint16_t cds_gain;
	uint16_t vga_gain;
	uint16_t black_level;
	uint16_t exposure_time;
	uint16_t num_pictures;
	uint16_t adc_minimum;
	uint16_t led_period;
} CameraParams;

typedef struct CameraAnalysisParamsType {
	uint16_t param;
	uint16_t startVal;
	uint16_t stepVal;
	uint16_t stepNum;
} CameraAnalysisParams;


class DeviceController : public PacketProtocol {
public:
	DeviceController(Uart * user_hUart) : PacketProtocol(user_hUart), image_piece_number_(0), kisAnalyzingParam(false) {
		pImageProcessing_ = new ImageProcessing();
		initCameraParams();
	};

	DeviceController(std::string port_number) : PacketProtocol(port_number), image_piece_number_(0), kisAnalyzingParam(false) {
		pImageProcessing_ = new ImageProcessing();
		initCameraParams();
	};
	~DeviceController() {};

	virtual void PickAndSendCommand(void);
	
	void ControlDevice(BYTE c);
	void EchoTest(void);

	void CameraTest();
	void ChooseCameraOperation(void);
	void CameraCaptureWithExposure(void);

	void SetCameraParamsValue(void);


	

	void ChangeCameraOption(void);
	void SetCDSGain(void);
	void SetVGAGain(void);
	void SetBlackLevel(void);
	void SetExposureTime(void);
	void SetImageToRead(void);
	void SavePixelLog();

	virtual void DoCommand(void);

	void CheckEchoTest();
	void CollectImageData(void);
	void CollectPackedImageData(void);
	void CollectPNGImageData(void);
	void SetLEDPeriod(void);
	void SetADCMinimum(void);
	void SendShortValue(BYTE command, int value);
	void SetNumberOfPicture(void);

	int getParamMax(int param); 
	int checkValidParam(int param, int startVal, int stepVal, int stepNum);

	void captureAndSaveContinuously();
	void initCameraParams();
	void changeCameraParams();

	void DoControlCamera(void);
	void DoImageTransmission(void);
	void DoImageProcess(void);
	void DoControlLED(void);

	void ShowLEDOption(void);
	void TurnOnLED(void);
	void TurnOffLED(void);
	void BlankLED(void);

	void ShowTestOptions(void);
	void ShowEchoTestOptions(void);
	void ShowCameraOperationOption(void);
	void ShowExposureOption(void);
	void ShowFineSettingList(void);
	void ShowCDSOption(void);
	void ShowControlCameraOption(void);
	void ShowImageTransmissionOption(void);
	void ShowImageProcessOption(void);

	int setCameraAnalysisParams();
	int isAnalyzingParam() { return kisAnalyzingParam; };
	CollectedImageFormat GetCorrectImgFormat(unsigned int dataTotalLength, BYTE command);
private:

	WORD image_piece_number_;
	WORD received_packet_data_length;
	int imgByteOrder;
	int kisAnalyzingParam;
	ImageProcessing * pImageProcessing_;
	CameraParams cameraParams;
	CameraAnalysisParams cameraAnalysisParams;
};
