#include <Arduino.h>
#include "esp_gcs_hal_display_device.h"
#include "esp_gcs_hal_framebuffer_manager.h"
#include "esp_gcs_hal_render_task.h"


// --- Global objects for the application ---
DisplayConfig config;
DisplayDevice device(config);
FrameBufferManager fbManager(config);
RenderTask renderer(device, fbManager, config);

// --- Our application's drawing function ---
void myDrawFunction(LGFX_Sprite& sprite, void* context) {
    
    static float angle = 0.0f;

    // Use float math
    float x = sprite.width()  * 0.5f + sinf(angle) * 100.0f;
    float y = sprite.height() * 0.5f + cosf(angle) * 100.0f;

    // Update angle
    angle += 0.05f;
    if (angle > TWO_PI) angle -= TWO_PI;

    sprite.fillScreen(TFT_BLACK);
    sprite.fillCircle((int)x, (int)y, 40, TFT_BLUE);

    sprite.setTextSize(1);
    sprite.setTextColor(TFT_YELLOW, TFT_TRANSPARENT);
    sprite.drawString("Double Buffer (4-bit)", 50, 10);

    sprite.setTextSize(2);
    char buf[32];
    snprintf(buf, sizeof(buf), "FPS: %.1f", renderer.getFPS());
    sprite.setTextColor(TFT_RED, TFT_TRANSPARENT);
    sprite.drawString(buf, 10, sprite.height() - 30);
}


void setup2() {

    log_i("=== Display HAL Test ===");

    // 1. Set up configuration with actual pin numbers    
    config.pin_cs = 37;
    config.pin_blk = 45;
    
    config.rotation = 1;
    config.use_psram = true;
    config.target_fps = 60;
    config.framebuffer_count = 2;
    config.color_depth = 16; // Use 16-bit color for better quality
    config.task_priority = 2;
    config.task_stack_size = 8192; // Increased stack size

    // 2. Initialize hardware (in order)
    log_i("Initializing DisplayDevice...");
    if (!device.init()) {
        log_e("FAILED to init DisplayDevice!");
        while(1) { delay(1000); } // Halt
    }
    log_i("DisplayDevice initialized OK");

    // 3. Initialize framebuffer - pass the device directly, not getPanel()
    log_i("Initializing FrameBufferManager...");
    if (!fbManager.init(device.get())) {
        log_e("FAILED to init FrameBufferManager!");
        while(1) { delay(1000); } // Halt
    }
    log_i("FrameBufferManager initialized OK (%d buffers, %dx%d)", 
          fbManager.getCount(), fbManager.getWidth(), fbManager.getHeight());

    // 4. Start the rendering task FIRST
    log_i("Starting RenderTask...");
    if (!renderer.start()) {
        log_e("FAILED to start RenderTask!");
        while(1) { delay(1000); } // Halt
    }
    log_i("RenderTask started OK");
    
    // 5. Set the callback AFTER task is running
    log_i("Setting draw callback...");
    renderer.setDrawCallback(myDrawFunction, nullptr);
    log_i("Draw callback set");
    
    log_i("=== Setup Complete ===");
}

void loop2() {}