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

const int kPacketDataSize = 4096 - 11;
const BYTE kStx = 0x02;
const BYTE kEtx = 0x03;
const BYTE kSlaveId = 0x01; // It can be changed if there are one more slave
const unsigned long kCrc32Param = 0xEDB88320L;
const unsigned long kChecksumStandardValue = 0xffffffff;

typedef struct _Packet {
	_Packet() :number(1), command(0xff), length{ 0 }, data{ 0 } {};
	BYTE stx;					// 1byte
	BYTE slave_id;				// 1byte
	BYTE number;				// 1byte
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
	PACKET_NUMBER,
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

	CAMERA_POWER_ON  = 10,
	CAMERA_POWER_OFF = 11,
	CAMERA_SHOW_GAIN_SETTING = 12,
	CAMERA_CAPTURE   = 13,
	CAMERA_SEND_CAPTURED_IMAGE = 14,
	CAMERA_SEND_NEXT_PACKET = 15,

	CAMERA_SET_CDS_GAIN = 16,
	CAMERA_SET_VGA_GAIN = 17,
	CAMERA_SET_BLACK_LEVEL = 18,

	CAMERA_SET_EXPOSURE_TIME = 17, // Need to wait


};

