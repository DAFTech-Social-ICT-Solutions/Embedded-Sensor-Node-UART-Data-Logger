// unit tests
#include "../crc.h"
#include "../parser.h"
#include <stdint.h>
#include <stdio.h>

void test_crc_integrity(){
    printf("\n============ Test CRC Integrity ==================\n");
    uint8_t payload_example[4] = {1, 2, 55, 32};
    uint8_t result = generate_crc(payload_example);
    printf("\ninteger: [1 2 55 32]\nin hex:[0x01 0x02 0x37 0x20] -> CRC: 0x%02x\n",result);
}


void test_parser_integrity(uint8_t packet[7]){
    printf("\n============ Packet Test ================\n");
    for (int i=0; i<7; i++){
        parse_byte(packet[i]);
    }
}




int main(){
    test_crc_integrity();

    uint8_t valid_packet[7] = {0xAA, 0x01, 0x02, 0x0c, 0x22, 0xD2, 0x55};
    // CRC for data is precalculated 0xD2.
    uint8_t start_invalid[7] = {0xEE, 0x01, 0x02, 0x0c, 0x22, 0xD2, 0x55};
    uint8_t type_invalid[7] = {0xAA, 0xEE, 0x02, 0x0c, 0x22, 0xD2, 0x55};
    uint8_t length_invalid[7] = {0xAA, 0x01, 0xEE, 0x0c, 0x22, 0xD2, 0x55};
    uint8_t crc_invalid[7] = {0xAA, 0x01, 0x02, 0x0c, 0x22, 0xEE, 0x55};
    uint8_t end_invalid[7] = {0xAA, 0x01, 0x02, 0x0c, 0x22, 0xD2, 0xEE};

    test_parser_integrity(valid_packet);
    test_parser_integrity(start_invalid);
    test_parser_integrity(type_invalid);
    test_parser_integrity(length_invalid);
    test_parser_integrity(crc_invalid);
    test_parser_integrity(end_invalid);
    test_parser_integrity(valid_packet);


    return 0;
}

