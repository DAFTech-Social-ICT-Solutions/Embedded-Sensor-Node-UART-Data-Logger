#include "parser.h"
#include <stdio>
#include <stdint.h>

parser_states current_state = WAIT_START;
uint8_t* data_buffer[2];
uint8_t data_read = 0;

void parse_byte(uint8_t byte){
    switch (current_state) {
        case WAIT_START:
            // checks the incoming byte against 0xAA
            //
            current_state = READ_TYPE;
            printf("Packet parsing started; waiting for type... ");
            break; // changed current_state will make the next case executed immediately with the exact byte passed have now, to avoid this we use break.
        case READ_TYPE:
            current_state = READ_LENGTH;
            printf("Type: ");
            break;

        case READ_LENGTH:
            current_state = READ_DATA;
            printf("Length: ");
            break;

        case READ_DATA:
            // checks if data_buffer size is equal to LENGTH
            // assuming the sensor always sends 2 bytes  length == 2
            // now it will read the data high and then read data low while simultaneously changing the state to read_crc. so the next byte lands on read_crc cleanly.
            if (data_read == 1){
                // add the second data and switch the current_state to READ_CRC.
                data_buffer[1] = byte;
                current_state = READ_CRC;
                printf("Data read: ##.## ");
                break;
            }
            data_buffer[0] = byte;
            data_read += 1;
            break;

        case READ_CRC:
            // stores the data to CRC for checking validity later
            current_state = WAIT_END;
            break;
        case WAIT_END:
            // checks if end is a valid end byte else adds the error increment.
            parser_states_reset();
            break;
}
}

void parser_states_reset(){
    current_state = WAIT_START;
    data_buffer = 0;
    data_read = 0;
}


