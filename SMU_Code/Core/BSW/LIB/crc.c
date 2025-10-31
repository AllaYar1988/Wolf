/*
 * crc.c
 *
 *  Created on: Jan 19, 2025
 *      Author: 98912
 */
#include "crc.h"


u16 calculateCRC(u8 *frame, u16 len) {
	unsigned int crc = 0xFFFF;
	int i;
	int pos;
	for (pos = 0; pos < len; pos++) {
		crc ^= (int) frame[pos];         // XOR byte into least sig. byte of crc
		for (i = 8; i != 0; i--) {    // Loop over each bit
			if ((crc & 0x0001) != 0) {      // If the LSB is set
				crc >>= 1;                    // Shift right and XOR 0xA001
				crc ^= 0xA001;
			} else
				// Else LSB is not set
				crc >>= 1;                    // Just shift right
		}
	}
	return crc;
}

bool checkFrameCRC(char frame[], int frameSize) {
	int crc = calculateCRC((u8*)frame, (u16)frameSize - 2);
	char calculatedCRC[2];
	calculatedCRC[0] = crc & 0x00ff;
	calculatedCRC[1] = (crc & 0xff00) >> 8;
	if (calculatedCRC[0] == frame[frameSize - 2]
			&& calculatedCRC[1] == frame[frameSize - 1])
		return true;
	else
		return false;
}



// Function to reverse the bits of a byte
static u8 bitReverse(u8 byte) {
    byte = ((byte & 0xF0) >> 4) | ((byte & 0x0F) << 4);
    byte = ((byte & 0xCC) >> 2) | ((byte & 0x33) << 2);
    byte = ((byte & 0xAA) >> 1) | ((byte & 0x55) << 1);
    return byte;
}

// Calculate CRC8 for STPM34 with bit reversal
// This implementation matches the STPM34 datasheet requirement for bit-reversed CRC
u8 crc_stpm3x(u8 *data, u8 len)
{
    u8 crc = 0; // Initial CRC value

    for (u8 i = 0; i < len; i++) {
        crc ^= bitReverse(data[i]); // Reverse bits before processing

        for (u8 j = 0; j < 8; j++) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0x07; // Polynomial used in STPM34
            } else {
                crc <<= 1;
            }
        }
    }

    return bitReverse(crc); // Reverse bits of the final CRC result
}
