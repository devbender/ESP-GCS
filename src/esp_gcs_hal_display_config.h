#pragma once
#include <LovyanGFX.hpp>

/// @brief Configuration for Display initialization
struct DisplayConfig {
    // --- Hardware ---
    int pin_cs = -1;
    int pin_blk = -1;
    int pin_dc = -1;
    int pin_rst = -1;
    // ... add all other LGFX pins (DATA, WR, RD) ...

    // --- Rendering ---
    uint8_t color_depth = 8;
    uint8_t framebuffer_count = 2;
    uint8_t rotation = 1;
    bool use_psram = false;
    uint32_t target_fps = 0;

    // --- Task ---
    uint32_t task_stack_size = 4096;
    UBaseType_t task_priority = 5;
    BaseType_t task_core = 1;
};