#include <stdint.h>
#include <stdbool.h>

typedef enum  {
    WAIT_START, // the parser checks if the incoming is a start byte i.e. comparing it to 0xAA, if yes it goes to next state.
    READ_TYPE,
    READ_LENGTH,
    READ_DATA, // there are two 8bytes we will read here
    READ_CRC,
    WAIT_END, // waiting the end byte
    PACKET_COMPLETE, // we can check validity here
} parser_states;

// these are the expected bytes from the UART for correct parsing
#define START_BYTE 0xAA //
#define TYPE_BYTE 0x01 // for temperature
#define LEN_BYTE 0x02 //
#define END_BYTE 0x55

void parse_byte(uint8_t byte){
    // The state of the parser can be implemented in this file
}
void parser_states_reset(void){
    // this will set the state to 0 i.e. wait_start
    // and removes variables
}
