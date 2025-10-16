#pragma once

#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include <unordered_map>

#include "esp_gcs_system.h"

class ESP_GCS_ADSB: public ESP_GCS_SYSTEM {
    
    private:
        LGFX_Sprite adsb = LGFX_Sprite(&lcd);

        uint16_t TFT_SKY = 0x02B5;
        uint16_t TFT_GND = 0x5140;

        char txtBuff[4];
    
    public:
        ESP_GCS_ADSB();
        ESP_GCS_ADSB(esp_gcs_config_t* config);

        void init(esp_gcs_config_t* config);
        void init_base_layer(void);
        void init_top_layer(void);

        void render();
        void render_top_layer();
};