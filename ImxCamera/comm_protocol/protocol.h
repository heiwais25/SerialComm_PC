#pragma once
#include "../uart/uart.h"
#include "../utility/utility.h"
#include "../comm_protocol/comm_protocol.h"
#include <Windows.h>


class Protocol
{
public:
	Protocol(Uart * user_hUart);
	~Protocol();
	
	void init_protocol(void);
	void init_packet_parameter(void);
	void init_comm_state(void);

	void set_ready_comm_State(void);

	Packet get_hReceivedPacket(void) { return hReceivedPacket; };
	Packet get_hSendingPacket(void) { return hSendingPacket; };

	int isWaitResponse(void) { return kIsWaitResponse_; };
	int isPacketIncludeInfo(void) { return kIsPacketIncludeInfo_; };

	void StandByMode();
	void Reading();

	

	virtual void PickAndSendCommand() = 0;
	virtual void DoCommand(void) = 0;

	void GetPacket(WORD read_from_device);

	void set_hSendingPacket_with_data(BYTE packet_number, BYTE command, WORD length, BYTE * data);

	void SendPacket(void);
	void SendCommand(BYTE command);

	void SendPacketHeader(void);
	void SendPacketData(void);
	void SendPacketTail(void);

	
	void CheckPacketLength(void);
	void CheckPacketChecksumAndEtx(void);



	ReceiveStatus CheckReceivedPacket(WORD read_from_device);

protected:
	
	Uart * pUart;
	Crc32 * pCrc32;
	Packet hReceivedPacket;
	Packet hSendingPacket;
	
	unsigned short received_data_length_;

	unsigned long checksum_;
	unsigned long calculated_checksum_;
	unsigned long received_checksum_;

	int packet_parsing_count_; // Set start point for packet parsing
	int packet_data_count_;
	int packet_checksum_count_;

	int kIsPacket_; // Distinguish printf and packet data from device
	int kIsFinishReceivePacket_; // Telling it got whole packet or not

	int kIsPacketIncludeInfo_; // To make stable the class when use SendPacket by inheritted class, it is required
	int kIsWaitResponse_; // If it is set, we need to wait the response from device
	
};

struct PacketDataLengthError {};
struct PacketEtxError {};
struct PacketChecksumError {};
struct PacketSendingNoInfoError {};