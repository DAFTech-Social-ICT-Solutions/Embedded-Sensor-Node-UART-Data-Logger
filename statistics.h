#include <stdint.h>

// to keep limited memory and operation; running calculations when needed is better;
// otherwise MCU may overhead for stats updating.
int16_t get_max();
    // this returns the maximum value in the buffer memory
int16_t get_min();

int32_t get_average();


