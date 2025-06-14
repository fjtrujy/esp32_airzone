#ifndef SSD1306_H
#define SSD1306_H

#include "driver/i2c.h"
#include "esp_err.h"

typedef struct {
    i2c_port_t i2c_port;
    uint8_t address;
    uint8_t width;
    uint8_t height;
} ssd1306_t;

esp_err_t ssd1306_init(ssd1306_t* dev, uint8_t width, uint8_t height);
esp_err_t ssd1306_clear_screen(ssd1306_t* dev, bool invert);
esp_err_t ssd1306_display_text(ssd1306_t* dev, int page, const char* text, int text_len, bool invert);
esp_err_t ssd1306_refresh_gram(ssd1306_t* dev);

#endif // SSD1306_H 