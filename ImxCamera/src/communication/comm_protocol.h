#pragma once
#include <Windows.h>
#include <iostream>

class Crc32 {
public:
	Crc32();
	~Crc32();
	void init_crc32_table(void);
	unsigned long UpdateCrc32(unsigned long crc, BYTE c);

private:
	int isCrcTableInit;
	unsigned long crc32_table[256];
};

//const int kPacketDataSize = 4096 - 11;
const int kPacketDataSize = 1024 * 5 - 12;
const BYTE kStx = 0x02;
const BYTE kEtx = 0x03;
const BYTE kSlaveId = 0x01; // It can be changed if there are one more slave
const unsigned long kCrc32Param = 0xEDB88320L;
const unsigned long kChecksumStandardValue = 0xffffffff;

typedef struct _Packet {
	_Packet() :number{ 0 }, command(0xff), length{ 0 }, data{ 0 } {};
	BYTE stx;					// 1byte
	BYTE slave_id;				// 1byte
	BYTE number[2];				// 1byte
	BYTE command;				// 1byte
	BYTE length[2];				// 2byte
	BYTE data[kPacketDataSize]; // item detail transferred to board
	BYTE checksum[4];			// 4byte
	BYTE etx;					// 1byte
}Packet;

const enum ReceiveStatus {
	SLAVE_SLEEP = 0,
	SLAVE_NORMAL_CHARACTOR,
	SLAVE_COMMAND,
};

const enum PacketCount {
	PACKET_STX = 0,
	PACKET_SLAVE_ID,
	PACKET_NUMBER_1,
	PACKET_NUMBER_2,
	PACKET_COMMAND,
	PACKET_LENGTH_1,
	PACKET_LENGTH_2,
	PACKET_DATA,
	PACKET_CHECKSUM_1,
	PACKET_CHECKSUM_2,
	PACKET_CHECKSUM_3,
	PACKET_CHECKSUM_4,
	PACKET_ETX
};

const enum DeviceCommand {
	BAD_COMMUNICATION = 0x01,
	GOOD_COMMUNICATION = 0x02,

	ECHO_TEST = 0x05,

	// CAMERA_CONTROL(10~30)
	CAMERA_POWER_ON  = 10,
	CAMERA_POWER_OFF,
	CAMERA_CAPTURE,
	CAMERA_SET_CDS_GAIN,
	CAMERA_SET_VGA_GAIN,
	CAMERA_SET_BLACK_LEVEL,
	CAMERA_SET_EXPOSURE_TIME,
	CAMERA_SET_LED_PERIOD,
	CAMERA_SET_NUMBER_OF_PICTURES,
	CAMERA_SET_ADC_MINIMUM,
	CAMERA_CAPTURE_AND_TRANSMIT,
	CAMERA_SEND_CAMERA_PARAMS,


	// IMAGE_TRANSMISSION(30~49)
	CAMERA_SEND_CAPTURED_IMAGE = 30,
	CAMERA_SEND_PACKED_DATA,
	CAMERA_SEND_PNG,
	CAMERA_SEND_NEXT_PACKET,
	COMMAND_CAMERA_SEND_LAST_ACTIVATED_MODE,
	CAMERA_SEND_CUT_OFF_IMAGE,
	CAMERA_SEND_PACKED_PNG_CUT_OFF_IMAGE,
	CAMERA_SEND_PIXEL_MEAN,
	CAMERA_SEND_PIXEL_DEV,
	CAMERA_SEND_PIXEL_LOG,
	//CAMERA_SEND_HIGH_BYTES_PNG_FORMAT,
	//CAMERA_SEND_LOW_BYTES_PNG_FORMAT,

	// IMAGE_PROCESS(50~69)
	PACK_IMAGE = 50,
	PACK_CONVERT_PNG,
	SAVE_IMAGE,
	READ_IMAGE,
	SHOW_STORED_IMAGE_LIST,
	DELETE_LAST_NODE,
	CUT_IMAGE,
	PACK_CUT_OFF_CONVERT_PNG,
	CONVERT_MEAN_PNG,
	CONVERT_DEV_PNG,
	//CONVERT_IMG_HIGH_BYTE_TO_PNG,
	
	// LED_CONTROL(70~89)
	TURN_ON_LED = 70,
	TURN_OFF_LED,
	BLANK_LED,

	
	

	

};

