#include "crc.h"
#include <stdint.h>

// takes 4 arguments and calculates the CRC
// CRC from type, length, data[0] and data[1]
//

uint8_t generate_crc(uint8_t *payload){
    uint8_t crc_result = 0x00;

    for (int byte=0; byte<4; byte++){
        crc_result ^= payload[byte];
        // now doing left shifts until the MSB (bit)is 1
        for (int bit=0; bit<8; bit++){
            // to check if the MSB is 1;crc ANDed with 1000 0000
            // it will result in 1000 0000 if yes
            // 1000 0000 = 0x80
            if (crc_result & 0x80){
                crc_result = crc_result << 1;
                crc_result ^= 0x07;// crc-8 polynomial
            }
            else{
            crc_result = crc_result<<1;
            }
        }
    }
    return crc_result;

}
