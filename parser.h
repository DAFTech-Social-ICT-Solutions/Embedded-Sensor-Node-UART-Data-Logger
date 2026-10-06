#include <stdint.h>
#include <stdbool.h>

typedef enum  {
    WAIT_START, // Check if incoming byte is a start byte (compare with 0xAA).
    READ_TYPE,
    READ_LENGTH,
    READ_DATA, // There are two bytes we will read here
    READ_CRC, // Receiving and checking the CRC for errors
    WAIT_END, // Waiting the end byte
} parser_states;

typedef enum {
    PARSER_OK,
    PARSER_CRC_ERROR,
    PARSER_TYPE_ERROR,
    PARSER_LENGTH_ERROR,
    PARSER_BAD_END,
    PARSER_PACKET_COMPLETE,
    PARSER_WAITING
} parser_status;

// These are the expected bytes from the UART for correct parsing
#define START_BYTE 0xAA //
#define TYPE_BYTE 0x01 // For temperature
#define LEN_BYTE 0x02 // For length
#define END_BYTE 0x55

typedef struct {
    uint8_t type;
    uint8_t length;
    uint8_t data_high;
    uint8_t data_low;
} packet_data;

extern packet_data parsed_data;

parser_status parse_byte(uint8_t byte);
    // The state of the parser can be implemented in this file

void parser_states_reset();
