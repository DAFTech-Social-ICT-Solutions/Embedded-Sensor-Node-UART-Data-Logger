#include "parser.h"
#include "crc.h"

#include <stdint.h>

static parser_states current_state = WAIT_START;

static uint8_t data_read = 0;

packet_data parsed_data;

parser_status parse_byte(uint8_t byte){
    switch (current_state) {
        case WAIT_START:
            // checks the incoming byte against 0xAA
            if (byte == START_BYTE){
                current_state = READ_TYPE;
                return PARSER_OK;
            }
            return PARSER_WAITING;
        case READ_TYPE:
            if (byte == TYPE_BYTE){
                current_state = READ_LENGTH;
                parsed_data.type = byte;
                return PARSER_OK;
            }
            else{
                parser_states_reset();
                return PARSER_TYPE_ERROR;
            }

        case READ_LENGTH:
            if (byte == LEN_BYTE){
                current_state = READ_DATA;
                parsed_data.length = byte;
                return PARSER_OK;
            }
            else{
                parser_states_reset();
                return PARSER_LENGTH_ERROR;
            }

        case READ_DATA:
            // first -> data_high, second -> data_low and next state; so the next byte lands on READ_CRC cleanly.

            // checks if data_buffer has read two times
            if (data_read == 0) {
                parsed_data.data_high = byte;
                data_read = 1;
            } else {
                parsed_data.data_low = byte;
                current_state = READ_CRC;
            }

            return PARSER_OK;

        case READ_CRC:{
            // stores the data to CRC for checking validity.
            uint8_t crc_received = byte;
            uint8_t crc_payload[4] = {parsed_data.type, parsed_data.length, parsed_data.data_high, parsed_data.data_low};
            uint8_t crc_calculated = generate_crc(crc_payload);

            if (crc_received == crc_calculated){
                current_state = WAIT_END;
                return PARSER_OK;
            }
            else{
                parser_states_reset();
                return PARSER_CRC_ERROR;
            }
        }
        case WAIT_END:
            // Check if valid end byte
            if (byte == END_BYTE){
                // TODO: This immediately adds or returns the validated data  store.
                parser_states_reset();
                return PARSER_PACKET_COMPLETE;
            }
            else{
                parser_states_reset();
                return PARSER_BAD_END;
            }
        }
    return PARSER_WAITING; //Fallback
}

void parser_states_reset(){
    // set the state to 0 i.e. WAIT_START
    // and set data_read to none
    current_state = WAIT_START;
    data_read = 0;
}


