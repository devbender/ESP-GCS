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

float angle = 0;

// Timer for printing aircraft list
uint32_t last_print_time = 0;
const uint32_t PRINT_INTERVAL_MS = 5000; // 10 seconds


void setup() {
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
    datalink.set_adsb_local_reference(18.4650, -69.9428);
    adsb.render_fb();
}
    

void loop() {
  
    uint32_t current_time_ms = millis();

    // --- 3. Non-blocking 10-second timer ---
    if (current_time_ms - last_print_time >= PRINT_INTERVAL_MS) {
        last_print_time = current_time_ms;

        //log_d("\n--- AIRCRAFT DATABASE (Updated every 10s at %lus) ---", current_time_ms / 1000);

        // Get a read-only reference to the ADS-B context
        const adsb_context& ctx = ESP_GCS_DATALINK::get_adsb_context();

        int active_aircraft = 0;
        float current_time_sec = current_time_ms / 1000.0f;

        // --- 4. Loop through the aircraft array ---
        // (ADSB_MAX_AIRCRAFT is defined in esp_gcs_adsb_decoder.h)
        for (int i = 0; i < ADSB_MAX_AIRCRAFT; ++i) {

            //log_d("%i: %i%", i, ctx.aircraft[i].icao);
            
            // Get a reference to the aircraft data
            const adsb_data& ac = ctx.aircraft[i];

            // icao == 0 means the slot is empty, so we skip it
            if (ac.icao != 0) {
            //if(ac.valid_callsign == true){
            
                // Optional: Check if the aircraft is stale before printing
                // (The decoder prunes every 100 messages, but this filters
                // aircraft that haven't been seen in 60s)
                if ((current_time_sec - ac.last_seen) > ADSB_STALE_TIMEOUT_SEC) {
                    continue;
                }

                active_aircraft++;
                
                // --- 5. Print all available data in a formatted string ---
                char buffer[256];
                snprintf(buffer, sizeof(buffer),
                    " > ICAO: %06X | CS: %-8s | ALT: %5ld ft | SPD: %3.0f kt | HDG: %3.0f | POS: %7.4f, %7.4f | SEEN: %.0fs ago",
                    ac.icao,
                    ac.valid_callsign ? ac.callsign : "----",
                    (ac.alt == INT32_MIN) ? 0 : ac.alt, // Show 0 for invalid Gillham
                    ac.valid_vel ? ac.speed : 0.0f,
                    ac.valid_vel ? ac.heading : 0.0f,
                    ac.valid_pos ? ac.lat : 0.0f,
                    ac.valid_pos ? ac.lon : 0.0f,
                    current_time_sec - ac.last_seen
                );
                // Use log_d, which is a printf-style function, so we pass the buffer as a string format
                log_i("%s", buffer);                
            }
        }
        log_i("----------------------------------------------------------------------------------------------------------------");
        
        if (active_aircraft == 0) {
            log_d("  No active aircraft found.");
        }
        
    }


}
