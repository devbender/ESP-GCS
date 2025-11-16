#include "esp_gcs_hal_render_task.h"

RenderTask::RenderTask(DisplayDevice& dev, FrameBufferManager& fbm, const DisplayConfig& cfg)
    : device(dev), buffers(fbm), config(cfg) {
}

RenderTask::~RenderTask() {
    stop();
}

bool RenderTask::start() {
    if (running.load()) {
        log_w("RenderTask already running");
        return true;
    }
    
    // Create command queue
    command_queue = xQueueCreate(5, sizeof(RenderCommand));
    if (!command_queue) {
        log_e("Failed to create command queue");
        return false;
    }
    
    // Create shutdown signal semaphore
    shutdown_signal = xSemaphoreCreateBinary();
    if (!shutdown_signal) {
        log_e("Failed to create shutdown semaphore");
        vQueueDelete(command_queue);
        return false;
    }
    
    running.store(true);
    
    // Create the render task
    BaseType_t result = xTaskCreatePinnedToCore(
        taskLoopEntry,
        "RenderTask",
        config.task_stack_size,
        this,
        config.task_priority,
        &task_handle,
        config.task_core
    );
    
    if (result != pdPASS) {
        log_e("xTaskCreatePinnedToCore failed");
        running.store(false);
        vQueueDelete(command_queue);
        vSemaphoreDelete(shutdown_signal);
        return false;
    }
    
    log_i("RenderTask created on core %d with priority %d", 
          config.task_core, config.task_priority);
    
    // Give the task a moment to start
    vTaskDelay(pdMS_TO_TICKS(10));
    
    return true;
}

void RenderTask::stop() {
    if (!running.load()) return;
    
    log_i("Stopping RenderTask...");
    running.store(false);
    
    // Wait for task to acknowledge shutdown
    if (shutdown_signal) {
        xSemaphoreTake(shutdown_signal, portMAX_DELAY);
        vSemaphoreDelete(shutdown_signal);
        shutdown_signal = nullptr;
    }
    
    // Clean up queue
    if (command_queue) {
        vQueueDelete(command_queue);
        command_queue = nullptr;
    }
    
    task_handle = nullptr;
    log_i("RenderTask stopped");
}

void RenderTask::setDrawCallback(RenderCallback_t cb, void* context) {
    if (!command_queue) {
        log_w("command_queue is null in setDrawCallback");
        return;
    }
    
    RenderCommand cmd;
    cmd.type = RenderCommand::CMD_SET_CALLBACK;
    cmd.callback = cb;
    cmd.context = context;
    
    // Use portMAX_DELAY to ensure the message is sent
    if (xQueueSend(command_queue, &cmd, portMAX_DELAY) != pdTRUE) {
        log_e("Failed to send callback command");
    } else {
        log_i("Callback command sent successfully");
    }
}

void RenderTask::taskLoopEntry(void* arg) {
    log_i("RenderTask loop starting...");
    static_cast<RenderTask*>(arg)->taskLoop();
}

void RenderTask::taskLoop() {
    
    uint32_t target_frame_time = config.target_fps > 0 ? (1000 / config.target_fps) : 16;
    
    // Use microseconds for precision
    //uint32_t target_frame_time = config.target_fps > 0 ? (1000000 / config.target_fps) : 16667;
    
    log_i("RenderTask loop running (target frame time: %dms)", target_frame_time);
    
    // Verify we have buffers
    if (buffers.getCount() == 0) {
        log_e("CRITICAL: No framebuffers available!");
        running.store(false);
    }
    
    uint32_t frame_num = 0;
    
    while (running.load()) {
        uint32_t frame_start = millis();
        
        // Process any pending commands
        processQueue();
        
        // Get the current drawing buffer
        LGFX_Sprite* sprite = buffers.getBuffer(draw_index);
        
        if (!sprite) {
            log_e("Buffer %d is null!", draw_index);
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }
        
        if (current_callback) {
            // Execute user's drawing function
            current_callback(*sprite, current_context);
            
            // Push to display
            sprite->pushSprite(0, 0);
            
            // Swap buffers
            draw_index = (draw_index + 1) % buffers.getCount();
            frame_counter++;
            frame_num++;
            
            // Debug output every 60 frames
            if (frame_num % 60 == 0) {
                log_d("Frame %d rendered (FPS: %.2f)", frame_num, getFPS());
            }
        } else {
            // No callback set yet, just wait
            if (frame_num == 0) {
                log_w("Waiting for callback to be set...");
            }
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }
        
        updateFPS();
        
        // Frame rate limiting
        uint32_t frame_time = millis() - frame_start;
        if (frame_time < target_frame_time) {
            vTaskDelay(pdMS_TO_TICKS(target_frame_time - frame_time));
        } else if (frame_time > target_frame_time * 2) {
            log_d("Frame took %dms (target: %dms)", frame_time, target_frame_time);
        }
    }
    
    log_i("RenderTask loop exiting...");
    
    // Signal shutdown complete
    if (shutdown_signal) {
        xSemaphoreGive(shutdown_signal);
    }
    
    vTaskDelete(nullptr);
}

void RenderTask::processQueue() {
    RenderCommand cmd;
    
    // Process all pending commands (non-blocking)
    while (xQueueReceive(command_queue, &cmd, 0) == pdTRUE) {
        switch (cmd.type) {
            case RenderCommand::CMD_SET_CALLBACK:
                current_callback = cmd.callback;
                current_context = cmd.context;
                log_i("Callback updated: %p", cmd.callback);
                break;
            case RenderCommand::CMD_STOP:
                running.store(false);
                log_i("Stop command received");
                break;
        }
    }
}

void RenderTask::updateFPS() {
    uint32_t now = millis();
    
    if (now - last_fps_update >= 1000) {
        float elapsed = (now - last_fps_update) / 1000.0f;
        fps.store(frame_counter / elapsed, std::memory_order_relaxed);
        
        frame_counter = 0;
        last_fps_update = now;
    }
}