#pragma once
#include "esp_task_wdt.h"
#include "esp_gcs_hal_display_device.h"
#include "esp_gcs_hal_framebuffer_manager.h"
#include "esp_gcs_hal_display_config.h"
#include <atomic>


// C-style function pointer for max performance (no std::function overhead)
using RenderCallback_t = void(*)(LGFX_Sprite& sprite, void* context);

/// @brief Commands sent to the RenderTask's message queue.
struct RenderCommand {
    enum CommandType {
        CMD_SET_CALLBACK,
        CMD_STOP // Future use
    } type;

    // Payload for CMD_SET_CALLBACK
    RenderCallback_t callback = nullptr;
    void* context = nullptr;
};


/// @brief Manages the FreeRTOS rendering task and pipeline.
/// Does NOT own the device or buffers, just uses them.
class RenderTask {
public:
    RenderTask(DisplayDevice& device, FrameBufferManager& fbm, const DisplayConfig& cfg);
    ~RenderTask(); // Calls stop()

    // Non-copyable
    RenderTask(const RenderTask&) = delete;
    RenderTask& operator=(const RenderTask&) = delete;

    /// @brief Creates and starts the FreeRTOS render task.
    /// @return true on success.
    bool start();

    /// @brief Safely stops the render task.
    /// Signals the task and BLOCKS until the task confirms it has exited.
    /// This is 100% safe.
    void stop();

    /// @brief Set the drawing callback.
    /// This is thread-safe, non-blocking, and can be called from any task.
    /// @param cb The function pointer.
    /// @param context A user-defined pointer passed to the callback.
    void setDrawCallback(RenderCallback_t cb, void* context);

    /// @brief Get current FPS. Thread-safe.
    float getFPS() const { return fps.load(std::memory_order_relaxed); }

private:
    static void taskLoopEntry(void* arg);
    void taskLoop();
    void processQueue();
    void updateFPS();

    DisplayDevice& device;
    FrameBufferManager& buffers;
    const DisplayConfig& config;

    TaskHandle_t task_handle = nullptr;
    QueueHandle_t command_queue = nullptr;
    SemaphoreHandle_t shutdown_signal = nullptr;

    std::atomic<bool> running{false};
    std::atomic<float> fps{0.0f};

    // --- State private to the render task (no mutex needed) ---
    RenderCallback_t current_callback = nullptr;
    void* current_context = nullptr;
    int draw_index = 0;
    uint32_t frame_counter = 0;
    uint32_t last_fps_update = 0;
};