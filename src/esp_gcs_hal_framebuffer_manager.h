#pragma once
#include <LovyanGFX.hpp>
#include <vector>
#include <memory>
#include "esp_gcs_hal_display_config.h"
#include "esp_gcs_display_parallel16_9488.h"

/// @brief Allocates and owns the LGFX_Sprite framebuffers.
class FrameBufferManager {
public:
    FrameBufferManager(const DisplayConfig& cfg);
    ~FrameBufferManager(); // Frees all sprites

    // Non-copyable
    FrameBufferManager(const FrameBufferManager&) = delete;
    FrameBufferManager& operator=(const FrameBufferManager&) = delete;

    /// @brief Allocates all framebuffers.
    /// @param panel The LGFX device to parent the sprites to.
    /// @return true on success.
    bool init(LGFX_Parallel_9488* panel);

    /// @brief Get a specific framebuffer.
    LGFX_Sprite* getBuffer(int index) {
        return (index < buffers.size()) ? buffers[index].get() : nullptr;
    }

    /// @brief Get the total number of buffers.
    int getCount() const { return buffers.size(); }
    int getWidth() const { return screen_w; }
    int getHeight() const { return screen_h; }

private:
    std::vector<std::unique_ptr<LGFX_Sprite>> buffers;
    const DisplayConfig& config;
    int screen_w = 0;
    int screen_h = 0;
};