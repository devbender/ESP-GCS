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
ESP_GCS_DATALINK datalink;

// touch config
FT6236G ct;

#define I2C_SCL 39
#define I2C_SDA 38

uint16_t points[4];
uint16_t o_points[4];

int getTouch(uint16_t *pPoints);



// tasks definitions
static void task_events(void *pvParameters);
static void task_print_info(void *pvParameters);

// #############################################################################################################
// SETUP
// #############################################################################################################
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
    datalink.set_adsb_local_reference(18.4650, -69.9428); // for local CPR
    adsb.push_fb();

    TaskFunction_t task_events_ptr = task_events;
    const char * task_name = "event_task";
    const uint32_t task_stack_size = 8192;
    const UBaseType_t task_priority = 5;
    void *task_parameters = NULL;    
    TaskHandle_t *task_handle = &adsb.xHandleEventsTask;
    
    xTaskCreate(
        task_events_ptr,
        task_name,
        task_stack_size,
        task_parameters,
        task_priority,
        task_handle
    );
    

    ct.init(I2C_SDA, I2C_SCL, false, 400000);

    
}
    


// #############################################################################################################
// LOOP
// #############################################################################################################
void loop() {
  
    //datalink.print_aircrafts();
    //delay(5000);

    int i;  
  
    if(i = getTouch(points)) {    
        //adsb.lcd.fillRect(o_points[0], o_points[1], 10, 10, TFT_BLACK);    
        //adsb.lcd.fillRect(points[0], points[1], 10, 10, TFT_WHITE);

        memcpy(o_points, points, sizeof(points));
        log_i("A %d | %d", points[0], points[1]);

        
        if (i == 2) {      
            //adsb.lcd.fillRect(points[2], points[3], 10, 10, TFT_RED);          
            log_i("B %d | %d", points[2], points[3]);        
        }
    } 


}


int getTouch(uint16_t *pPoints) {
  TOUCHINFO ti;
  if (ct.getSamples(&ti) != FT_SUCCESS)
     return 0; // something went wrong
    if (pPoints) {
      // swap X/Y since the display is used 90 degrees rotated
      pPoints[0] = ti.y[0];
      pPoints[1] = adsb.lcd.height() - ti.x[0];
      pPoints[2] = ti.y[1];
      pPoints[3] = adsb.lcd.height() - ti.x[1];
    }
  return ti.count;
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