#include "translations.h"

// Default language (can be changed via configuration)
static language_t current_language = LANG_ENGLISH;

// Function to get current language
language_t get_current_language(void)
{
    return current_language;
}

// Function to set language
void set_language(language_t lang)
{
    current_language = lang;
}

// Function to get translation strings
const translations_t* get_translations(void)
{
    switch (current_language) {
        case LANG_SPANISH:
            return &translations_es;
        case LANG_ENGLISH:
        default:
            return &translations_en;
    }
} 