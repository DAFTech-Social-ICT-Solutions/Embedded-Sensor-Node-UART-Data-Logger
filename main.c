// This is the main code run on a specific MCU.
#include "crc.h"
#include "parser.h"
#include "ring_buffer.h"
#include "statistics.h"

#include <stdint.h>
#include <stdbool.h>
#include <time.h>

bool new_data_ready = false;

void UART_ISR(uint8_t incoming_byte){
    parser_status status;

    status = parse_byte(incoming_byte);

    if (status == PARSER_PACKET_COMPLETE){
        new_data_ready = true;
    }
}

int main(){
    buffer_init();
    parser_states_reset();

    while(1){
        // Check
        if (new_data_ready){
            new_data_ready = false;
            uint16_t temp = parsed_data.data_high << 8 | (uint16_t)parsed_data.data_low;

            measurement_t measured = {
                .temperature=temp,
                .timestamp=time(NULL)
            };

            buffer_push(measured);
        }
    }
    return 0;
}
