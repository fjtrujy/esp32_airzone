idf_component_register(SRCS "ssd1306.c" "main.c" "translations.c"
                    INCLUDE_DIRS "."
                    REQUIRES driver freertos dht)