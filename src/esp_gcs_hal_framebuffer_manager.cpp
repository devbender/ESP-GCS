#include "esp_gcs_hal_framebuffer_manager.h"

FrameBufferManager::FrameBufferManager(const DisplayConfig& cfg) 
    : config(cfg) {
}

FrameBufferManager::~FrameBufferManager() {
    // unique_ptr automatically cleans up sprites
    buffers.clear();
}

bool FrameBufferManager::init(LGFX_Parallel_9488* panel) {
    if (!panel) {
        log_e("panel is null");
        return false;
    }
    
    screen_w = panel->width();
    screen_h = panel->height();
    
    log_i("Screen dimensions: %dx%d", screen_w, screen_h);
    
    // Check available memory
    if (config.use_psram) {
        log_i("PSRAM free: %d bytes", ESP.getFreePsram());
    }
    log_i("Heap free: %d bytes", ESP.getFreeHeap());
    
    // Calculate memory needed per buffer
    size_t bytes_per_pixel = config.color_depth / 8;
    size_t buffer_size = screen_w * screen_h * bytes_per_pixel;
    log_i("Memory per buffer: %d bytes (%d-bit color)", buffer_size, config.color_depth);
    
    // Allocate framebuffers
    for (int i = 0; i < config.framebuffer_count; i++) {
        std::unique_ptr<LGFX_Sprite> framebuffer(new LGFX_Sprite(panel));
        
        // Create sprite with color depth
        if (config.use_psram) {
            framebuffer->setPsram(true);
        }
        
        framebuffer->setColorDepth(config.color_depth);
        
        if (!framebuffer->createSprite(screen_w, screen_h)) {
            log_e("Failed to create framebuffer %d (need %d bytes)", i, buffer_size);
            log_e("PSRAM free: %d, Heap free: %d", ESP.getFreePsram(), ESP.getFreeHeap());
            buffers.clear();
            return false;
        }
        
        log_i("Created buffer %d at %p", i, framebuffer->getBuffer());
        buffers.push_back(std::move(framebuffer));
    }
    
    log_i("All %d framebuffers allocated successfully", config.framebuffer_count);
    return true;
}