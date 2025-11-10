#pragma once

#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include <FT6236G.h>

#include "esp_gcs_colors.h"
#include "my_esp_gcs_config.h"
#include "esp_gcs_datalink.h"
#include "esp_gcs_display_parallel16_9488.h"
#include "esp_gcs_display_spi_9342.h"


class ESP_GCS_SYSTEM {

    private:        
        int mavlink_hb_max_window = 3;

        esp_gcs_touch_t touch = FT6236;
        esp_gcs_display_t display = PARALLEL_9488;
    
    protected:
        static FT6236G tc;
        static TOUCHINFO ti;
        static touch_point_t tpoint, tpoint_o;

        static const uint8_t ft6236g_i2c_sda = 38;
        static const uint8_t ft6236g_i2c_scl = 39;

        static SemaphoreHandle_t mutex;
        static QueueHandle_t queue_events;


    public:
        LGFX_Parallel_9488 lcd;
        int fb_center_x, fb_center_y;
        uint16_t fb_width, fb_height;
        
        LGFX_Sprite fb_0 = LGFX_Sprite(&lcd);
        LGFX_Sprite fb_1 = LGFX_Sprite(&lcd);

        TaskHandle_t xHandleEventsTask = NULL;

    public:
        
        static ESP_GCS_DATALINK datalink;        

    public:
        ESP_GCS_SYSTEM();

        void init(void);
        void init_fb(esp_gcs_config_t* config);
        void cfg_sp(LGFX_Sprite*, int, int);
        
        void print_banner(void);
        void print_memory_info(void);
        void print_partition_info(void);

        void set_palette_4bit(LGFX_Sprite *sp);

        void run();

        void push_fb();

        static void task_touch_events(void *pvParameters);
        static void task_system_events(void *pvParameters);

        
};