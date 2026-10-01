// unit tests
#include "../crc.h"
#include <stdint.h>
#include <stdio.h>

void test_crc_integrity(){

    uint8_t payload_example[4] = {1, 2, 55, 32};
    uint8_t result = generate_crc(payload_example);
    printf("integer: [1 2 55 32]\nin hex:[0x01 0x02 0x37 0x20] -> CRC: 0x%02x\n",result);
}

int main(){
    test_crc_integrity();
    return 0;
}
