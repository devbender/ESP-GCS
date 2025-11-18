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
    lcd.initDMA();
    lcd.setRotation(1);        
    lcd.fillScreen(TFT_BLACK);
    lcd.setPivot( lcd.width()/2, lcd.height()/2 );    

    fb_width = lcd.width(); 
    fb_height = lcd.height();

    lcd.drawCenterString("INITIALIZING...", fb_width/2, fb_height/2);
    delay(1000);


    // initialize framebuffers
    for (int i = 0; i < FRAMEBUFFER_COUNT; i++) {
        fb[i].setPsram(false);
        fb[i].setColorDepth(COLOR_DEPTH);
        
        if (!fb[i].createSprite(fb_width, fb_height)) {
            log_i("Failed to allocate fb[%d]", i);
            lcd.drawCenterString("Framebuffer allocation failed!", fb_width / 2, fb_height / 2);
            while (true) delay(1000);
        }
        else {
            log_i("Framebuffer %d allocated: %dx%d @ %d bpp", i, fb_width, fb_height, COLOR_DEPTH);
            lcd.fillScreen(TFT_BLACK);
            lcd.drawCenterString("FB ALLOCATION OK!", fb_width/2, fb_height/2);
        }
        
        fb[i].fillScreen(TFT_BLACK);
    }

    lcd.startWrite();
    last_fps_update = millis();
    
    
    // cfg_sp(&fb_0, fb_width, fb_height);
    // cfg_sp(&fb_1, fb_width, fb_height);    
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
    //fb_0.pushSprite(0, 0);

// Wait for previous DMA to finish
    while (lcd.dmaBusy()) { }

    LGFX_Sprite* fb_current = &fb[draw_index];

    // Calculate position
    static float angle = 0.0f;
    int x = fb_width/2 + (int)(sin(angle) * 100);
    int y = fb_height/2 + (int)(cos(angle) * 100);
    angle += 0.05f;

    // Draw - fillScreen is actually fastest for full updates
    fb_current->fillScreen(TFT_BLACK);  // 0 = BLACK
    fb_current->fillCircle(x, y, 40, TFT_BLUE);  // 2 = BLUE
    
    fb_current->setTextSize(1);
    fb_current->setTextColor(TFT_YELLOW, TFT_TRANSPARENT);  // WHITE on BLACK
    fb_current->drawString("Double Buffer (4-bit)", 50, 10);
    
    fb_current->setTextSize(2);
    char buf[32];
    snprintf(buf, sizeof(buf), "FPS: %.1f", fps);
    fb_current->setTextColor(TFT_RED, TFT_TRANSPARENT);  // YELLOW on BLACK
    fb_current->drawString(buf, 10, fb_height - 30);

    // Push via DMA
    lcd.pushImageDMA(
        0, 0, fb_width, fb_height,
        fb_current->getBuffer(),
        fb_current->getColorDepth(),
        fb_current->getPalette()
    );

    // Swap buffers
    draw_index = 1 - draw_index;

    // FPS tracking
    frame_counter++;
    uint32_t now = millis();
    if (now - last_fps_update >= 1000) {
        fps = (frame_counter * 1000.0f) / (now - last_fps_update);
        frame_counter = 0;
        last_fps_update = now;        
    }
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





void ESP_GCS_SYSTEM::task_framebuffer(void *pvParameters) {

}