#pragma once
#include <Windows.h>
#include "uart.h"
#include "utility.h"
#include "comm_protocol.h"

const enum SENDING_PACKET_STATE{
	PACKET_EMPTY = 0,
	PACKET_FILLED
};

const enum RECEIVING_PACKET_STATE {
	NOT_WATING = 0,
	WAITING = 1,
};

class PacketProtocol {
	public:
		PacketProtocol(Uart * user_hUart);
		PacketProtocol(std::string port_number);
		~PacketProtocol();
	
		//void InitPacketProtocol(void);
		void InitPacketParams(void);

		/*void init_comm_state(void);
		void IndicatePacketFilled(void);*/

		void IndicatePacketState(SENDING_PACKET_STATE state);

		Packet get_hReceivedPacket(void) { return hReceivedPacket; };
		Packet get_hSendingPacket(void) { return hSendingPacket; };

		int isWaitResponse(void) { return kIsWaitResponse_; };
		int isPacketIncludeInfo(void) { return kIsPacketFilled_; };

		void StandByMode();
		void ScanPort(void);

		virtual void PickAndSendCommand() = 0;
		virtual void DoCommand(void) = 0;

		void GetPacket(WORD read_from_device);

		void SetSendingPacketInfo(WORD packet_number, BYTE command, WORD length, BYTE * data);

		void SendPacket(void);
		void SendCommand(BYTE command);

		void SendPacketHeader(void);
		void SendPacketData(void);
		void SendPacketFooter(void);

	
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

		int kIsPacketFilled_; // To make stable the class when use SendPacket by inheritted class, it is required
		int kIsWaitResponse_; // If it is set, we need to wait the response from device
	
};

struct PacketDataLengthError {};
struct PacketEtxError {};
struct PacketChecksumError {};
struct PacketSendingNoInfoError {};
