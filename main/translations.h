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
    const char* set_temp;
    const char* current_temp;
    const char* mode_cool;
    const char* mode_heat;
    const char* mode_off;
    const char* control_active;
    const char* control_inactive;
    const char* mode_label;
    const char* status_label;
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
    .starting = "Starting...",
    .set_temp = "Set Temperature:",
    .current_temp = "Current Temperature:",
    .mode_cool = "Cool",
    .mode_heat = "Heat",
    .mode_off = "Off",
    .control_active = "Control Active",
    .control_inactive = "Control Inactive",
    .mode_label = "Mode:",
    .status_label = "Status:"
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
    .starting = "Iniciando...",
    .set_temp = "Temperatura Configurada:",
    .current_temp = "Temperatura Actual:",
    .mode_cool = "Frío",
    .mode_heat = "Calor",
    .mode_off = "Apagado",
    .control_active = "Control Activo",
    .control_inactive = "Control Inactivo",
    .mode_label = "Modo:",
    .status_label = "Estado:"
};

// Function to get current language (default to English)
language_t get_current_language(void);

// Function to set language
void set_language(language_t lang);

// Function to get translation strings
const translations_t* get_translations(void); 