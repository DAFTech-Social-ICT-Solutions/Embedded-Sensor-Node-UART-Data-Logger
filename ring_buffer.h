// Store validated data
#include <stdint.h>
#include <stdbool.h>

#define BUFFER_SIZE 16

typedef struct {
    int16_t temperature;
    uint32_t timestamp;
} measurement_t;


void buffer_init(void);
bool buffer_push(measurement_t measurement);
bool buffer_pop(measurement_t *measurement);
bool buffer_is_empty(void);
bool buffer_is_full(void);
uint8_t buffer_count(void);
