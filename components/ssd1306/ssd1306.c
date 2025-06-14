#include "ssd1306.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "SSD1306";

#define SSD1306_I2C_ADDRESS 0x3C
#define SSD1306_WIDTH 128
#define SSD1306_HEIGHT 64
#define SSD1306_PAGES 8

static uint8_t gram[SSD1306_PAGES][SSD1306_WIDTH];

esp_err_t ssd1306_init(ssd1306_t* dev, uint8_t width, uint8_t height)
{
    dev->i2c_port = I2C_NUM_0;
    dev->address = SSD1306_I2C_ADDRESS;
    dev->width = width;
    dev->height = height;
    
    // Clear display buffer
    memset(gram, 0, sizeof(gram));
    
    // Initialize display commands
    uint8_t init_commands[] = {
        0xAE, // Display off
        0xD5, 0x80, // Set display clock
        0xA8, 0x3F, // Set multiplex ratio
        0xD3, 0x00, // Set display offset
        0x40, // Set start line
        0x8D, 0x14, // Charge pump
        0x20, 0x00, // Memory mode
        0xA1, // Segment remap
        0xC8, // COM scan direction
        0xDA, 0x12, // COM pins
        0x81, 0xCF, // Contrast
        0xD9, 0xF1, // Pre-charge
        0xDB, 0x40, // VCOM detect
        0xA4, // Display all on resume
        0xA6, // Normal display
        0xAF  // Display on
    };
    
    for (int i = 0; i < sizeof(init_commands); i++) {
        uint8_t cmd = init_commands[i];
        i2c_cmd_handle_t cmd_handle = i2c_cmd_link_create();
        i2c_master_start(cmd_handle);
        i2c_master_write_byte(cmd_handle, (dev->address << 1) | I2C_MASTER_WRITE, true);
        i2c_master_write_byte(cmd_handle, 0x00, true); // Command mode
        i2c_master_write_byte(cmd_handle, cmd, true);
        i2c_master_stop(cmd_handle);
        esp_err_t ret = i2c_master_cmd_begin(dev->i2c_port, cmd_handle, 1000 / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(cmd_handle);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to send init command 0x%02X", cmd);
            return ret;
        }
    }
    
    ESP_LOGI(TAG, "SSD1306 initialized successfully");
    return ESP_OK;
}

esp_err_t ssd1306_clear_screen(ssd1306_t* dev, bool invert)
{
    memset(gram, invert ? 0xFF : 0x00, sizeof(gram));
    return ESP_OK;
}

esp_err_t ssd1306_display_text(ssd1306_t* dev, int page, const char* text, int text_len, bool invert)
{
    if (page >= SSD1306_PAGES || text_len > 16) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // Simple text rendering (you can improve this with a proper font)
    int x = 0;
    for (int i = 0; i < text_len && x < SSD1306_WIDTH; i++) {
        char c = text[i];
        if (c >= 32 && c < 127) {
            // Simple character rendering
            for (int col = 0; col < 8 && x < SSD1306_WIDTH; col++) {
                gram[page][x++] = invert ? 0xFF : 0x01;
            }
        } else {
            // Space
            x += 8;
        }
    }
    
    return ESP_OK;
}

esp_err_t ssd1306_refresh_gram(ssd1306_t* dev)
{
    for (int page = 0; page < SSD1306_PAGES; page++) {
        // Set page address
        i2c_cmd_handle_t cmd_handle = i2c_cmd_link_create();
        i2c_master_start(cmd_handle);
        i2c_master_write_byte(cmd_handle, (dev->address << 1) | I2C_MASTER_WRITE, true);
        i2c_master_write_byte(cmd_handle, 0x00, true); // Command mode
        i2c_master_write_byte(cmd_handle, 0xB0 + page, true); // Set page
        i2c_master_write_byte(cmd_handle, 0x02, true); // Set lower column
        i2c_master_write_byte(cmd_handle, 0x10, true); // Set higher column
        i2c_master_stop(cmd_handle);
        esp_err_t ret = i2c_master_cmd_begin(dev->i2c_port, cmd_handle, 1000 / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(cmd_handle);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to set page address");
            return ret;
        }
        
        // Write page data
        cmd_handle = i2c_cmd_link_create();
        i2c_master_start(cmd_handle);
        i2c_master_write_byte(cmd_handle, (dev->address << 1) | I2C_MASTER_WRITE, true);
        i2c_master_write_byte(cmd_handle, 0x40, true); // Data mode
        i2c_master_write(cmd_handle, gram[page], SSD1306_WIDTH, true);
        i2c_master_stop(cmd_handle);
        ret = i2c_master_cmd_begin(dev->i2c_port, cmd_handle, 1000 / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(cmd_handle);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to write page data");
            return ret;
        }
    }
    
    return ESP_OK;
} 