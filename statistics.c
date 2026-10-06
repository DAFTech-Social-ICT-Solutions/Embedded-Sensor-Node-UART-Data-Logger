#include "statistics.h"
#include "ring_buffer.h"

#include <stdint.h>

int16_t get_max(){
    if (buffer_is_empty()){

        return 0;
    }
    int16_t max = INT16_MIN; // biggest negative number int16 can hold.
    measurement_t value;

    uint8_t count = buffer_count();
    for (int i=0; i < count; i++){
        buffer_peek_at(i, &value);
        if (max < value.temperature){
            max = value.temperature;
        }
    }
    return max;
}

int16_t get_min(){
    if (buffer_is_empty()){
        return 0;
    }
    int16_t min = INT16_MAX; // biggest positive number int16 can hold.
    measurement_t value;

    uint8_t count = buffer_count();
    for (int i=0; i < count; i++){
        buffer_peek_at(i, &value);
        if (min > value.temperature){
            min = value.temperature;
        }
    }
    return min;
}

int32_t get_average(){
    if (buffer_is_empty()){
        return 0;
    }
    int32_t sum = 0; //
    measurement_t value;

    uint8_t count = buffer_count();
    for (int i=0; i < count; i++){
        buffer_peek_at(i, &value);
        sum += value.temperature;
    }
    return sum / count;
}
