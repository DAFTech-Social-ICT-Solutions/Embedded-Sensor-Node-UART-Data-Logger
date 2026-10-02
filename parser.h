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

typedef enum {
    PARSER_OK,
    PARSER_CRC_ERROR,
    PARSER_TYPE_ERROR,
    PARSER_LENGTH_ERROR,
    PARSER_BAD_END,
    PARSER_BUFFER_OVERFLOW_ERROR,
    PARSER_PACKET_COMPLETE,
    PARSER_WAITING

} parser_status;

// these are the expected bytes from the UART for correct parsing
#define START_BYTE 0xAA //
#define TYPE_BYTE 0x01 // for temperature
#define LEN_BYTE 0x02 //
#define END_BYTE 0x55


parser_status parse_byte(uint8_t byte);
    // The state of the parser can be implemented in this file

void parser_states_reset();
