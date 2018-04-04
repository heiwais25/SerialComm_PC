#include "stdafx.h"
#include "packet_protocol.h"

/* Protocol.cc
- The basic action 
	1. Reading
	2. PickAndSendCommand

- Reading
	- Reading the signal from the port continuously
	- If the signal is the charactor just written by printf, it will print to user console
	- If the signal is part of Packet, it will form the packet
	- If PC get packet successfully, it will jump out to "DoCommand" and execute depending on the command from device
	- "DoCommand" function need to be re-definned by inheritted class

- PickAndSendCommand
	- When there are no signal from the device, it will be called.
	- We can just send charactor to device
	- Also we can send packet to make device do depending on the command
	- "PickAndSendCommand" function need to be re-definned by inheritted class

- To write Packet to device, you can use "SendPacket()" after using "SetSendingPacketInfo()" function
	Order : 1)"SetSendingPacketInfo()" 2) "SendPacket()"
	- If you want to send just command, use "SendCommand()" with your specific command

	2017-10-14 JongHyun */

PacketProtocol::PacketProtocol(Uart * user_hUart){
	pUart = user_hUart;
	pCrc32 = new Crc32();
	//InitPacketProtocol();
	InitPacketParams();
}

PacketProtocol::PacketProtocol(std::string port_number) {
	pUart = new Uart();
	pUart->OpenPort(port_number);
	pCrc32 = new Crc32();
	//InitPacketProtocol();
	InitPacketParams();
}


PacketProtocol::~PacketProtocol(){
}



// The basic action : read or write (just charactor or packet)
void PacketProtocol::StandByMode(void) {
	while (1) {
		ScanPort();
		PickAndSendCommand();
	}
}

// When there are signal at the port, scan the value
void PacketProtocol::ScanPort(void) {
	WORD data_from_device;
	BYTE user_input;
	ReceiveStatus slave_status;

	const BYTE kCommandKey = '`';

	while (1) {
		data_from_device = pUart->Read();
		slave_status = CheckReceivedPacket(data_from_device);

		if(slave_status == SLAVE_SLEEP){ 

			if (user_input = GetOneCharKeyboardInput()) {
				if (user_input == kCommandKey) break; // Want to send packet
				else pUart->Write(user_input);        // Just send charactor
			}
		}
		else if(slave_status == SLAVE_NORMAL_CHARACTOR){
			ReadColorFont(static_cast<BYTE>(data_from_device));
		}
		else if (slave_status == SLAVE_COMMAND) {
			try {
				GetPacket(data_from_device);
			}
			catch (PacketDataLengthError& e) {
				std::cerr << "Packet data length error" << endl;
			}
			catch (PacketChecksumError &e) {
				std::cerr << "Packet Checksum error" << endl;
			}
			catch (PacketEtxError& e) {
				std::cerr << "Packet ETX error" << endl;
			}
			catch (...) {
				std::cerr << "There are Error in getting packet" << endl;
			}
			if (kIsFinishReceivePacket_) {
				DoCommand(); // It will be written in the inheritted class
				//InitPacketProtocol();
				InitPacketParams();
			}
		}
	}
}



 // Set packet information required to send packet to device
void PacketProtocol::SetSendingPacketInfo(WORD packet_number, BYTE command, WORD length, BYTE * data) {
	if (length > kPacketDataSize) {
		SplitLine();
		std::cerr << "Packet data size exceed the maximum packet data size " << length << " > " << kPacketDataSize << endl;
		throw PacketDataLengthError{};
	}
	hSendingPacket.number[0] = packet_number & 0Xff;
	hSendingPacket.number[1] = packet_number >> 8;
	hSendingPacket.command = command;
	hSendingPacket.length[0] = length & 0xff;
	hSendingPacket.length[1] = length >> 8;
	memcpy(hSendingPacket.data, data, length);
	IndicatePacketState(PACKET_FILLED);
}

// Set packet information required to send packet to device
// In the case of vector
void PacketProtocol::SetSendingPacketInfo(WORD packet_number, BYTE command, vector<BYTE> data) {
	int length = data.size();
	if (length > kPacketDataSize) {
		SplitLine();
		std::cerr << "Packet data size exceed the maximum packet data size " << length << " > " << kPacketDataSize << endl;
		throw PacketDataLengthError{};
	}
	hSendingPacket.number[0] = packet_number & 0Xff;
	hSendingPacket.number[1] = packet_number >> 8;
	hSendingPacket.command = command;
	hSendingPacket.length[0] = length & 0xff;
	hSendingPacket.length[1] = length >> 8;
	memcpy(hSendingPacket.data, &data[0], length);
	IndicatePacketState(PACKET_FILLED);
}


// Send only command
void PacketProtocol::SendCommand(BYTE command) {
	SetSendingPacketInfo(0x01, command, 0x00, NULL);
	SendPacket();
}

void PacketProtocol::SendPacket(void) {
	// 1. send packet header
	// 2. send packet data
	// 3. send packet tail
	// during sending process, we need to calculate crc also

	// You need to set hSendingPacket before using this function
	if (!isPacketIncludeInfo()) {
		SplitLine();
		std::cerr << "Please check did you include hSendingPacket with your data" << endl;
		std::cerr << "If so, you might don't use set_ready_comm_State() function" << endl;
		std::cerr << "It is required to set to tell SendPacket you fill hSendingPacket" << endl;
		SplitLine();
		throw PacketSendingNoInfoError{};
	}
	
	SendPacketHeader();
	SendPacketData();
	SendPacketFooter();
}


// Need to set number, command
void PacketProtocol::SendPacketHeader(void) {
	// Send packet header(STX ~ COMMAND)
	checksum_ = kChecksumStandardValue;
	BYTE packet_number_0 = hSendingPacket.number[0];
	BYTE packet_number_1 = hSendingPacket.number[1];
	BYTE packet_command = hSendingPacket.command;
	BYTE packet_piece[5] = { kStx , kSlaveId , packet_number_0, packet_number_1, packet_command};

	pUart->Write(kStx);
	pUart->Write(kSlaveId);
	pUart->Write(packet_number_0);
	pUart->Write(packet_number_1);
	pUart->Write(packet_command);

	// Calculate Crc32
	for (int i = 0; i < sizeof(packet_piece); i++) 
		checksum_ = pCrc32->UpdateCrc32(checksum_, packet_piece[i]);
	
}

// Need to set length, data
void PacketProtocol::SendPacketData(void) {
	// Send packet data
	int sending_data_count = 0;
	BYTE packet_length_0 = hSendingPacket.length[0];
	BYTE packet_length_1 = hSendingPacket.length[1];
	WORD packet_length = (packet_length_1 << 8) + packet_length_0;
	BYTE * pPacketData = hSendingPacket.data;

	pUart->Write(packet_length_0);
	checksum_ = pCrc32->UpdateCrc32(checksum_, packet_length_0);

	pUart->Write(packet_length_1);
	checksum_ = pCrc32->UpdateCrc32(checksum_, packet_length_1);

	if(packet_length != 0)
		while (packet_length--) {
			checksum_ = pCrc32->UpdateCrc32(checksum_, pPacketData[sending_data_count]);
			pUart->Write(pPacketData[sending_data_count++]);
		}
}

void PacketProtocol::SendPacketFooter(void) {
	// Send packet tail
	calculated_checksum_ = checksum_ ^ kChecksumStandardValue;
	pUart->Write(static_cast<BYTE>(calculated_checksum_ & 0xff));
	pUart->Write(static_cast<BYTE>(calculated_checksum_ >> 8));
	pUart->Write(static_cast<BYTE>(calculated_checksum_ >> 16));
	pUart->Write(static_cast<BYTE>(calculated_checksum_ >> 24));
	pUart->Write(kEtx);
}

// 1. Forming Packet structure
// 2. Check valid packet
// 3. If valid, do command(나누는 게 낫지않나?)
void PacketProtocol::GetPacket(WORD read_from_device) {
	BYTE packet_piece = static_cast<BYTE> (read_from_device);
	checksum_ = pCrc32->UpdateCrc32(checksum_, packet_piece);
	switch (packet_parsing_count_) { 
		case PACKET_STX:
			if (packet_piece == kStx) {
				InitPacketParams(); // init data count, checksum count and value 
				kIsPacket_ = 1;
				checksum_ = pCrc32->UpdateCrc32(checksum_, packet_piece);
				packet_parsing_count_++;
			}
			break;

		case PACKET_SLAVE_ID:
			hReceivedPacket.slave_id = packet_piece;
			packet_parsing_count_++;
			break;

		case PACKET_NUMBER_1:
			hReceivedPacket.number[0] = packet_piece;
			packet_parsing_count_++;
			break;

		case PACKET_NUMBER_2:
			hReceivedPacket.number[1] = packet_piece;
			packet_parsing_count_++;
			break;

		case PACKET_COMMAND:
			hReceivedPacket.command = packet_piece;
			packet_parsing_count_++;
			break;

		case PACKET_LENGTH_1:
			hReceivedPacket.length[0] = packet_piece;
			packet_parsing_count_++;
			break;

		case PACKET_LENGTH_2:
			hReceivedPacket.length[1] = packet_piece;
			received_data_length_ = (hReceivedPacket.length[1] << 8) + hReceivedPacket.length[0];
			packet_parsing_count_++;
			CheckPacketLength();
			break;

		case PACKET_DATA:
			hReceivedPacket.data[packet_data_count_++] = packet_piece;
			if (--received_data_length_ == 0) {
				calculated_checksum_ = checksum_ ^ kChecksumStandardValue;
				packet_parsing_count_ = PACKET_CHECKSUM_1;
			}
			break;

		case PACKET_CHECKSUM_1:
		case PACKET_CHECKSUM_2:
		case PACKET_CHECKSUM_3:
		case PACKET_CHECKSUM_4:
			hReceivedPacket.checksum[packet_checksum_count_++] = packet_piece;
			packet_parsing_count_++;
			break;

		case PACKET_ETX:
			hReceivedPacket.etx = packet_piece;
			CheckPacketChecksumAndEtx();
			kIsFinishReceivePacket_ = 1;
			break;
	}
}

//void PacketProtocol::InitPacketProtocol() {
//	kIsPacket_ = 0;
//	kIsFinishReceivePacket_ = 0;
//	packet_parsing_count_ = PACKET_STX;
//	kIsPacketFilled_ = PACKET_EMPTY;
//	checksum_ = kChecksumStandardValue;
//}

void PacketProtocol::InitPacketParams(void) {
	checksum_ = kChecksumStandardValue;
	packet_checksum_count_ = 0;
	packet_data_count_ = 0;
	kIsPacketFilled_ = PACKET_EMPTY;
	kIsPacket_ = 0;
	kIsFinishReceivePacket_ = 0;
	packet_parsing_count_ = PACKET_STX;
}

//void PacketProtocol::init_comm_state(void) {
//	kIsPacketFilled_ = PACKET_EMPTY;
//	//kIsWaitResponse_ = 0;
//}

// Inidicate the latest packed is filled with data which means it can be transmitted
void PacketProtocol::IndicatePacketState(SENDING_PACKET_STATE state) {
	kIsPacketFilled_ = state;
	kIsWaitResponse_ = state ? WAITING : NOT_WATING;
}

// Check the packet length is within the range
void PacketProtocol::CheckPacketLength(void) {
	packet_checksum_count_ = 0;
	if (received_data_length_ > kPacketDataSize) {
		throw PacketDataLengthError{};
	}
	
	if (received_data_length_ == 0) { // No data : command packet
		calculated_checksum_ = checksum_ ^ kChecksumStandardValue;
		packet_parsing_count_ = PACKET_CHECKSUM_1;
	}
	else { // data include
		packet_parsing_count_ = PACKET_DATA;
	}
}

void PacketProtocol::CheckPacketChecksumAndEtx(void) {
	BYTE * checksum = &hReceivedPacket.checksum[0];
	if (hReceivedPacket.etx == kEtx) { // Correct packet
		received_checksum_ = (checksum[3] << 24) + (checksum[2] << 16) +
								(checksum[1] << 8) + (checksum[0]);
		if (calculated_checksum_ == received_checksum_)  // Wrong Checksum
			return;
		else 
			throw PacketChecksumError{};
	}
	else  // Bad packet structure
		throw PacketEtxError{};
}

// Check received word whether it is packet data or just charactor
ReceiveStatus PacketProtocol::CheckReceivedPacket(WORD word_from_device) {
	if (word_from_device == kNoData)
		return SLAVE_SLEEP;
	else if (kIsPacket_ || kStx == (BYTE)word_from_device)
		return SLAVE_COMMAND;
	else
		return SLAVE_NORMAL_CHARACTOR;
}


