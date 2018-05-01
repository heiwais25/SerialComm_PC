#pragma once
#include "packet_protocol.h"
#include "image_processing.h"
#include "utility.h"
#include <queue>

const enum CAMERA_PARAMS {
	CDS_GAIN = 0,
	VGA_GAIN,
	BLACK_LEVEL,
	EXPOSRUE_TIME,
};


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
		paramsToSendNum = 0;
		initCameraParams();
	};

	DeviceController(std::string port_number) : PacketProtocol(port_number), image_piece_number_(0), kisAnalyzingParam(false) {
		pImageProcessing_ = new ImageProcessing();
		paramsToSendNum = 0;
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
	vector<BYTE> DeviceController::copyParamsToVector(CameraParams params);

	virtual void DoCommand(void);

	void CheckEchoTest();
	void CollectImageData(void);
	void SetLEDPeriod(void);
	void SetADCMinimum(void);
	void SendShortValue(BYTE command, int value);
	void SetNumberOfPicture(void);


	// Send camera params after reading it from paramSetInfo file
	// It will capture to the same number of set in the file
	void captureAndSendDataContinuously();

	// Send camera params and receive data analyzing our desired number of captured image
	void captureAndSendAnalyzedDataContinuously();

	void initCameraParams();

	void DoControlCamera(void);
	void DoImageTransmission(void);
	void DoImageProcess(void);
	void DoControlLED(void);

	void ShowLEDOption(void);
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

	int isAnalyzingParam() { return kisAnalyzingParam; };
	CollectedImageFormat GetCorrectImgFormat(unsigned int dataTotalLength, BYTE command);
	void DeviceController::saveLastCameraParam(int mode, int count);

private:

	WORD image_piece_number_;
	WORD received_packet_data_length;
	int imgByteOrder;
	int kisAnalyzingParam;
	ImageProcessing * pImageProcessing_;
	CameraParams currentParams;
	CameraAnalysisParams cameraAnalysisParams;
	queue<map<string, int> > paramsQueue;
	LastCameraParam lastCameraParam;
	int paramsToSendNum;
};
