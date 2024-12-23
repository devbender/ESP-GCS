#pragma once

#define LGFX_USE_V1
#include <LovyanGFX.hpp>

#include "esp_gcs_system.h"


class ESP_GCS_PFD: public ESP_GCS_SYSTEM {
    
    private:
        LGFX_Sprite ai = LGFX_Sprite(&lcd);
        LGFX_Sprite lvl1 = LGFX_Sprite(&lcd);
        LGFX_Sprite lvl2 = LGFX_Sprite(&lcd);

        LGFX_Sprite ti = LGFX_Sprite(&lcd);
        LGFX_Sprite da = LGFX_Sprite(&lcd);

        LGFX_Sprite br = LGFX_Sprite(&lcd);
        LGFX_Sprite tbr = LGFX_Sprite(&lcd);

        int roll_ref_lines[10][4];

        uint16_t TFT_SKY = 0x02B5;
        uint16_t TFT_GND = 0x5140;

        uint16_t counter = 0;
        long startMillis = millis();
        uint16_t interval = 50;

        char txtBuff[4];
        hbmode_t prevMode;

        int indicator_rad = 80;
    
    public:
        ESP_GCS_PFD();
        void init(esp_gcs_config_t* config);
        void render(int pitch, int roll);
        void render(mavlink_heartbeat_t hb, mavlink_attitude_t atti, mavlink_vfr_hud_t hud);


    private:
        void checkButtons();
        void calculateFPS();
        void preCalcRollReferenceLines();
        void renderPFDTurnIndicator(LGFX_Sprite *sp);
        void renderPFDPitchScale(LGFX_Sprite *sp, int pitchPx);
};