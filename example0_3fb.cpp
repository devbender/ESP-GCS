#define LGFX_USE_V1
#include <LovyanGFX.hpp>

#include "esp_gcs_display_parallel16_9488.h"

// =====================================================
// Globals
// =====================================================
LGFX_Parallel_9488 lcd;
LGFX_Sprite fb[3] = { LGFX_Sprite(&lcd), LGFX_Sprite(&lcd), LGFX_Sprite(&lcd) };

// Buffer Management (volatile for thread-safety)
volatile int dma_index = -1;  // Buffer index currently transferring via DMA (-1 when idle)
volatile int draw_index = 0; // Buffer index currently being drawn into by the CPU
volatile int ready_index = -1; // Buffer index ready to be transferred

float fps = 0.0f;
uint32_t frame_counter = 0;
uint32_t last_fps_update = 0;

const int SCREEN_W = 480;
const int SCREEN_H = 320;

const int CENTER_X = (SCREEN_W / 2);
const int CENTER_Y = (SCREEN_H / 2);


// =====================================================
// Setup
// =====================================================
void setup() {

    pinMode(LCD_CS, OUTPUT);
    pinMode(LCD_BLK, OUTPUT);
    digitalWrite(LCD_CS, LOW);
    digitalWrite(LCD_BLK, HIGH);
    
    lcd.init();
    lcd.initDMA();
    lcd.setRotation(1);
    lcd.fillScreen(TFT_BLACK);

    // log_i is replaced with Serial.printf for standard Arduino compatibility
    log_i("PSRAM total: %d bytes | free: %d bytes", ESP.getPsramSize(), ESP.getFreePsram());

    // Allocate all three framebuffers in PSRAM
    for (int i = 0; i < 3; i++) {
        fb[i].setPsram(true);
        fb[i].setColorDepth(8);
        if (!fb[i].createSprite(SCREEN_W, SCREEN_H)) {
            log_i("Failed to allocate fb[%d]", i);
            lcd.drawCenterString("Framebuffer allocation failed!", SCREEN_W / 2, SCREEN_H / 2, 2);
            while (true) delay(1000);
        }        
        fb[i].fillScreen(TFT_BLACK);
    }

    lcd.startWrite();
    last_fps_update = millis();

    log_i("PSRAM total: %d bytes | free: %d bytes", ESP.getPsramSize(), ESP.getFreePsram());
    delay(1000);
}



void loop() {
    uint32_t now = millis();

    // 1. Check if DMA transfer is complete
    if (dma_index != -1 && !lcd.dmaBusy()) {
        dma_index = -1; 
    }

    // 2. DRAW PHASE
    LGFX_Sprite* draw_fb = &fb[draw_index];

    static float angle = 0.0f;
    int x = CENTER_X + (int)(sin(angle) * 100);
    int y = CENTER_Y + (int)(cos(angle) * 100);
    angle += 0.05f;

    // Just clear the whole screen - it's actually faster than tracking in triple buffer
    draw_fb->fillScreen(TFT_BLACK);    
    draw_fb->fillCircle(x, y, 40, TFT_BLUE);    
    draw_fb->setTextSize(1);
    draw_fb->setTextColor(TFT_WHITE, TFT_BLACK);
    draw_fb->drawString("Triple Buffer (16-bit, DMA)", 50, 10);
    
    draw_fb->setTextSize(2);
    char buf[32];
    snprintf(buf, sizeof(buf), "FPS: %.1f", fps);
    draw_fb->setTextColor(TFT_YELLOW, TFT_BLACK);
    draw_fb->drawString(buf, 10, SCREEN_H - 30);

    // 3. Mark buffer as ready
    ready_index = draw_index;

    // 4. Switch to next buffer for drawing
    int next = (draw_index + 1) % 3;
    if (next == dma_index) next = (next + 1) % 3;    
    draw_index = next;

    // 5. Start DMA if possible
    if (ready_index != -1 && dma_index == -1) {
        dma_index = ready_index;

        lcd.pushImageDMA(
            0, 0, SCREEN_W, SCREEN_H,
            fb[dma_index].getBuffer(),
            fb[dma_index].getColorDepth(),
            fb[dma_index].getPalette()
        );
        
        ready_index = -1;
    }

    // 6. FPS calculation
    frame_counter++;
    if (now - last_fps_update >= 1000) {
        fps = (frame_counter * 1000.0f) / (now - last_fps_update);
        frame_counter = 0;
        last_fps_update = now;
    }
    
}