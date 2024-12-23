#pragma once

#define LGFX_USE_V1
#include <LovyanGFX.hpp>

#include <esp_spi_flash.h> // For flash size
#include <esp_heap_caps.h> // For PSRAM size
#include <esp_partition.h> // for partitions

#include "esp_gcs_datalink.h"
#include "esp_gcs_display_parallel16_9488.h"
#include "esp_gcs_display_spi_9342.h"



class ESP_GCS_SYSTEM {

    private:        
        int mavlink_hb_max_window = 3;

    protected:
        int fb_center_x, fb_center_y;
        uint16_t fb_width, fb_height;

        LGFX_Parallel_9488 lcd;        
        LGFX_Sprite frame_buffer = LGFX_Sprite(&lcd);

    public:
        static ESP_GCS_DATALINK datalink;        

    public:
        ESP_GCS_SYSTEM();

        void init_fb(esp_gcs_config_t* config);    
        
        void print_banner(void);
        void print_memory_info(void);
        void print_partition_info(void);       

        void run();    
};