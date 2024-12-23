#include "esp_gcs_system.h"

ESP_GCS_SYSTEM::ESP_GCS_SYSTEM() { }



void ESP_GCS_SYSTEM::print_memory_info() {
    
    // Get total and available flash size in MB
    uint32_t flash_size = spi_flash_get_chip_size() / (1024 * 1024);
    log_d("Total Flash Size: %d MB", flash_size);

    // PSRAM info (ESP32 with PSRAM enabled)
    size_t psram_free = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    size_t psram_total = heap_caps_get_total_size(MALLOC_CAP_SPIRAM);

    log_d("Total PSRAM Size: %.2f MB", psram_total / (1024.0 * 1024));
    log_d("Free PSRAM: %.2f MB", psram_free / (1024.0 * 1024));    
}



void ESP_GCS_SYSTEM::print_banner() {
    delay(2000);
    log_d("#####################################################");
    log_d("#                    ESP-GCS                        #");
    log_d("#####################################################");
}



void ESP_GCS_SYSTEM::print_partition_info() {
    log_d("ESP32 Partition Information:");
    log_d("------------------------------------------------------");
    log_d("%-10s %-8s %-10s %-10s %-10s", "Name", "Type", "SubType", "Offset", "Size");

    const esp_partition_t* part = NULL;
    esp_partition_iterator_t it = esp_partition_find(ESP_PARTITION_TYPE_ANY, ESP_PARTITION_SUBTYPE_ANY, NULL);

    while (it != NULL) {
        part = esp_partition_get(it);
        log_d("%-10s 0x%-6x 0x%-8x 0x%-8x %u KB",
              part->label,
              part->type,
              part->subtype,
              part->address,
              part->size / 1024); // Size in KB
        it = esp_partition_next(it);
    }

    esp_partition_iterator_release(it);

    log_d("------------------------------------------------------");
    log_d("Done!");
}



void ESP_GCS_SYSTEM::init_fb(esp_gcs_config_t* config) {


    if(config->display == PARALLEL_9488) {
        // Pin init
        pinMode(LCD_CS, OUTPUT);
        pinMode(LCD_BLK, OUTPUT);

        digitalWrite(LCD_CS, LOW);
        digitalWrite(LCD_BLK, HIGH);
    }

    else if(config->display == SPI_9342) { }
    else {}

    lcd.init();
    lcd.setRotation(1);        
    lcd.fillScreen(TFT_BLACK);
    lcd.setPivot( lcd.width()/2, lcd.height()/2 );

    fb_width = config->fb_width;
    fb_height = config->fb_height;

    frame_buffer.setColorDepth(8);
    frame_buffer.createSprite(config->fb_width, config->fb_height);
    frame_buffer.fillSprite(TFT_TRANSPARENT);

    frame_buffer.setTextSize(1);
    frame_buffer.setTextColor(TFT_WHITE);
    frame_buffer.drawCenterString("FB init OK!", frame_buffer.width()/2, frame_buffer.height()/2);

    fb_center_x = ( lcd.width() - frame_buffer.width() )/2;
    fb_center_y = ( lcd.height() - frame_buffer.height() )/2;

    frame_buffer.pushSprite(fb_center_x, fb_center_y);

    print_banner();
    print_memory_info();
    print_partition_info();


    datalink.init(config);
}



void ESP_GCS_SYSTEM::run() {

    uint32_t last_hb_received_time = (millis()/1000 - datalink.time_since_last_hb);

    if( last_hb_received_time > mavlink_hb_max_window ) {
        log_e("No mavlink heartbeat in last %i seconds", last_hb_received_time);
        datalink.tcp.close();
        delay(1000);
    }

    frame_buffer.fillSprite(TFT_BLUE);

    char buff[32];
    snprintf(buff, sizeof(buff), "%.2f     %.2f     %.2f", degrees(datalink.atti.pitch), degrees(datalink.atti.roll), degrees(datalink.atti.yaw));
    frame_buffer.drawCenterString(buff, frame_buffer.width()/2, frame_buffer.height()/2 - 8);
    
    frame_buffer.pushSprite(fb_center_x, fb_center_y);
}