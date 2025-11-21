#include <Arduino.h>
#include "esp_gcs_adsb.h"
#include "esp_gcs_types.h"

#include "my_esp_gcs_config.h"
#include "esp_gcs_datalink.h"

ESP_GCS_ADSB adsb;
esp_gcs_config_t config;
ESP_GCS_DATALINK datalink;

void setup(){
    delay(1000);
    
    config.display.type = PARALLEL_9488;
    config.display.width = 480;
    config.display.height = 320;

    config.link = TCP;
    config.protocol = ADSB_RAW;
    
    config.network.ssid = WIFI_SSID;
    config.network.password = WIFI_PASS;

    config.network.ip = IPAddress(10,0,0,58);
    config.network.port = 30002;   

    datalink.init( &config );
    ESP_GCS_DATALINK::set_cpr_local_reference(18.465000, -69.942800); // for local CPR, does not need to be exact

    if (!adsb.begin()) {
        while (true) {
            log_e("ADS-B INIT FAILED");
            delay(10000); // Halt
        }
    }

    // add test aircraft
    adsb.add_aircraft( 38, { 140, 100, 45,  60 } );
    adsb.add_aircraft( 39, { 140, 200, 270, 60 } );
}


void loop() {
    ESP_GCS_DATALINK::print_aircrafts();
    //log_i("running... %i", millis());
    delay(5000);
}