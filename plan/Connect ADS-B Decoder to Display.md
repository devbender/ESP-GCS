# Problem Statement
The ADS-B decoder in ESP_GCS_DATALINK processes raw ADS-B messages and stores aircraft data in `adsb_context.aircraft[]` array, but this data is not currently being transferred to the ESP_GCS_ADSB display class. The display class currently uses test data added manually via `add_aircraft()`. We need to connect the decoder's real aircraft data to the display.
# Current State
## Data Flow
1. **ESP_GCS_DATALINK** receives ADS-B messages via TCP and decodes them using `adsb_decode_message()` in `process_raw()`
2. Decoded aircraft are stored in `static adsb_context m_adsb_ctx` with array: `aircraft[ADSB_MAX_AIRCRAFT]`
3. **adsb_data** struct contains: `icao, alt, lat, lon, heading, speed, v_speed, last_seen, callsign, valid_pos, valid_vel`
4. **ESP_GCS_ADSB** maintains its own `std::unordered_map<uint32_t, aircraft_data_t> aircraft_list` for display
5. **aircraft_data_t** struct contains: `x, y, heading, ttl` (screen coordinates, not geo coordinates)
## Key Differences
* Decoder stores geographic data (lat/lon in degrees)
* Display needs screen pixel coordinates (x, y)
* Decoder uses array with ICAO lookup, display uses hash map
* Display has TTL (time-to-live) for aging out stale aircraft visually
## Current Test Data
`main.cpp` manually adds test aircraft: `adsb.add_aircraft(38, {140, 100, 45, 60})`
This uses pixel coordinates (140, 100) directly, which is what the display needs.
# Proposed Solution
## Architecture
Create a periodic sync mechanism that:
1. Reads aircraft from datalink's `adsb_context` 
2. Converts geographic coordinates (lat/lon) to screen coordinates (x, y)
3. Updates display's `aircraft_list` map
4. Handles ownship positioning and range scaling
## Implementation Steps
### 1. Add coordinate conversion function to ESP_GCS_ADSB
Create method to convert lat/lon to relative screen coordinates:
* Takes reference position (ownship lat/lon)
* Calculates relative bearing and distance
* Maps to circular display area (centered, scaled by range)
* Returns screen pixel coordinates (x, y)
### 2. Add sync method to ESP_GCS_ADSB
Create `sync_from_datalink(const adsb_context& ctx)` method:
* Iterate through `ctx.aircraft[]` array
* Skip empty slots (icao == 0) and invalid positions (!valid_pos)
* Convert each aircraft's lat/lon to screen x/y
* Call `add_aircraft()` with converted coordinates
* Existing `add_aircraft()` already handles thread-safe map updates and TTL reset
### 3. Call sync periodically from main loop
In `main.cpp` loop():
* Get reference to datalink context via `ESP_GCS_DATALINK::get_adsb_context()`
* Call `adsb.sync_from_datalink(ctx)` every iteration or at desired rate
* This replaces the current test code that manually rotates headings
### 4. Optional: Add range/scale configuration
* Add display range setting (e.g., 10 NM, 20 NM radius)
* Add zoom factor to DisplayConfig or ESP_GCS_ADSB
* Use this in coordinate conversion to scale appropriately
## Coordinate Conversion Details
For circular radar-style display:
```warp-runnable-command
1. Calculate relative position:
   delta_lat = aircraft_lat - ownship_lat
   delta_lon = aircraft_lon - ownship_lon
   
2. Convert to distance/bearing:
   Use haversine or flat-earth approximation for short distances
   distance_nm = sqrt((delta_lat*60)^2 + (delta_lon*60*cos(lat))^2)
   bearing_deg = atan2(delta_lon, delta_lat) * 180/PI
   
3. Map to screen coordinates:
   radius_pixels = (distance_nm / max_range_nm) * display_radius
   angle_rad = bearing_deg * PI/180
   x = center_x + radius_pixels * sin(angle_rad)
   y = center_y - radius_pixels * cos(angle_rad)  // Inverted Y
```
## Ownship Handling
Currently ownship sprite is drawn at screen center. Two approaches:
1. **Fixed center**: Ownship always at center, all traffic relative to it (current approach is correct)
2. Set ownship lat/lon and use it as reference for coordinate conversion
For approach 1 (simpler): Set a fixed reference position via `set_cpr_local_reference()` and draw all aircraft relative to that, with ownship at center.
# Files to Modify
* `esp_gcs_adsb.h` - Add sync method and coordinate conversion method declarations
* `esp_gcs_adsb.cpp` - Implement sync and coordinate conversion
* `main.cpp` - Replace test code with sync call in loop()
