#include <Arduino.h>
#include <esp_partition.h>
#include "esp_gcs_system.h"
#include "esp_gcs_pfd.h"

ESP_GCS_PFD pfd;

void setup() {
    esp_gcs_config_t config {"SSID", "PASSWORD", IPAddress(10,0,0,100), TCP, 3000, display_driver, width, height};
    pfd.init( &config );
}

void loop() {

    //pfd.run();
    //pfd.render( pfd.datalink.hb, pfd.datalink.atti, pfd.datalink.hud);
}
