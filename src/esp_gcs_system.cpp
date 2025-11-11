#include "esp_gcs_system.h"

ESP_GCS_SYSTEM::ESP_GCS_SYSTEM() { 

 }


void ESP_GCS_SYSTEM::init(void) {
    log_d("System initializing...");

    pinMode(LCD_CS, OUTPUT);
    pinMode(LCD_BLK, OUTPUT);

    digitalWrite(LCD_CS, LOW);
    digitalWrite(LCD_BLK, HIGH);

    lcd.init();
    lcd.setRotation(1);        
    lcd.fillScreen(TFT_BLACK);
    lcd.setPivot( lcd.width()/2, lcd.height()/2 );

    fb_width = 480; fb_height = 320;
    
    cfg_sp(&fb_0, fb_width, fb_height);
    cfg_sp(&fb_1, fb_width, fb_height);    
}





void ESP_GCS_SYSTEM::cfg_sp(LGFX_Sprite *sp, int width, int height) {
    sp->setColorDepth(4);    
    sp->createSprite(width, height);
    set_palette_4bit(sp);
    sp->fillSprite(COLOR_TRANSPARENT);
}




// RGB to 565 coversion
static inline uint16_t RGB(uint8_t R, uint8_t G, uint8_t B) {
  return ((R & 0xF8) << 8) | ((G & 0xFC) << 3) | (B >> 3);
}


void ESP_GCS_SYSTEM::set_palette_4bit(LGFX_Sprite *layer) {

    layer->setPaletteColor(COLOR_BLACK,     TFT_BLACK);
    layer->setPaletteColor(COLOR_WHITE,     TFT_WHITE);
    layer->setPaletteColor(COLOR_SKY,       0x02B5);
    layer->setPaletteColor(COLOR_GND,       0x5140);
    layer->setPaletteColor(COLOR_YELLOW,    0xEE4C);
    layer->setPaletteColor(COLOR_RED,       TFT_RED);
    
    layer->setPaletteColor(UI_BACKGROUND_COLOR,         RGB( 60, 60, 60 ) ); //layer->setPaletteColor(6, TFT_BLACK);
    layer->setPaletteColor(UI_TOP_BAR_DIV_LINE_COLOR,   RGB( 150, 150, 150 ) );
    layer->setPaletteColor(UI_OUTER_CIRCLE_COLOR,       RGB( 150, 150, 150 ) );
    
    layer->setPaletteColor(COLOR_GREEN, TFT_GREEN);
    layer->setPaletteColor(10, TFT_BLACK);
    layer->setPaletteColor(11, TFT_BLACK);
    layer->setPaletteColor(12, TFT_BLACK);
    layer->setPaletteColor(13, TFT_BLACK);
    layer->setPaletteColor(14, TFT_BLACK);
    
    layer->setPaletteColor(COLOR_TRANSPARENT, TFT_TRANSPARENT);
}




void ESP_GCS_SYSTEM::push_fb(){
    fb_0.pushSprite(0, 0);
}



FT6236G ESP_GCS_SYSTEM::tc;
TOUCHINFO ESP_GCS_SYSTEM::ti;
touch_point_t ESP_GCS_SYSTEM::tpoint;
touch_point_t ESP_GCS_SYSTEM::tpoint_o;

QueueHandle_t ESP_GCS_SYSTEM::queue_events = nullptr;

ESP_GCS_DATALINK ESP_GCS_SYSTEM::datalink;


#define ESP_GCS_EVENT_TOUCH 39

void ESP_GCS_SYSTEM::task_touch_events(void *pvParameters) {

    log_i("task_touch_events - started");
    
    tc.init(ft6236g_i2c_sda, ft6236g_i2c_scl, false, 400000);
    
    for(;;) {        

        if (tc.getSamples(&ti) == FT_SUCCESS) {
            tpoint.x = ti.y[0];
            tpoint.y = 320 - ti.x[0];

            if(tpoint_o.x != tpoint.x || tpoint_o.y != tpoint.y) {
                tpoint_o = tpoint;                

                // send touch event                
                uint8_t event = ESP_GCS_EVENT_TOUCH;
                xQueueSend( queue_events, (void*)&event, 0 );
                
                // debounce
                vTaskDelay(100 / portTICK_PERIOD_MS);
            }

        }
    }
}



void ESP_GCS_SYSTEM::task_system_events(void *pvParameters) {

    log_i("task_system_events - started"); 

    queue_events = xQueueCreate(10, sizeof(uint8_t));
    
    
    for(;;) {

        if(queue_events != 0) {

            // wait for event
            uint8_t event;
            xQueueReceive( queue_events, &event, portMAX_DELAY );
                
            switch(event) {
                case ESP_GCS_EVENT_TOUCH: {
                    log_i("TOUCH EVENT - %d | %d", tpoint.x, tpoint.y);
                    datalink.print_aircrafts();
                    break;
                }

                default: {
                    log_i("UNKNOWN EVENT RECEIVED");
                    break;
                }
            }
            taskYIELD();
        } 
        else {
        
            log_i("SYSTEM EVENT TASK RUNNING");
            vTaskDelay(3000 / portTICK_PERIOD_MS);
        }
    }
}