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

	// IMAGE_TRANSMISSION(30~40)
	CAMERA_SEND_CAPTURED_IMAGE = 30,
	CAMERA_SEND_PACKED_DATA,
	CAMERA_SEND_PNG,
	CAMERA_SEND_NEXT_PACKET,
	COMMAND_CAMERA_SEND_LAST_ACTIVATED_MODE,

	// IMAGE_PROCESS(40~50)
	PACK_IMAGE = 40,
	PACK_CONVERT_PNG,
	SAVE_IMAGE,
	READ_IMAGE,
	SHOW_STORED_IMAGE_LIST,
	DELETE_LAST_NODE,
	CAMERA_CAPTURE_LONG_EXPOSURE, // Need to wait
	
	// LED_CONTROL(50~60)
	TURN_ON_LED = 50,
	TURN_OFF_LED,
	BLANK_LED,

	
	

	

};

