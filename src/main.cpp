#include <Arduino.h>
#include <esp_partition.h>

#include "my_esp_gcs_config.h"

#include "esp_gcs_pfd.h"
#include "esp_gcs_adsb.h"
#include "esp_gcs_hsi.h"

ESP_GCS_PFD pfd;
ESP_GCS_ADSB adsb;
ESP_GCS_HSI hsi;

esp_gcs_config_t config;
ESP_GCS_DATALINK datalink;

void setup() {
    config.display.type = PARALLEL_9488;
    config.display.width = 480;
    config.display.height = 320;

    config.link = TCP;
    config.protocol = SBS1;
    
    config.network.ssid = WIFI_SSID;
    config.network.password = WIFI_PASS;

    config.network.ip = IPAddress(10,0,0,58);
    config.network.port = 30003;
    
    //pfd.init( &config );
    adsb.init( &config );    
    //hsi.init( &config );
}

void loop() {   
    
    adsb.render();
    //hsi.render();
    //pfd.render_all( pfd.datalink.hb, pfd.datalink.atti, pfd.datalink.hud);
}
