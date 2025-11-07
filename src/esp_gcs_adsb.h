#pragma once

#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include <unordered_map>

#include "esp_gcs_system.h"

#define AIRCRAFT_SIZE	6
#define VECTOR_SIZE 	20
#define AIRPORT_SIZE	3
#define AIRCRAFT_TTL 	30

class ESP_GCS_ADSB: public ESP_GCS_SYSTEM {
    
    public:
        LGFX_Sprite adsb_layer = LGFX_Sprite(&lcd);
        LGFX_Sprite aircraft = LGFX_Sprite(&lcd);

    
    public:
        ESP_GCS_ADSB();
        ESP_GCS_ADSB(esp_gcs_config_t* config);

        void render_ui_layer(LGFX_Sprite *layer);
        void render_aircraft(uint16_t color=COLOR_WHITE);

};