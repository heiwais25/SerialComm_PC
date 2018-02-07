#include "stdafx.h"
#include "comm_protocol.h"

Crc32::Crc32() {
	isCrcTableInit = FALSE;
	init_crc32_table();
}

Crc32::~Crc32() {

}

void Crc32::init_crc32_table(void) {
	unsigned long crc;
	for (int i = 0; i<256; i++) {
		crc = (unsigned long)i;
		for (int j = 0; j<8; j++) {
			if (crc & 0x00000001L) crc = (crc >> 1) ^ kCrc32Param;
			else crc = crc >> 1;
		}
		crc32_table[i] = crc;
	}
	isCrcTableInit = TRUE;
}

unsigned long Crc32::UpdateCrc32(unsigned long crc, BYTE c) {
	unsigned long long_c = 0x000000ffL & (unsigned long)c;
	/*if (!isCrcTableInit)
		init_crc32_table();*/
	unsigned long xor_crc = crc ^ long_c;
	unsigned long crc_updated = (crc >> 8) ^ crc32_table[xor_crc & 0xff];
	return crc_updated;
}