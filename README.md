# Design Approach

For systems with constraints on memory and processing power i.e. embedded systems; low latency, power efficiency and reliability are key considerations. To achieve a considerable low latent and power efficient data logging system adhering to some key design approaches is necessary:
### Separation of concerns:
Each components strictly do only what they are meant to do. i.e. parser component only parses and validates but doesn't need to save to buffer. Thus each component does best of its ability on what it concerns only.

### Quantization of floats:
As per the cost of processing in embedded systems; integer calculations are favoured over floating point calculations. Using representations of float as in , for Example: 12.03*C -> 123 and  -> 12.34*C -> 1234 is the calculations approach used on parsing, storing and statistics calculations. Still displaying floating point format at the final will suffice. 
CRITICAL: Other systems cascaded or added to this system should take this into consideration!

### Finite state machine
Usage of explicit states for readable code and memory efficiency makes the parser less prone to bugs and unexpected behaviours. The parser processes incoming bytes based on the 7 states. 

### Bottom up approach
The development begins by developing low-level foundational parts before combining them. Core utilities—such as the 8-bit CRC generator using 0x07 polynomial, state based parser, and array-backed ring buffer operations—were fully implemented and unit-tested in isolation. After these individual components were proven stable were they linked together to form the complete telemetry parsing, storage, and statistics processing pipeline.

### Test driven design : 
Cumulative rigorous tests for each components of system as well as their integration before advancing. Generated CRC bytes can be tested against reliable CRC-gen tools found online, components like parser can be tested on generated mixture of various packet both valid and invalid. 

### Real applications : 
The code assumes a real UART data logging system. This can be used on a device that takes the output of an UART receiver which gives data (a byte at a time) in parallel. 

# State machine
Using a state machine for the parser is crucial to track which type of byte is expected. So successful (expected) bytes will advance the parser to the next state. In the code 7 states are used as follows: 

    WAIT_START, // 
    READ_TYPE,
    READ_LENGTH,
    READ_DATA, // there are two bytes we will read here
    READ_CRC, // Receiving and checking the CRC for errors
    WAIT_END, // waiting the end byte
    PACKET_COMPLETE

For the purpose of error logging the parser will notify of what its status is using enum returns:

    PARSER_OK,
    PARSER_CRC_ERROR,
    PARSER_TYPE_ERROR,
    PARSER_LENGTH_ERROR,
    PARSER_BAD_END,
    PARSER_PACKET_COMPLETE,
    PARSER_WAITING
    
<img width="453" height="507" alt="datalogger_state_diagram" src="https://github.com/user-attachments/assets/3dca9a2c-caf6-47da-801e-8a2cf298bc30" />

### WAIT START 
This is the default state the parser initializes; It checks if any incoming byte is the start protocol i.e. 0xAA. If so it advances the parser to the next state. The parser will emit PARSER_OK status whenever incoming bytes are as expected. Otherwise PARSER_WAITING is emitted if it expected a start byte but got another.

### READ TYPE and READ LENGTH
These states check for valid type and length indicating bytes and advance the parser to the next states respectively emitting a PARSER_OK. It saves both bytes into the parsed_data variable. If things go wrong, this state emits PARSER_TYPE_ERROR or PARSER_LENGTH_ERROR errors for unexpected indicator bytes and thus resetting the state to WAIT_START immediately.

### READ DATA
This state accepts any byte, as data byte can be anything, if not explicitly ranged. This state needs to read two byte so to achieve it the state needs to stay for two bytes unlike previous states. A variable like bytes_read can be used to decide and advance based on whether it has read all these two bytes or not. The data is saved in parsed_data as data_high and data_low. 

### READ CRC
This stage of parser will take on the received CRC and compares it to parser generated CRC using the bytes it has previously received. If equal, it advances; if not it immediately throws PARSER_CRC_ERROR and rejects the packet.

### WAIT END
This stage waits for end byte that is 0x55. If it gets; it emits PARSER_PACKET_COMPLETE to notify the caller it can save the data to buffer. Otherwise the packet is completely rejected!

# Circular Buffer
Data needs to be put for further processing. Sensor systems usually send data upto thousands of times per second (kHz). Putting such data for processing is essential. To account for the system constraints in embedded systems (memory), it is imperative that it cannot put the data indefinitely. So to solve such problems circular buffers are used.

Circular buffers or ring buffers are buffer systems that use only a specific block of memory and wraps around to write data. It keeps on two primary variables to track the buffer system: head and tail. Head of a circular buffer is where data will be written at the time it was called, and tail is where data will be read at the time of calling. The head and tail advance to the next address whenever push and pop are called. The head advances until the end of the block is reached, in which case it wraps around to continue to its tail until eventually the buffer_capacity is reached (i.e. maximum buffer memory used). Thus the write and read process achieves FIFO first in first out queue behavior. 

# CRC Implementation

## Theory
The CRC error checking of a polynomial 8 is implemented as follows:

It starts at 0x00 then it is XORed with the first byte. 
 
Taking an example on 4 bytes:
0000 0000 is the starting base, then after XORed with 0000 0001 (type temperature)
0000 0001 is the result, this is shifted to the left bit by bit and whenever a bit 1 falls off the edge, XOR the result with the polynomial.
Which results in
0000 0000 this will be XORed with the polynomial byte that is 0x07 0000 0111

0000 0111 is the result, this is XORed with the next byte that's 0x02 
0000 0101 will result, then this again is shifted to the left with the same rules.

The implementation boils down to this:   
## Code implementation
### First load with init state

**The outer loop (run 4 times):**

* XOR with the byte

**The inner loop (run 8 times):**

* do shift left by 1
* if a bit 1 fell of the edge, XOR with the polynomial immediately.

```c
uint8_t generate_crc(uint8_t *payload){
    uint8_t crc_result = 0x00; // initialized 0000 0000
    for (int byte=0; byte<4; byte++){ 
        crc_result ^= payload[byte]; 

        // do left shift while checking 
        for (int bit=0; bit<8; bit++){
            // check if the Left most bit is 1;
            // results in 0x80 = 1000 0000
            // in C any non-zero value is considered true
            if (crc_result & 0x80){
                crc_result = crc_result << 1;
                crc_result ^= 0x07;// crc-8 polynomial
            }
            else{
                crc_result = crc_result<<1;
            }
        }
    }
    return crc_result;

}
```

# Testing performed
**CRC: ** This is tested against reliable online CRC calculator [Sunshine CRC Calculator]https://sunshine2k.de/coding/javascript/crc/crc_js.html by asserting a couple of the locally/code generated crc with results found from the site. 

**Parser: ** This tested by a load of 350 stream of bytes (50 packets) of various cases generated with python scripts:
- Valid 
- Type invalid
- Length invalid
- CRC Error
- Bad ends
- Incomplete packets
- Noise stream (random)

=================== SUMMARY REPORT ===================
 Valid Packets Parsed:  42
 Stored Packets: 16
 Total Errors Detected: 32
---------------------------------------------------------
  - CRC Errors:             3
  - Type Errors:            1
  - Length Errors:          1
  - Framing/Bad End Errors: 1
  - Buffer Overflow Errors: 26
===========================================================


**Ring buffer: ** Valid parsed packets were saved to the ring buffer sequentially until the buffer reached its capacity (16).
These buffer data were tested with push and pop commands. 
-   Carefully verifying their queue behavior is FIFO instead of LIFO. 
-   Out of bound cases like poping an empty buffer and pushing on a full buffer. 

**Statistics: **This function does operations over the buffer to calculate the max, min and average of the entire buffer values. 
The results returned from the statistics were asserted against calculated values.

# Assumptions and Limitations
## Assumptions
- Input to the MCU is delivered as byte sequence.
- Temperature input is communicated as a **signed** 8 bit, thus -327.68*C to 327,67*C for 0xFF,FF.
- The Sensor system outputs a reliable CRC-8 polynomial correctly.

## Limitations
If a packet is incomplete it can miss one subsequently incoming valid packet.
e.g. 0xAA 0x01 0x02 , then it halts and sends 0xAA 0x01 0x02 ...(complete and valid packet).
Explanation: The parser expects the next bytes but finds a CRC error or some bad end error but what it was getting is a start of valid packet. So the valid packet is lost. 

Solving this is entirely possible, but it requires either branched parsing (memory and CPU inefficient) or limiting all byte ranges to assume 0xAA and 0xEE are always a start byte and end byte despite the state of the parser. 

# Performance metrics:
For embedded systems low latency, memory and power usage are key considerations...

Giving crude analysis on the performance metrics we can consider the code simulation takedowns:
**Memory**
The memory usage can be classified as code memory (where the instructions sit) and runtime memory (all data processed);
*code or flash memory (instructions)*: 
    PARSER:  
    
    CRC:
    
    RING_BUFFER:
*runtime memory (variables)*: 
    
    PARSER function:
        parser_state: enum of 7 elements 1 byte
        parser_status: enum of 7 elements 1 byte
        packet_data / parsed_data: struct of 4 uint8_t elements (4bytes)
        current_state: 1 byte
            
    CRC Function:
        crc_result: uint8_t type takes one byte memory
        payload: array of 4 uint8_t <= packet_data zero-copy (memory save!) no byte take!
        
    RING BUFFER function:
        measurement_t: struct of int16_t and uint32_t 2 and 4 bytes => 6bytes
        head, tail and count: uint8_t => 3bytes
        data: 16 x measurement_t = 6*16 = 96
    Memory usage: roughly 109 bytes!
