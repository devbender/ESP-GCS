#pragma once

#define LGFX_USE_V1
#include <LovyanGFX.hpp>

#include "esp_gcs_system.h"
#include "esp_gcs_filter.h"

class ESP_GCS_PFD: public ESP_GCS_SYSTEM {
    
    private:
        LGFX_Sprite ai = LGFX_Sprite(&lcd);
        LGFX_Sprite lvl1 = LGFX_Sprite(&lcd);
        LGFX_Sprite lvl2 = LGFX_Sprite(&lcd);

        LGFX_Sprite ti = LGFX_Sprite(&lcd);
        LGFX_Sprite da = LGFX_Sprite(&lcd);

        LGFX_Sprite br = LGFX_Sprite(&lcd);
        LGFX_Sprite tbr = LGFX_Sprite(&lcd);

        LGFX_Sprite spd = LGFX_Sprite(&lcd);

        int roll_ref_lines[10][4];

        uint16_t TFT_SKY = 0x02B5;
        uint16_t TFT_GND = 0x5140;

        uint16_t counter = 0;
        long startMillis = millis();
        uint16_t interval = 50;

        char txtBuff[4];
        hbmode_t prevMode;

        int indicator_rad = 90;
        int level_offset = 60;

        float aircraft_level_size = 1.2;
        float aircraft_indicator_size = 1.1;
        float framebuffer_zoom = 1.2;
        float side_indicators_size = 1;

        float ai_x, ai_y, lvl1_x, lvl1_y, lvl2_x, lvl2_y, ti_x, ti_y;

        esp_gcs_filter_t pitch_filter, roll_filter;
    
    public:
        ESP_GCS_PFD();


        void init(esp_gcs_config_t* config);
        void init2(esp_gcs_config_t* config);
        
        void render(float pitch, float roll);
        void render(int pitch, int roll, bool t);
        void render2(mavlink_heartbeat_t hb, mavlink_attitude_t atti, mavlink_vfr_hud_t hud);

        void render_all(mavlink_heartbeat_t hb, mavlink_attitude_t atti, mavlink_vfr_hud_t hud);


    private:

        void init_base_layer(void);
        void init_top_layer(void);

        void render_base_layer(mavlink_attitude_t atti);
        void render_top_layer(mavlink_attitude_t atti);

        void checkButtons();
        void calculateFPS();
        void precalculate_roll_ref_lines();
        
        void render_turn_indicator(LGFX_Sprite *sp);
        void render_pitch_scale(LGFX_Sprite *sp, int pitchPx);
        void render_aircraft(LGFX_Sprite *sp, int inRoll);
        void render_side_indicators(LGFX_Sprite *sp, int inRoll, int alt, int spd);
};