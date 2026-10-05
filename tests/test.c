// unit tests
#include "../crc.h"
#include "../parser.h"
#include "../ring_buffer.h"
#include "../statistics.h"

#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <assert.h>

void test_crc_integrity(){
    printf("\n============ Test CRC Integrity ==================\n");

    uint8_t payload_1[4] = {1, 2, 55, 32};
    uint8_t result_1 = generate_crc(payload_1);
    uint8_t payload_2[4] = {1, 2, 0xFF, 0xFF};
    uint8_t result_2 = generate_crc(payload_2);
    uint8_t payload_3[4] = {1, 2, 0x00, 0x00};
    uint8_t result_3 = generate_crc(payload_3);
    uint8_t payload_4[4] = {0xAF, 0xBF, 0xCF, 0xDF};
    uint8_t result_4 = generate_crc(payload_4);
    assert( result_1 == 0xb2); // tested against real world calculators
    assert( result_2 == 0xE4); // tested against real world calculators
    assert( result_3 == 0xC0); // tested against real world calculators
    assert( result_4 == 0xBD); // tested against real world calculators
    printf("CRC Integrity TEST PASSED!");
}


void test_parser_integrity(uint8_t stream[]){
    uint8_t valid_packets = 0;
    uint8_t crc_error = 0;
    uint8_t length_error = 0;
    uint8_t type_error = 0;
    uint8_t bad_end_error = 0;
    uint8_t buffer_overflow_errors = 0;

    printf("\n============ Packet Test ================\n");

    for (int byte = 0; byte < 350; byte++){
        printf("== Byte %d = %02x =", byte, stream[byte]);
        switch(parse_byte(stream[byte])){
            case PARSER_WAITING:
                printf(" Waiting for start byte...\n");
                break;

            case PARSER_OK:
                printf(" PARSER_OK\n");
                break;

            case PARSER_CRC_ERROR:
                printf(" CRC Error\n");
                crc_error++;
                break;

            case PARSER_TYPE_ERROR:
                 printf(" Type Error\n");
                 type_error++;
                break;

            case PARSER_LENGTH_ERROR:
                 printf(" Length Error\n");
                 length_error++;
                break;

            case PARSER_BAD_END:
                 printf(" Bad End Error\n");
                 bad_end_error++;
                break;

            case PARSER_PACKET_COMPLETE:
                printf(" Packet validated: saving...  ");
                valid_packets++;

                uint16_t temp = (uint16_t) (parsed_data.data_high << 8 )| ((uint16_t)parsed_data.data_low);
                measurement_t measured = {
                    .temperature=temp,
                    .timestamp=time(NULL)
                };

                if (!buffer_push(measured)){
                    printf("Buffer: Overflow error\n");
                    buffer_overflow_errors++;

                }
                else{
                    printf("Buffer: Saved!\n");
                }
                break;
        }
    }
    uint16_t total_errors = crc_error + type_error + length_error +
    bad_end_error + buffer_overflow_errors;

    printf("\n=================== SUMMARY REPORT ===================\n");
    printf(" Valid Packets Parsed:  %u\n", valid_packets);
    printf(" Stored Packets: %u\n", buffer_count());
    printf(" Total Errors Detected: %u\n", total_errors);
    printf("---------------------------------------------------------\n");
    printf("  - CRC Errors:             %u\n", crc_error);
    printf("  - Type Errors:            %u\n", type_error);
    printf("  - Length Errors:          %u\n", length_error);
    printf("  - Framing/Bad End Errors: %u\n", bad_end_error);
    printf("  - Buffer Overflow Errors: %u\n", buffer_overflow_errors);
    printf("===========================================================\n\n");
}

void test_statistics_integrity(){
    int16_t max = get_max();
    int16_t min = get_min();
    int32_t avg = get_average();

    printf("\n========== Current Buffer State ===============");
    measurement_t m;
    for (uint8_t index = 0; index < buffer_count(); index++ ){
        buffer_peek_at(index, &m);
        printf("\nBuffer %d: %d", index, m.temperature);
    }

    printf("\n================ Statistics ==================\n");
    printf("=  MAX : %d.%02d*C\n", max/100, abs(max)%100);
    printf("=  MIN : %d.%02d*C\n", min/100, abs(min)%100);
    printf("=  AVG : %d.%02d*C\n", avg/100, abs(avg)%100);

   /*
    the buffer is expected to have exactly these data at the end of stream:
    Buffer at 0: 4334
    Buffer at 1: -1500
    Buffer at 2: 0
    Buffer at 3: 32767
    Buffer at 4: -32768
    Buffer at 5: -18361
    Buffer at 6: -3951
    Buffer at 7: -5372
    Buffer at 8: -10856
    Buffer at 9: -13283
    Buffer at 10: 15741
    Buffer at 11: -14303
    Buffer at 12: 7651
    Buffer at 13: -17918
    Buffer at 14: -18048
    Buffer at 15: -13860*/
    // Sum = -89727 count = 16 avg = −5607.9375 now C truncates decimal points for integer division
    // Expected answer = -5607 or -56.07*C
    assert(max == 32767);
    assert(min == -32768);
    assert(avg == -5607);// -5607.9375 from online average calculators

    printf("Statistics test: PASSED!");
    // TODO add test cases for buffer that is not full buffer and that of empty buffer.
}

uint8_t test_stream[350] = {
    0xAA, 0x01, 0x02, 0x10, 0xEE, 0x13, 0x55, // Pkt 01: VALID: Val =  +4334 (0x10EE) | CRC = 0x13
    0xAA, 0x01, 0x02, 0xFA, 0x24, 0xAA, 0x55, // Pkt 02: VALID: Val =  -1500 (0xFA24) | CRC = 0xAA
    0xAA, 0x01, 0x02, 0x00, 0x00, 0xC0, 0x55, // Pkt 03: VALID: Val =     +0 (0x0000) | CRC = 0xC0
    0xEE, 0x01, 0x02, 0xCE, 0x60, 0xDC, 0x55, // Pkt 04: INVALID START: Byte = 0xEE (Expected 0xAA)
    0xAA, 0x01, 0x02, 0x7F, 0xFF, 0x52, 0x55, // Pkt 05: VALID: Val = +32767 (0x7FFF) | CRC = 0x52
    0xAA, 0x01, 0x02, 0x80, 0x00, 0x76, 0x55, // Pkt 06: VALID: Val = -32768 (0x8000) | CRC = 0x76
    0xAA, 0x01, 0x02, 0xB8, 0x47, 0xF5, 0x55, // Pkt 07: VALID: Val = -18361 (0xB847) | CRC = 0xF5
    0xAA, 0x99, 0x02, 0xF8, 0x48, 0x65, 0x55, // Pkt 08: INVALID TYPE: Type = 0x99 (Expected 0x01)
    0xAA, 0x01, 0x02, 0xF0, 0x91, 0x2A, 0x55, // Pkt 09: VALID: Val =  -3951 (0xF091) | CRC = 0x2A
    0xAA, 0x01, 0x02, 0xEB, 0x04, 0x08, 0x55, // Pkt 10: VALID: Val =  -5372 (0xEB04) | CRC = 0x08
    0xAA, 0x01, 0x02, 0xD5, 0x98, 0xFA, 0x55, // Pkt 11: VALID: Val = -10856 (0xD598) | CRC = 0xFA
    0xAA, 0x01, 0x02, 0xCC, 0x1D, 0x82, 0x55, // Pkt 12: VALID: Val = -13283 (0xCC1D) | CRC = 0x82
    0xAA, 0x01, 0x02, 0x3D, 0x7D, 0xA4, 0x55, // Pkt 13: VALID: Val = +15741 (0x3D7D) | CRC = 0xA4
    0xAA, 0x01, 0x02, 0xC8, 0x21, 0x62, 0x55, // Pkt 14: VALID: Val = -14303 (0xC821) | CRC = 0x62
    0xAA, 0x01, 0x0A, 0x49, 0x0A, 0x41, 0x55, // Pkt 15: INVALID LENGTH: Len = 0x0A (Expected 0x02)
    0xAA, 0x01, 0x02, 0x1D, 0xE3, 0xD9, 0x55, // Pkt 16: VALID: Val =  +7651 (0x1DE3) | CRC = 0xD9
    0xAA, 0x01, 0x02, 0xBA, 0x02, 0x03, 0x55, // Pkt 17: VALID: Val = -17918 (0xBA02) | CRC = 0x03
    0xAA, 0x01, 0x02, 0xB9, 0x80, 0xBB, 0x55, // Pkt 18: VALID: Val = -18048 (0xB980) | CRC = 0xBB
    0x7F, 0x12, 0xFE, // GARBAGE NOISE: 3 non-sync bytes inserted
    0xAA, 0x01, 0x02, 0xC9, 0xDC, 0x8A, 0x55, // Pkt 19: VALID: Val = -13860 (0xC9DC) | CRC = 0x8A
    0xAA, 0x01, 0x02, 0xE9, 0xD8, 0x38, 0x55, // Pkt 20: VALID: Val =  -5672 (0xE9D8) | CRC = 0x38
    0xAA, 0x01, 0x02, 0xED, 0x6F, 0x60, 0x55, // Pkt 21: VALID: Val =  -4753 (0xED6F) | CRC = 0x60
    0xAA, 0x01, 0x02, 0x33, 0x3E, 0x43, 0x55, // Pkt 22: INVALID CRC: Corrupted to 0x43
    0xAA, 0x01, 0x02, 0x4B, 0xFD, 0xF1, 0x55, // Pkt 23: VALID: Val = +19453 (0x4BFD) | CRC = 0xF1
    0xAA, 0x01, 0x02, 0xB8, 0xAB, 0x7F, 0x55, // Pkt 24: VALID: Val = -18261 (0xB8AB) | CRC = 0x7F
    0xAA, 0x01, 0x02, 0x41, 0x8D, 0x24, 0x55, // Pkt 25: VALID: Val = +16781 (0x418D) | CRC = 0x24
    0xAA, 0x01, 0x02, 0xE4, 0xC7, 0x8C, 0x55, // Pkt 26: VALID: Val =  -6969 (0xE4C7) | CRC = 0x8C
    0xAA, 0x01, 0x02, 0x3D, 0x61, 0xF0, 0x55, // Pkt 27: VALID: Val = +15713 (0x3D61) | CRC = 0xF0
    0xAA, 0x01, 0x02, 0x1D, 0x45, 0xA2, 0x55, // Pkt 28: VALID: Val =  +7493 (0x1D45) | CRC = 0xA2
    0xAA, 0x01, 0x02, 0xEA, 0x4E, 0xEC, 0x55, // Pkt 29: VALID: Val =  -5554 (0xEA4E) | CRC = 0xEC
    0xAA, 0x01, 0x02, 0x24, 0xDF, 0x29, 0xEE, // Pkt 30: INVALID END: Byte = 0xEE (Expected 0x55)
    0xAA, 0x01, 0x02, 0x48, 0xBA, 0x1C, 0x55, // Pkt 31: VALID: Val = +18618 (0x48BA) | CRC = 0x1C
    0xAA, 0x01, 0x02, 0xF9, 0x17, 0x0C, 0x55, // Pkt 32: VALID: Val =  -1769 (0xF917) | CRC = 0x0C
    0xAA, 0x01, 0x02, 0xB3, 0x89, 0x06, 0x55, // Pkt 33: VALID: Val = -19575 (0xB389) | CRC = 0x06
    0xAA, 0x01, 0x02, 0xDA, 0xBF, 0xCC, 0x55, // Pkt 34: VALID: Val =  -9537 (0xDABF) | CRC = 0xCC
    0xAA, 0x01, 0x02, 0x1E, 0x10, 0x31, 0x55, // Pkt 35: VALID: Val =  +7696 (0x1E10) | CRC = 0x31
    0xAA, 0x01, 0x02, 0x08, 0xFA, 0x80, 0x55, // Pkt 36: VALID: Val =  +2298 (0x08FA) | CRC = 0x80
    0xAA, 0x01, 0x02, 0xF9, 0x02, 0x67, 0x55, // Pkt 37: VALID: Val =  -1790 (0xF902) | CRC = 0x67
    0xAA, 0x01, 0x02, 0xD9, // Pkt 38: INCOMPLETE PACKET: Truncated to 4 bytes (Cut mid-payload)
    0xAA, 0x01, 0x02, 0xE8, 0xFE, 0xDF, 0x55, // Pkt 39: VALID: Val =  -5890 (0xE8FE) | CRC = 0xDF
    0xAA, 0x01, 0x02, 0x08, 0x0B, 0x59, 0x55, // Pkt 40: VALID: Val =  +2059 (0x080B) | CRC = 0x59
    0xAA, 0x01, 0x02, 0xCC, 0x0A, 0xE7, 0x55, // Pkt 41: VALID: Val = -13302 (0xCC0A) | CRC = 0xE7
    0xAA, 0x01, 0x02, 0xC9, 0x9E, 0x43, 0x55, // Pkt 42: VALID: Val = -13922 (0xC99E) | CRC = 0x43
    0xAA, 0x01, 0x02, 0x13, 0x22, 0x46, 0x55, // Pkt 43: VALID: Val =  +4898 (0x1322) | CRC = 0x46
    0xAA, 0x01, 0x02, 0xCA, 0xA2, 0xC8, 0x55, // Pkt 44: VALID: Val = -13662 (0xCAA2) | CRC = 0xC8
    0xAA, 0x01, 0x02, 0x0D, 0xC6, 0x8A, 0x55, // Pkt 45: INVALID CRC: Corrupted to 0x8A
    0xAA, 0x01, 0x02, 0x09, 0xED, 0xF0, 0x55, // Pkt 46: VALID: Val =  +2541 (0x09ED) | CRC = 0xF0
    0xAA, 0x01, 0x02, 0x4C, 0x6D, 0x63, 0x55, // Pkt 47: VALID: Val = +19565 (0x4C6D) | CRC = 0x63
    0xAA, 0x01, 0x02, 0xF5, 0x97, 0x79, 0x55, // Pkt 48: VALID: Val =  -2665 (0xF597) | CRC = 0x79
    0xAA, 0x01, 0x02, 0xBC, 0xFF, 0x80, 0x55, // Pkt 49: VALID: Val = -17153 (0xBCFF) | CRC = 0x80
    0xAA, 0x01, 0x02, 0x27, 0x7C, 0x76, 0x55 // Pkt 50: VALID: Val = +10108 (0x277C) | CRC = 0x76
};

void test_buffer_integrity(){
    // the valid info stream: 4334 -> -1500 -> 0 ...
    measurement_t m;
    printf("\n\n============ Buffer Test ================");
    printf("\n Running buffer tests on current buffer state");

    assert (buffer_count() == 16 );
    buffer_pop(&m);
    assert(buffer_count() == 15);
    assert(m.temperature == 4334);// it is FIFO
    printf("\n pop -> %d (count %d)",m.temperature, buffer_count());

    buffer_pop(&m);
    assert(buffer_count() == 14);
    assert(m.temperature == -1500); // it is FIFO
    printf("\n pop -> %d (count %d)",m.temperature, buffer_count());

    buffer_push(m); // should put -1500 to last queue
    assert(buffer_count() == 15);
    printf("\n push -> %d (count %d)",m.temperature, buffer_count());

    buffer_pop(&m);
    assert(buffer_count() == 14);
    assert(m.temperature == 0); // should not pop the last in. But keep sequence
    printf("\n pop -> %d (count %d)",m.temperature, buffer_count());

    // Test does it handle out of bound pops?
    printf("\nPerform boundary tests");
    for (int i = 0; i < 20; i++){
        buffer_pop(&m);
    }
    assert(buffer_count() == 0);
    // Test does it handle out of bound pushs?
    for (int i = 0; i < 20; i++){
        buffer_push(m);
    }
    assert(buffer_count() == 16);
    printf("\nCyclic buffer tests: PASSED!");
}

int main(){
    test_crc_integrity();
    test_parser_integrity(test_stream);
    test_statistics_integrity();
    test_buffer_integrity();
    return 0;
}

