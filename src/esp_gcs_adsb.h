#pragma once

#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include <unordered_map>
#include <mutex>

#include "esp_gcs_hal_display_config.h"
#include "esp_gcs_hal_display_device.h"
#include "esp_gcs_hal_framebuffer_manager.h"
#include "esp_gcs_hal_render_task.h"

#define AIRCRAFT_SIZE	6
#define VECTOR_SIZE 	20
#define AIRPORT_SIZE	3
#define AIRCRAFT_TTL 	60


struct aircraft_data_t {
    uint16_t x, y;
    int16_t heading;
    uint8_t ttl; // time to live in frames
};

class ESP_GCS_ADSB {

    public:
        ESP_GCS_ADSB();
        ~ESP_GCS_ADSB();

        bool begin();        

        void add_aircraft(uint32_t icao, aircraft_data_t aircraft);
        
        void render_ui_layer(LGFX_Sprite *layer);
        void draw_aircraft_sprite(LGFX_Sprite* sprite, uint16_t color);

    private:
        DisplayConfig config;
        DisplayDevice device;
        FrameBufferManager fbManager;
        RenderTask renderer;

        std::mutex aircraft_list_mutex;
        std::unordered_map<uint32_t, aircraft_data_t> aircraft_list;

        const uint8_t sprite_size = 2 * AIRCRAFT_SIZE + 4; // -> e.g., 16+4 = 20
        
        LGFX_Sprite* aircraft_sprite = nullptr;
        LGFX_Sprite* ownship_sprite = nullptr;

        int rotation=0;
        
        bool init_sprite(LGFX_Sprite*& sprite, uint16_t aircraft_color);
        static void draw_loop(LGFX_Sprite& sprite, void* context);        
};