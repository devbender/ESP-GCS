#include <Arduino.h>
#include "esp_gcs_adsb.h"

ESP_GCS_ADSB adsb;

void setup() {
    delay(1500);
    log_i("Setup complete");

    if (!adsb.begin()) {
        log_e("ADS-B Display failed to initialize!");
        while (true) {
            delay(1000); // Halt
        }
    }

    adsb.add_aircraft( 38, { 140, 100, 45,  60 } );
    adsb.add_aircraft( 39, { 140, 200, 270, 60 } );
}


void loop() {
    // Nothing to do here
}