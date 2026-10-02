// unit tests
#include "../crc.h"
#include "../parser.h"
#include "../ring_buffer.h"

#include <stdint.h>
#include <stdio.h>
#include <time.h>

void test_crc_integrity(){
    printf("\n============ Test CRC Integrity ==================\n");

    uint8_t payload_example[4] = {1, 2, 55, 32};
    uint8_t result = generate_crc(payload_example);

    printf("\ninteger: [1 2 55 32]\nin hex:[0x01 0x02 0x37 0x20] -> CRC: 0x%02x\n",result);
}
uint8_t valid_packets = 0;
uint8_t crc_error = 0;
uint8_t length_error = 0;
uint8_t type_error = 0;
uint8_t bad_end_error = 0;


void test_parser_integrity(uint8_t stream[]){
    // length is how many elements we have in the packet
    printf("\n============ Packet Test ================\n");

    for (int byte = 0; byte < 56; byte++){
        printf("\n============ Byte %d = %02x ================\n", byte, stream[byte]);
        switch(parse_byte(stream[byte])){
            case PARSER_WAITING:
                printf("\nWaiting for start byte...");
                break;
            case PARSER_OK:
                printf("\nPARSER_OK");
                break;
            case PARSER_CRC_ERROR:
                printf("\nCRC Error");
                crc_error++;
                break;
            case PARSER_TYPE_ERROR:
                 printf("\nType Error");
                 type_error++;
                break;
            case PARSER_LENGTH_ERROR:
                 printf("\nLength Error");
                 length_error++;
                break;
            case PARSER_BAD_END:
                 printf("\nBad End Error");
                 bad_end_error++;
                break;
            case PARSER_BUFFER_OVERFLOW_ERROR:
                 printf("\nBuffer overflow Error");
                break;
            case PARSER_PACKET_COMPLETE:
                printf("\nPacket accepted: %i", byte);
                valid_packets++;
                int16_t temp = (parsed_data.data_high << 8 )| parsed_data.data_low;
                measurement_t measured = {.temperature=temp, .timestamp=time(NULL)};
                if (buffer_push(measured)){
                    printf("\nStored!");
                }
                break;
        }
    }
    uint16_t total_errors = crc_error + type_error + length_error +
    bad_end_error;

    printf("\n=================== SUMMARY REPORT ===================\n");
    printf(" Valid Packets Parsed:  %u\n", valid_packets);
    printf(" Total Errors Detected: %u\n", total_errors);
    printf("---------------------------------------------------------\n");
    printf("  - CRC Errors:             %u\n", crc_error);
    printf("  - Type Errors:            %u\n", type_error);
    printf("  - Length Errors:          %u\n", length_error);
    printf("  - Framing/Bad End Errors: %u\n", bad_end_error);
    printf("===========================================================\n\n");
}

int main(){
    test_crc_integrity();

    uint8_t packet_stream[] = {
        0xAA, 0x01, 0x02, 0x0c, 0x22, 0xD2, 0x55, // valid packet crc = 0xD2 precalculated
        0xEE, 0x01, 0x02, 0x0c, 0x22, 0xD2, 0x55, // bad start byte packet
        0xAA, 0xEE, 0x02, 0x0c, 0x22, 0xD2, 0x55, // type invalid packet
        0xAA, 0x01, 0xEE, 0x0c, 0x22, 0xD2, 0x55, // length invalid packet
        0xAA, 0x01, 0x02, 0x0c, 0x22, 0xEE, 0x55, // crc invalid packet
        0xAA, 0x01, 0x02, 0x0c, 0x22, 0xD2, 0xEE, //end invalid packet

        0xAA, 0xEE, 0x02, 0x0c, 0x22, 0xD2, 0x55, // type invalid packet
        0xAA, 0x01, 0xEE, 0x0c, 0x22, 0xD2, 0x55, // length invalid packet
    };
    test_parser_integrity(packet_stream);
    return 0;
}

