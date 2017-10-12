#include "stdafx.h"
#include "protocol.h"

// Reading이 하는 동작
// 1. Device로부터 오는 신호를 그냥 읽기
// 2. Device에서 Packet이 오면 이를 캐치하여 동작
//		Packet의 구조는 Structure or Class?
//		STX부터 하나씩 더하도록...Data 구조니까 Structure로 하자.
// 3. 사용자로부터 '`'입력이 들어오면 break하여 PickAndSendCommand로 이동

/*
	Todo 
	1. 주석달기
	2. 필요없는 상수 지우기
		kIsPacketIncludeInfo


*/

Protocol::Protocol(Uart * user_hUart){
	pUart = user_hUart;
	pCrc32 = new Crc32();
	init_protocol();
}

Protocol::~Protocol(){
}

void Protocol::init_protocol() {
	kIsPacket = 0;						// Distinguish printf and packet data from device
	kIsFinishReceivePacket = 0;			// Telling it got whole packet or not
	packet_parsing_count = PACKET_STX;  // Set start point for packet parsing
	
	kIsPacketIncludeInfo = 0; // Is it needed?
	checksum = kChecksumStandardValue; // 여기서 초기화할 필요가 있는가?
}

// The basic action : read or write (just charactor or packet)
void Protocol::StandByMode(void) {
	while (1) {
		Reading();
		PickAndSendCommand();
	}
}

void Protocol::Reading(void) {
	WORD read_from_device;
	BYTE user_input;
	const BYTE kCommandKey = '`';
	ReceiveStatus slave_status;

	while (1) {
		read_from_device = pUart->Read();
		slave_status = CheckReceivedPacket(read_from_device);

		if(slave_status == SLAVE_SLEEP){ 
			if (user_input = GetOneCharKeyboardInput()) {
				if (user_input == kCommandKey) break; // Want to send packet
				else pUart->Write(user_input);        // Just send charactor
			}
		}
		else if(slave_status == SLAVE_NORMAL_CHARACTOR){
			std::cout << static_cast<BYTE>(read_from_device);
		}
		else if (slave_status == SLAVE_COMMAND) {
			try {
				GetPacket(read_from_device);
			}
			catch (PacketDataLengthError& e) {
				std::cerr << "Packet data length error" << std::endl;
			}
			catch (PacketChecksumError &e) {
				std::cerr << "Packet Checksum error" << std::endl;
			}
			catch (PacketEtxError& e) {
				std::cerr << "Packet ETX error" << std::endl;
			}
			catch (...) {
				std::cerr << "There are Error in getting packet" << std::endl;
			}
			if (kIsFinishReceivePacket) {
				DoCommand(); // It will be written in the inheritted class
				init_protocol();
			}
		}
	}
}

void Protocol::set_hSendingPacket_with_data(BYTE packet_number, BYTE command, WORD length, BYTE * data) {
	hSendingPacket.number = packet_number;
	hSendingPacket.command = command;
	hSendingPacket.length[0] = length & 0xff;
	hSendingPacket.length[1] = length >> 8;
	memcpy(hSendingPacket.data, data, length);
	set_ready_comm_State();
}

void Protocol::SendPacket(void) {
	// 1. send packet header
	// 2. send packet data
	// 3. send packet tail
	// during sending process, we need to calculate crc also

	// You need to set hSendingPacket before using this function
	if (!isPacketIncludeInfo()) {
		SplitLine();
		std::cerr << "Please check did you include hSendingPacket with your data" << std::endl;
		std::cerr << "If so, you might don't use set_ready_comm_State() function" << std::endl;
		std::cerr << "It is required to set to tell SendPacket you fill hSendingPacket" << std::endl;
		SplitLine();
		throw PacketSendingNoInfoError{};
	}
	
	SendPacketHeader();
	SendPacketData();
	SendPacketTail();
}

void Protocol::SendCommand(BYTE command) {
	set_hSendingPacket_with_data(0x01, command, 0x00, NULL);
	SendPacket();
}

// Need to set number, command
void Protocol::SendPacketHeader(void) { 
	// Send packet header(STX ~ COMMAND)
	checksum = kChecksumStandardValue;
	BYTE packet_number = hSendingPacket.number;
	BYTE packet_command = hSendingPacket.command;
	BYTE packet_piece[4] = { kStx , kSlaveId , packet_number , packet_command };

	pUart->Write(kStx);
	pUart->Write(kSlaveId);
	pUart->Write(packet_number);
	pUart->Write(packet_command);

	// Calculate Crc32
	for (int i = 0; i < 4; i++) {
		checksum = pCrc32->UpdateCrc32(checksum, packet_piece[i]);
	}
}

// Need to set length, data
void Protocol::SendPacketData(void) {
	// Send packet data
	int sending_data_count = 0;
	BYTE packet_length_0 = hSendingPacket.length[0];
	BYTE packet_length_1 = hSendingPacket.length[1];
	WORD packet_length = (packet_length_1 << 8) + packet_length_0;
	BYTE * pPacketData = hSendingPacket.data;

	pUart->Write(packet_length_0);
	checksum = pCrc32->UpdateCrc32(checksum, packet_length_0);

	pUart->Write(packet_length_1);
	checksum = pCrc32->UpdateCrc32(checksum, packet_length_1);

	if(packet_length != 0)
		while (packet_length--) {
			checksum = pCrc32->UpdateCrc32(checksum, pPacketData[sending_data_count]);
			pUart->Write(pPacketData[sending_data_count++]);
		}
}

void Protocol::SendPacketTail(void) {
	// Send packet tail
	calculated_checksum = checksum ^ kChecksumStandardValue;
	pUart->Write(static_cast<BYTE>( calculated_checksum & 0xff));
	pUart->Write(static_cast<BYTE>(calculated_checksum >> 8));
	pUart->Write(static_cast<BYTE>(calculated_checksum >> 16));
	pUart->Write(static_cast<BYTE>(calculated_checksum >> 24));
	pUart->Write(kEtx);
}

// 1. Forming Packet structure
// 2. Check valid packet
// 3. If valid, do command(나누는 게 낫지않나?)
void Protocol::GetPacket(WORD read_from_device) {
	BYTE packet_piece = static_cast<BYTE> (read_from_device);

	checksum = pCrc32->UpdateCrc32(checksum, packet_piece);
	switch (packet_parsing_count) { 
		case PACKET_STX:
			if (packet_piece == kStx) {
				kIsPacket = 1;
				init_packet_parameter(); // init data count, checksum count and value 
				checksum = pCrc32->UpdateCrc32(checksum, packet_piece);
				packet_parsing_count++;
			}
			break;

		case PACKET_SLAVE_ID:
			hReceivedPacket.slave_id = packet_piece;
			packet_parsing_count++;
			break;

		case PACKET_NUMBER:
			hReceivedPacket.number = packet_piece;
			packet_parsing_count++;
			break;

		case PACKET_COMMAND:
			hReceivedPacket.command = packet_piece;
			packet_parsing_count++;
			break;

		case PACKET_LENGTH_1:
			hReceivedPacket.length[0] = packet_piece;
			packet_parsing_count++;
			break;

		case PACKET_LENGTH_2:
			hReceivedPacket.length[1] = packet_piece;
			received_data_length = (hReceivedPacket.length[1] << 8) + hReceivedPacket.length[0];
			packet_parsing_count++;
			CheckPacketLength();
			break;

		case PACKET_DATA:
			hReceivedPacket.data[packet_data_count++] = packet_piece;
			if (--received_data_length == 0) {
				calculated_checksum = checksum ^ kChecksumStandardValue;
				packet_parsing_count = PACKET_CHECKSUM_1;
			}
			break;

		case PACKET_CHECKSUM_1:
		case PACKET_CHECKSUM_2:
		case PACKET_CHECKSUM_3:
		case PACKET_CHECKSUM_4:
			hReceivedPacket.checksum[packet_checksum_count++] = packet_piece;
			packet_parsing_count++;
			break;

		case PACKET_ETX:
			hReceivedPacket.etx = packet_piece;
			CheckPacketChecksumAndEtx();
			kIsFinishReceivePacket = 1;
			break;
	}
}

void Protocol::init_packet_parameter(void) {
	checksum = kChecksumStandardValue;
	packet_checksum_count = 0;
	packet_data_count = 0;
}

void Protocol::init_comm_state(void) {
	kIsPacketIncludeInfo = 0;
	kIsWaitResponse = 0;
}

void Protocol::set_ready_comm_State(void) {
	kIsPacketIncludeInfo = 1;
	kIsWaitResponse = 1;
}

void Protocol::CheckPacketLength(void) {
	packet_checksum_count = 0;
	if (received_data_length > kPacketDataSize) {
		throw PacketDataLengthError{};
	}
	else if (received_data_length == 0) { // No data : command packet
		calculated_checksum = checksum ^ kChecksumStandardValue;
		packet_parsing_count = PACKET_CHECKSUM_1;
	}
	else { // data include
		packet_parsing_count = PACKET_DATA;
	}
}

void Protocol::CheckPacketChecksumAndEtx(void) {

	if (hReceivedPacket.etx == kEtx) { // Correct packet
		received_checksum = (hReceivedPacket.checksum[3] << 24) + (hReceivedPacket.checksum[2] << 16) +
			(hReceivedPacket.checksum[1] << 8) + (hReceivedPacket.checksum[0]);
		if (calculated_checksum == received_checksum) { // Wrong Checksum
			return;
		}
		else {
			throw PacketChecksumError{};
		}
	}
	else { // Bad packet structure
		throw PacketEtxError{};
	}
}

// Check received word whether it is packet data or just charactor
ReceiveStatus Protocol::CheckReceivedPacket(WORD word_from_device) {
	if (word_from_device == kNoData)
		return SLAVE_SLEEP;
	else if (kIsPacket || kStx == (BYTE)word_from_device)
		return SLAVE_COMMAND;
	else
		return SLAVE_NORMAL_CHARACTOR;
}


