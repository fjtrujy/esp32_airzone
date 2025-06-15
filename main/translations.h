#pragma once

#include <stdint.h>

// Language selection
typedef enum {
    LANG_ENGLISH = 0,
    LANG_SPANISH = 1
} language_t;

// Translation strings structure
typedef struct {
    const char* temperature;
    const char* humidity;
    const char* status_ok;
    const char* status_error;
    const char* sensor_error;
    const char* check_wiring;
    const char* esp32_airzone;
    const char* thermostat;
    const char* starting;
} translations_t;

// English translations
static const translations_t translations_en = {
    .temperature = "Temperature:",
    .humidity = "Humidity:",
    .status_ok = "Status: OK",
    .status_error = "Status: ERROR",
    .sensor_error = "Sensor Error:",
    .check_wiring = "Check wiring",
    .esp32_airzone = "ESP32 Airzone",
    .thermostat = "Thermostat",
    .starting = "Starting..."
};

// Spanish translations
static const translations_t translations_es = {
    .temperature = "Temperatura:",
    .humidity = "Humedad:",
    .status_ok = "Estado: OK",
    .status_error = "Estado: ERROR",
    .sensor_error = "Error Sensor:",
    .check_wiring = "Verificar cableado",
    .esp32_airzone = "ESP32 Airzone",
    .thermostat = "Termostato",
    .starting = "Iniciando..."
};

// Function to get current language (default to English)
language_t get_current_language(void);

// Function to set language
void set_language(language_t lang);

// Function to get translation strings
const translations_t* get_translations(void); 