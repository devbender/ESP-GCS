#include <Arduino.h>
#include <esp_partition.h>

#include "my_esp_gcs_config.h"

#include "esp_gcs_pfd.h"
#include "esp_gcs_adsb.h"
#include "esp_gcs_hsi.h"

#include <FT6236G.h>

ESP_GCS_PFD pfd;
ESP_GCS_ADSB adsb;
ESP_GCS_HSI hsi;

esp_gcs_config_t config;
//ESP_GCS_DATALINK datalink;

// tasks definitions
static void task_events(void *pvParameters);
static void task_print_info(void *pvParameters);

// #############################################################################################################
// SETUP
// #############################################################################################################
void setup() {
    Serial.begin(115200);
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

    //datalink.init( &config );
    //datalink.set_adsb_local_reference(18.4650, -69.9428); // for local CPR
    adsb.push_fb();
    

    
    xTaskCreate(
        adsb.task_system_events,
        "system_events_task",
        1024*4,
        NULL,
        4,
        NULL
    );



    xTaskCreate(
        adsb.task_touch_events,
        "touch_events_task",
        1024*2,
        NULL,
        5,
        &adsb.xHandleEventsTask
    );


    log_i("init datalink...");
    adsb.datalink.set_cpr_local_reference(18.4650, -69.9428); // for local CPR
    adsb.datalink.init( &config );
    
}
    


// #############################################################################################################
// LOOP
// #############################################################################################################
void loop() {




}




// #############################################################################################################
// EVENT TASK
// #############################################################################################################
static void task_events(void *pvParameters) {
    log_d("EVENT TASK STARTED");
    
    int angle = 0;

    for(;;) {
        log_d("EVENT TASK RUNNING");        

        adsb.fb_1.pushRotateZoom(&adsb.fb_0, 0, 1.0,1.0);
        angle += 5;
        if (angle >= 360) angle = 0;
        adsb.aircraft.pushRotateZoom(&adsb.fb_0, 130, 100, angle, 1.0, 1.0);

        adsb.push_fb();

        // Yield to other tasks
		vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}






// #############################################################################################################
// EVENT TASK
// #############################################################################################################
static void task_print_info(void *pvParameters) {    
    log_d("DISPLAY INFO TASK STARTED");
    
    for(;;) {
        log_d("DISPLAY INFO TASK RUNNING");


        log_i("Free RAM: %.2f KB\n", ESP.getFreeHeap() / 1024.0);

        // Yield to other tasks
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}