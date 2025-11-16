#pragma once
#include <LovyanGFX.hpp>
#include "esp_gcs_display_parallel16_9488.h" // Your LGFX class
#include "esp_gcs_hal_display_config.h"
#include <memory>

/// @brief Manages the raw LGFX device lifecycle.
/// Follows RAII: Constructor creates, Destructor cleans up.
class DisplayDevice {
public:
    DisplayDevice(const DisplayConfig& cfg);
    ~DisplayDevice(); // Calls lcd->endWrite()

    // Non-copyable
    DisplayDevice(const DisplayDevice&) = delete;
    DisplayDevice& operator=(const DisplayDevice&) = delete;

    /// @brief Initializes the LCD panel and DMA.
    /// @return true on success.
    bool init();

    /// @brief Get a pointer to the raw LGFX device.
    LGFX_Parallel_9488* get() { return lcd.get(); }

private:
    std::unique_ptr<LGFX_Parallel_9488> lcd;
    const DisplayConfig& config;
    bool initialized = false;
};