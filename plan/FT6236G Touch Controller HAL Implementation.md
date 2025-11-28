# Problem Statement
The FT6236G touch controller needs to be integrated into the current HAL-based system design. The existing `esp_gcs_system.h/cpp` implementation is deprecated and uses the old architecture with static members and direct FreeRTOS task management. The new system follows a three-layer HAL abstraction (DisplayDevice, FrameBufferManager, RenderTask) with RAII principles and thread-safe callback patterns.
# Current State
## Deprecated Touch Implementation
The old system (`esp_gcs_system.cpp`) implements touch handling via:
* `FT6236G` library from bitbank2 (platformio dependency)
* Static touch controller instance: `FT6236G ESP_GCS_SYSTEM::tc`
* Static `TOUCHINFO` and `touch_point_t` structs for storing touch data
* Hardcoded I2C pins: SDA=38, SCL=39
* `task_touch_events()` FreeRTOS task that polls `tc.getSamples(&ti)` continuously
* Event queue system: touch events are sent to `queue_events` and processed by `task_system_events()`
* Coordinate transformation: `tpoint.x = ti.y[0]; tpoint.y = 320 - ti.x[0]` to account for display rotation
* 100ms debounce delay after detecting touch position changes
## Alternative Touch Implementation
There's also `esp_gcs_input_ft6236.h` with a custom `INPUT_FT6236` class:
* Uses Arduino Wire library directly for I2C communication
* Implements register-level access: `TOUCH_REG_XH`, `TOUCH_REG_XL`, `TOUCH_REG_YH`, `TOUCH_REG_YL`
* I2C address: `0x38`
* Provides `getTouchPointX()`, `getTouchPointY()`, and `ft6236_pos()` methods
* Event detection via bit 6-7 of XH register (returns -1 if no touch)
* No external library dependency, direct Wire protocol implementation
## Current HAL Architecture
The new system (used by `ESP_GCS_ADSB`) follows:
1. **DisplayConfig** - Configuration struct with pin numbers, rendering parameters, task settings
2. **DisplayDevice** - RAII wrapper managing LGFX device lifecycle, owns the hardware
3. **FrameBufferManager** - Allocates and owns LGFX_Sprite framebuffers
4. **RenderTask** - FreeRTOS task orchestration with thread-safe callback system via command queue
Pattern: Constructor initializes with config references, `begin()` method chains initialization calls, callback-based rendering via `renderer.setDrawCallback(draw_loop, this)`.
# Proposed Changes
## 1. Create TouchConfig Structure
Add to `esp_gcs_hal_display_config.h`:
* I2C pin configuration (SDA, SCL pins)
* I2C frequency (default 400kHz)
* Optional: debounce time, polling interval
* Optional: coordinate transformation settings (rotation compensation)
## 2. Create TouchDevice HAL Class
New files: `esp_gcs_hal_touch_device.h` and `esp_gcs_hal_touch_device.cpp`
* RAII wrapper managing FT6236G touch controller lifecycle
* Constructor takes `TouchConfig` reference
* `init()` method to initialize I2C and touch controller
* Non-copyable (deleted copy constructor/assignment)
* Encapsulates FT6236G library instance or direct Wire implementation
* Provides `bool getTouchPoint(uint16_t& x, uint16_t& y)` method
* Handles coordinate transformation based on display rotation
* Thread-safe access via mutex if needed for multi-task access
## 3. Create TouchTask HAL Class
New files: `esp_gcs_hal_touch_task.h` and `esp_gcs_hal_touch_task.cpp`
* Manages FreeRTOS touch polling task
* Constructor takes `TouchDevice&` and `TouchConfig&` references
* `start()` and `stop()` methods following RenderTask pattern
* Command queue for thread-safe callback updates
* Callback signature: `using TouchCallback_t = void(*)(uint16_t x, uint16_t y, void* context)`
* `setTouchCallback(TouchCallback_t cb, void* context)` method
* Internal polling loop with configurable rate
* Debouncing logic to filter rapid position changes
* Watchdog subscription for long-running task
## 4. Integration Pattern
Application classes (like ESP_GCS_ADSB) will:
* Add `TouchConfig` member to config struct (or separate config)
* Add `TouchDevice` member initialized with config reference
* Add `TouchTask` member initialized with TouchDevice and config references
* Call `touchDevice.init()` in `begin()` method
* Call `touchTask.start()` in `begin()` method
* Set touch callback via `touchTask.setTouchCallback(touch_handler, this)`
* Implement static `touch_handler(uint16_t x, uint16_t y, void* context)` method
## 5. Update ESP_GCS_ADSB Example
Modify `esp_gcs_adsb.h/cpp` to demonstrate touch integration:
* Add touch config setup in constructor
* Initialize touch device and task in `begin()`
* Implement example touch callback that logs coordinates
* Show pattern for accessing aircraft list or other app state from touch handler
## Implementation Notes
* Use FT6236G library (already in dependencies) rather than custom Wire implementation for reliability
* Follow existing HAL patterns: RAII, reference-based composition, thread-safe callbacks
* Keep polling task separate from render task for clean separation of concerns
* Support both touch callback and optional event queue mechanisms
* Coordinate transformation should be configurable based on display rotation setting
* Debounce implementation: track last valid position and timestamp, only invoke callback if position changed and debounce time elapsed
