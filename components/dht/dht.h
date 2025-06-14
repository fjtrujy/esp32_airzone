#ifndef DHT_H
#define DHT_H

#include "driver/gpio.h"
#include "esp_err.h"

typedef enum {
    DHT_TYPE_DHT11 = 0,
    DHT_TYPE_DHT22 = 1,
} dht_type_t;

typedef struct {
    gpio_num_t pin;
    dht_type_t type;
} dht_t;

esp_err_t dht_read_float_data(dht_type_t type, gpio_num_t pin, float* humidity, float* temperature);

#endif // DHT_H 