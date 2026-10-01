#include <stdint.h>
#include <stdbool.h>

typedef enum  {
    WAIT_START, // check if incoming byte is a start byte (compare with 0xAA).
    READ_TYPE,
    READ_LENGTH,
    READ_DATA, // there are two 8bytes we will read here
    READ_CRC, // Receiving and checking the CRC for errors
    WAIT_END, // waiting the end byte
    PACKET_COMPLETE,
} parser_states;

// these are the expected bytes from the UART for correct parsing
#define START_BYTE 0xAA //
#define TYPE_BYTE 0x01 // for temperature
#define LEN_BYTE 0x02 //
#define END_BYTE 0x55

// Error statistics
// uint8_t valid_packets;
// uint8_t invalif_packets;
// uint8_t crc_errors;
// uint8_t invalid_type_errors;
// uint8_t invalid_length_errors;
// uint8_t invalid_end_errors;
// uint8_t buffer_overflow_error;

void parse_byte(uint8_t byte);
    // The state of the parser can be implemented in this file

void parser_states_reset();
    // this will set the state to 0 i.e. wait_start
    // and removes variable
