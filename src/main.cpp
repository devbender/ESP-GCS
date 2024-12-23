#include <Arduino.h>
#include <esp_partition.h>
#include "esp_gcs_system.h"
#include "esp_gcs_pfd.h"
#include "my_esp_gcs_config.h"

ESP_GCS_PFD pfd;

void setup() {    
    esp_gcs_config_t config {WIFI_SSID, WIFI_PASS, IPAddress(10,0,0,95), TCP, 3000, PARALLEL_9488, 480, 320};
    pfd.init( &config );
}

void loop() {
    pfd.render( pfd.datalink.hb, pfd.datalink.atti, pfd.datalink.hud);
}
