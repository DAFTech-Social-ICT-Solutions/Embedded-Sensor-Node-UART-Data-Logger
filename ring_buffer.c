#include <stdint.h>
#include "ring_buffer.h"
#include <stdio.h>
static uint8_t head = 0;
static uint8_t tail = 0;
static uint8_t count = 0;

static measurement_t data[BUFFER_SIZE];

void buffer_init(){
    head = 0;
    tail = 0;
    count = 0;
}

bool buffer_push(measurement_t measurement){
    if (count < BUFFER_SIZE){
    data[head] = measurement;
    head = (head + 1) % BUFFER_SIZE;
    count++;
    return true;
    }
    else{
        return false;
    }
}

bool buffer_pop(measurement_t *measurement){
    //*measurement is reference to measurement
    if (count > 0){
        *measurement = data[tail]; //result will be put to this pointer
        tail = (tail + 1) % BUFFER_SIZE;
        count--;
        return true;
    }
    else{
        return false;
    }
}

bool buffer_is_empty(){
    return count == 0;
}

bool buffer_is_full(){
    return count == BUFFER_SIZE;
}

uint8_t buffer_count(){
    return count;
}

bool buffer_peek_at(uint8_t index, measurement_t *out_measurement){
    // a pop function without affecting the ring_buffer
    // at tail + index
    if (index < count){
        index = (tail + index)%BUFFER_SIZE;
        *out_measurement = data[index];
        return true;
    }
    else{
        return false;
    }

}
