idf_component_register(SRCS "src/main.cpp"
                       PRIV_INCLUDE_DIRS "include"
                       PRIV_REQUIRES storage camera mbedtls nvs_flash esp_http_server esp_wifi mqtt esp_psram)
