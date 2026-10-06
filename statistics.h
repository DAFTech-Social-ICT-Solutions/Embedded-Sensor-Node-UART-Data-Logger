#include <stdint.h>

// To keep limited memory and operation; running calculations when needed is better;
// Otherwise MCU may  for stats updating.
int16_t get_max();
    // This returns the maximum value in the buffer memory
int16_t get_min();

int32_t get_average();


