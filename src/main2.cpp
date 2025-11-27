// #define LGFX_USE_V1
// #include <LovyanGFX.hpp>

// #include "esp_gcs_display_parallel16_9488.h"

// // =====================================================
// // Globals
// // =====================================================
// LGFX_Parallel_9488 lcd;

// const int COLOR_DEPTH = 8;
// const int FRAMEBUFFER_COUNT = 2;
// LGFX_Sprite fb[FRAMEBUFFER_COUNT] = { LGFX_Sprite(&lcd), LGFX_Sprite(&lcd) };

// volatile int draw_index = 0;

// float fps = 0.0f;
// uint32_t frame_counter = 0;
// uint32_t last_fps_update = 0;

// int SCREEN_W, SCREEN_H;
// int CENTER_X, CENTER_Y;
// // =====================================================
// // Setup
// // =====================================================
// void setup() {
//     pinMode(LCD_CS, OUTPUT);
//     pinMode(LCD_BLK, OUTPUT);
//     digitalWrite(LCD_CS, LOW);
//     digitalWrite(LCD_BLK, HIGH);
    
//     lcd.init();
//     lcd.initDMA();
//     lcd.setRotation(1);
//     lcd.fillScreen(TFT_BLACK);
//     lcd.printf("Initializing... | %i x %i\n", lcd.width(), lcd.height());
//     log_i("Initializing... | %i x %i\n", lcd.width(), lcd.height());

//     SCREEN_W = lcd.width();
//     SCREEN_H = lcd.height();
//     CENTER_X = SCREEN_W / 2;
//     CENTER_Y = SCREEN_H / 2;

//     delay(1500);

//     // Try 4-bit first, then 8-bit if you need more colors
//     for (int i = 0; i < FRAMEBUFFER_COUNT; i++) {
//         fb[i].setPsram(true);
//         fb[i].setColorDepth(COLOR_DEPTH);
        
//         if (!fb[i].createSprite( lcd.width(), lcd.height() )  ) {
//             log_i("Failed to allocate fb[%d]", i);
//             lcd.printf("Framebuffer allocation failed!");
//             while (true) delay(1000);
//         }
        
//         // // Simple 4-bit palette
//         // fb[i].setPaletteColor(0, TFT_BLACK);
//         // fb[i].setPaletteColor(1, TFT_WHITE);
//         // fb[i].setPaletteColor(2, TFT_BLUE);
//         // fb[i].setPaletteColor(3, TFT_YELLOW);
//         // for (int c = 4; c < 16; c++) {
//         //     fb[i].setPaletteColor(c, TFT_BLACK);
//         // }
        
//         fb[i].fillScreen(TFT_BLACK);
//     }

//     lcd.startWrite();
//     last_fps_update = millis();
    
//     log_i("Free after alloc: %d", ESP.getFreePsram());
// }


// // =====================================================
// // Loop - Simple and Fast
// // =====================================================
// void loop() {
//     // Wait for previous DMA to finish
//     //while (lcd.dmaBusy()) { }

//     LGFX_Sprite* fb_current = &fb[draw_index];

//     // Calculate position
//     static float angle = 0.0f;
//     int x = CENTER_X + (int)(sin(angle) * 100);
//     int y = CENTER_Y + (int)(cos(angle) * 100);
//     angle += 0.05f;

//     // Draw - fillScreen is actually fastest for full updates
//     fb_current->fillScreen(TFT_BLACK);  // 0 = BLACK
//     fb_current->fillCircle(x, y, 40, TFT_BLUE);  // 2 = BLUE
    
//     fb_current->setTextSize(1);
//     fb_current->setTextColor(TFT_YELLOW, TFT_TRANSPARENT);  // WHITE on BLACK
//     fb_current->drawString("Double Buffer (4-bit)", 50, 10);
    
//     fb_current->setTextSize(2);
//     char buf[32];
//     snprintf(buf, sizeof(buf), "FPS: %.1f", fps);
//     fb_current->setTextColor(TFT_RED, TFT_TRANSPARENT);  // YELLOW on BLACK
//     fb_current->drawString(buf, 10, SCREEN_H - 30);

//     while (lcd.dmaBusy()) { }

//     // Push via DMA
//     lcd.pushImageDMA(
//         0, 0, SCREEN_W, SCREEN_H,
//         fb_current->getBuffer(),
//         fb_current->getColorDepth(),
//         fb_current->getPalette()
//     );

//     // Swap buffers
//     draw_index = 1 - draw_index;

//     // FPS tracking
//     frame_counter++;
//     uint32_t now = millis();
//     if (now - last_fps_update >= 1000) {
//         fps = (frame_counter * 1000.0f) / (now - last_fps_update);
//         frame_counter = 0;
//         last_fps_update = now;        
//     }
// }