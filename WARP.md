# WARP.md

This file provides guidance to WARP (warp.dev) when working with code in this repository.

## Project Overview

ESP-GCS is an ESP32-based Ground Control Station with display capabilities for aviation data visualization. The project focuses on Primary Flight Display (PFD), ADS-B aircraft tracking, and Horizontal Situation Indicator (HSI) displays for ESP32-S3 devices with PSRAM and LCD screens.

## Build Commands

### Initial Setup
```bash
# Clone with submodules (MAVLink is included as submodule)
git submodule update --init --recursive

# Install PlatformIO if not already installed
pip install platformio
```

### Configuration
Before building, copy and configure the WiFi settings:
```bash
# Copy the config template
cp src/esp_gcs_config.h src/my_esp_gcs_config.h
# Edit my_esp_gcs_config.h with your WiFi credentials
```

### Build & Upload
```bash
# Build the project
platformio run

# Upload to device (ensure board is connected via /dev/ttyUSB0)
platformio run --target upload

# Monitor serial output
platformio device monitor

# Build for specific environment
platformio run -e esp32-s3-wroom-1-n16r2
```

### Development Commands
```bash
# Clean build artifacts
platformio run --target clean

# Update libraries
platformio lib update
```

## Code Architecture

### Hardware Abstraction Layer (HAL)

The display system uses a three-layer abstraction to separate hardware concerns from application logic:

1. **DisplayDevice** (`esp_gcs_hal_display_device.h/cpp`) - Manages the raw LGFX device lifecycle using RAII principles
2. **FrameBufferManager** (`esp_gcs_hal_framebuffer_manager.h/cpp`) - Allocates and owns LGFX_Sprite framebuffers for double/triple buffering
3. **RenderTask** (`esp_gcs_hal_render_task.h/cpp`) - Manages the FreeRTOS rendering task and DMA pipeline with thread-safe callback system

This architecture ensures clean separation: DisplayDevice owns hardware, FrameBufferManager owns memory, and RenderTask orchestrates the render loop.

### Display Systems

**ESP_GCS_SYSTEM** - Base system class providing:
- LCD initialization and framebuffer management
- Touch input handling (FT6236G)
- FreeRTOS task coordination
- Memory diagnostics (PSRAM, flash partition info)
- 4-bit and 8-bit color palette management

**Specialized Display Classes** (inherit from ESP_GCS_SYSTEM):
- **ESP_GCS_PFD** - Primary Flight Display with artificial horizon, pitch ladder, turn indicator
- **ESP_GCS_ADSB** - ADS-B aircraft tracking display with multiple aircraft sprites
- **ESP_GCS_HSI** - Horizontal Situation Indicator for navigation

### Data Link Layer

**ESP_GCS_DATALINK** - Network communication handler supporting:
- TCP/UDP connectivity
- Multiple protocols: MAVLink, SBS-1, Raw ADS-B
- Async TCP for non-blocking I/O
- MAVLink message parsing (heartbeat, attitude, VFR HUD)
- ADS-B decoding with CPR (Compact Position Reporting)

### Configuration System

All system configuration flows through `esp_gcs_config_t` struct (`esp_gcs_types.h`):
- Display type and dimensions
- Network settings (WiFi, IP, port)
- Link type (TCP/UDP/Bluetooth/Serial/ESP-NOW)
- Protocol selection (ADSB_MAVLINK/ADSB_SBS1/ADSB_RAW)

### Display Hardware Support

Two display drivers are available:
- **PARALLEL_9488**: ILI9488 via 16-bit parallel bus (primary target, see `esp_gcs_display_parallel16_9488.h`)
- **SPI_9342**: ILI9342 via SPI (alternate, see `esp_gcs_display_spi_9342.h`)

Pin configurations are hardcoded in display driver classes. LCD_CS (pin 37) and LCD_BLK (pin 45) are defined constants.

### Color System

The project uses palette-based rendering for memory efficiency:
- 4-bit palettes for memory-constrained sprites
- 8-bit palettes for higher color depth rendering
- Custom palette setup via `set_palette_4bit()` in ESP_GCS_SYSTEM
- Color constants defined in `esp_gcs_colors.h`

### MAVLink Integration

MAVLink is included as a git submodule under `include/mavlink/`. The project uses:
- `mavlink_heartbeat_t` - Vehicle status and flight mode
- `mavlink_attitude_t` - Roll, pitch, yaw for PFD
- `mavlink_vfr_hud_t` - Airspeed, altitude, heading, climb rate

Flight modes are mapped in `esp_gcs_types.h` with associated colors for display rendering.

### FreeRTOS Task Architecture

The system uses multiple concurrent tasks:
- **Render Task**: Manages display refresh with DMA and double/triple buffering
- **Touch Events Task**: Handles touchscreen input processing
- **System Events Task**: Coordinates system-level events
- **Framebuffer Task**: Manages framebuffer swapping and synchronization

Tasks communicate via FreeRTOS queues and semaphores. The render task uses a command queue for thread-safe callback updates.

## Key Development Patterns

### Framebuffer Rendering Flow
1. Application draws to a drawing buffer (sprite)
2. Callback function passed to RenderTask via `setDrawCallback()`
3. RenderTask calls user callback with available framebuffer sprite
4. User draws into sprite reference
5. RenderTask pushes completed buffer to LCD via DMA
6. Buffer swapping managed automatically by triple-buffer logic

### Memory Management
- PSRAM is used for large framebuffers (enable with `BOARD_HAS_PSRAM` build flag)
- Sprites use LGFX memory allocation (default or PSRAM based on config)
- Color depth reduction (4-bit/8-bit) conserves memory
- Check free memory: `ESP.getFreeHeap()` and `ESP.getFreePsram()`

### Display Rotation
Standard rotation is `1` (landscape mode), configured via:
```cpp
lcd.setRotation(1);
```

## Target Hardware

- **Board**: ESP32-S3-WROOM-1-N16R2 (16MB Flash, 2MB PSRAM)
- **Display**: ILI9488 320x480 or ILI9341 compatible
- **Touch**: FT6236G capacitive touch controller
- **Serial**: /dev/ttyUSB0 at 115200 baud
- **Memory**: Custom partition table (`esp_gcs_partitions_16mb.csv`) for 16MB flash

## Logging

The project uses ESP-IDF logging macros:
- `log_i()` - Info level
- `log_d()` - Debug level
- `log_e()` - Error level
- Set `CORE_DEBUG_LEVEL` in platformio.ini (0=none, 3=info, 5=verbose)

## Common Pitfalls

- **Missing Config**: Must create `src/my_esp_gcs_config.h` from template before building
- **Submodules**: MAVLink won't compile if submodules not initialized
- **PSRAM**: Ensure PSRAM flags are set correctly for large framebuffers
- **DMA Conflicts**: Don't modify framebuffers while DMA transfer is active (use `waitDMA()` or check `dmaBusy()`)
- **Color Depth**: Palette-based rendering requires palette setup before drawing
- **Thread Safety**: Use mutex/semaphores when accessing shared resources from multiple tasks
