#include <stdio.h>
#include <stdint.h>

#include "parser.h"
#include "crc.h"

static parser_states current_state = WAIT_START;

static uint8_t data_read = 0;
static uint8_t crc_payload[4];

void parse_byte(uint8_t byte){
    switch (current_state) {
        case WAIT_START:
            // checks the incoming byte against 0xAA
            if (byte == START_BYTE){
                current_state = READ_TYPE;
                printf("\nPacket parsing started; waiting for type... \n");
            }
            // silently fail when byte != start_byte
            break; // prevent immediate fall through (to next case)
        case READ_TYPE:
            if (byte == TYPE_BYTE){
            current_state = READ_LENGTH;
            crc_payload[0] = byte;
            printf("Type processed: Temperature\n");

            }
            else{
                printf("Invalid type: packet rejected.\n");
                parser_states_reset();
            }
            break;

        case READ_LENGTH:
            if (byte == LEN_BYTE){
                current_state = READ_DATA;
                crc_payload[1] = byte;
                printf("Length processed: 2byte\n");
            }
            else{
                printf("Invalid length: packet rejected.\n");
                parser_states_reset();
            }
            break;

        case READ_DATA:
            // checks if data_buffer size is equal to LENGTH( == 2)
            // the data high and then read data low while simultaneously changing the state to read_crc. so the next byte lands on read_crc cleanly.

            // first -> data_high second -> data_low
            if (data_read == 1){
                crc_payload[3] = byte;
                current_state = READ_CRC;

                printf("Data read:  %u.%02u*C\n", crc_payload[2], crc_payload[3]);
                // e.g. [12, 34] -> 12.34
                // e.g. [12, 4] -> 12.04
                break;
            }
            crc_payload[2] = byte;// conserves memory instead of adding a new variable for data;
            data_read += 1;
            break;

        case READ_CRC:
            // stores the data to CRC for checking validity.
            uint8_t crc_received = byte;
            uint8_t crc_calculated = generate_crc(crc_payload);

            if (crc_received == crc_calculated){
                current_state = WAIT_END;
                printf("CRC success (received:%u expected:%u)\n", crc_received, crc_calculated);
                break;
            }
            else{
                printf("CRC error (received:%u expected:%u)\n", crc_received, crc_calculated);
                parser_states_reset();
            }
            break;

        case WAIT_END:
            // check if valid end byte
            if (byte == END_BYTE){
                printf("Packet END received\n");// TODO: This immediately adds or returns the validated data  store.
                parser_states_reset();
            }
            else{
                parser_states_reset();
                printf("Invalid end byte: packet rejected.");
            }
            break;
        // packet_complete state may not be needed
        case PACKET_COMPLETE:
          break;
        }
}

void parser_states_reset(){
    current_state = WAIT_START;
    data_read = 0;
}


