#include "esp_gcs_adsb.h"


ESP_GCS_ADSB::ESP_GCS_ADSB() 
  : config(), device(config), fbManager(config), renderer(device, fbManager, config) {

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

}



ESP_GCS_ADSB::~ESP_GCS_ADSB() {  
  renderer.stop();
  
  if (aircraft_sprite) {
      aircraft_sprite->deleteSprite();
      delete aircraft_sprite;
      aircraft_sprite = nullptr;
  }
}


// Define the shape of the aircraft (triangle + vector) at (0, 0)
bool ESP_GCS_ADSB::init_sprite(LGFX_Sprite*& sprite, uint16_t aircraft_color) {
    
    // 1. Get the parent panel from the DisplayDevice
    LGFX_Parallel_9488* panel = device.get();
    if (!panel) {
        log_e("Panel not initialized for sprite creation.");
        return false;
    }

    // if we already had one, free it first
    if (sprite) {
        sprite->deleteSprite();
        delete sprite;
        sprite = nullptr;
    }

    // Allocate sprite with a valid parent (safe)
    sprite = new LGFX_Sprite(panel);
    if (!sprite) {
        log_e("Failed to allocate aircraft_sprite object.");
        return false;
    }

    // 3. Configure the sprite    
    sprite->setPsram(config.use_psram);
    sprite->setColorDepth(config.color_depth);

    // Create the sprite
    // Add padding to avoid clipping on rotation
    const int sprite_size = 2 * AIRCRAFT_SIZE + 4; // -> e.g., 16+4(padding) = 20
    if (!sprite->createSprite(sprite_size, sprite_size)) {
        log_e("Failed to create aircraft_sprite (%dx%d) psram=%d depth=%d",
              sprite_size, sprite_size, (int)config.use_psram, (int)config.color_depth);
        delete sprite;
        sprite = nullptr;
        return false;
    }
    // fill with black
    sprite->fillScreen(TFT_BLACK);

    // draw sprite
    draw_aircraft_sprite(sprite, aircraft_color);
    
    // Set the pivot point to the center of the sprite for rotation.
    sprite->setPivot(sprite_size/2, sprite_size/2);
    
    log_i("Aircraft sprite initialized.");
    return true;
}


void ESP_GCS_ADSB::draw_aircraft_sprite(LGFX_Sprite* sprite, uint16_t color) {
  
  // Draw the shape at the center of its own sprite.
  const int sprite_size = 2 * AIRCRAFT_SIZE + 4; // -> e.g., 16+4(padding) = 20
  int center = sprite_size / 2;
  int A = AIRCRAFT_SIZE;

  // ============================
  // Main white triangle
  // ============================
  sprite->fillTriangle(
      center - A,           center + A - 1,   // (x, y + A*2-1)
      center,               center - A,       // (x + A, y)
      center + A - 1,       center + A - 1,   // (x + A*2-1, y + A*2-1)
      color
  );

  // ============================
  // Black cut-out triangle
  // ============================
  sprite->fillTriangle(
      center,               center + A - 3,   // (x + A, y + A*2-3)
      center - 4,           center + A - 1,   // (x + A-4, y + A*2-1)
      center + 4,           center + A - 1,   // (x + A+4, y + A*2-1)
      TFT_BLACK
  );
}



// --- Public Methods ---
bool ESP_GCS_ADSB::begin() {
    log_i("Initializing DisplayDevice...");
    if (!device.init()) {
        log_e("FAILED to init DisplayDevice!");
        return false;
    }

    log_i("Initializing FrameBufferManager...");
    if (!fbManager.init(device.get())) {
        log_e("FAILED to init FrameBufferManager!");
        return false;
    }

    // --- NEW STEP: Initialize Aircraft Sprite ---
    log_i("Initializing Aircraft Sprite...");
    if (!init_sprite(aircraft_sprite, TFT_WHITE)) {
        log_e("FAILED to init aircraft sprite!");
        return false;
    }


    log_i("Initializing Ownship Aircraft Sprite...");
    if (!init_sprite(ownship_sprite, TFT_RED)) {
        log_e("FAILED to init aircraft sprite!");
        return false;
    }

    log_i("Starting RenderTask...");
    if (!renderer.start()) {
        log_e("FAILED to start RenderTask!");
        return false;
    }

    // Set the callback.
    // Pass static function 'drawLoop' and a pointer to 'this' class instance as the context.
    log_i("Setting draw callback...");
    renderer.setDrawCallback(draw_loop, this);

    log_i("begin() complete.");
    return true;
}



void ESP_GCS_ADSB::draw_loop(LGFX_Sprite& fb, void* context) {
    // 'context' is the 'this' pointer we passed in begin().
    // Cast it back to our class type.
    ESP_GCS_ADSB* self = static_cast<ESP_GCS_ADSB*>(context);

    // Now we can call our non-static member functions
    //self->draw_aircraft(sprite);
    self->render_ui_layer(&fb);

    // adsb ui small delay
    vTaskDelay(1);

    // Draw FPS
    char buf[32];
    snprintf(buf, sizeof(buf), "FPS:%.1f", self->renderer.getFPS());
    fb.setTextColor(TFT_GREEN, TFT_TRANSPARENT);
    fb.drawString(buf, fb.width()-60, 10);
}






void ESP_GCS_ADSB::add_aircraft(uint32_t icao, aircraft_data_t aircraft) {
  
  // CRITICAL SECTION START
  // std::lock_guard locks the mutex upon creation.
  // It is an RAII object, guaranteeing the mutex is unlocked
  // when 'lock' goes out of scope (i.e., when the function exits).
  std::lock_guard<std::mutex> lock(aircraft_list_mutex);


  // 1. Reset Time To Live (TTL)
  // Receiving new data means the aircraft is 'fresh', so reset its life counter.
  aircraft.ttl = AIRCRAFT_TTL; 
  
  // 2. Insert or Update
  // std::unordered_map::operator[] handles both cases:
  // - If 'icao' exists, it updates the data.
  // - If 'icao' does not exist, it inserts a new entry.
  aircraft_list[icao] = aircraft;
  
  // CRITICAL SECTION END
}


void ESP_GCS_ADSB::render_ui_layer(LGFX_Sprite *layer) {
  
  // Background
	layer->fillScreen(TFT_DARKGRAY);	
		
	// Top Black Bar
	layer->fillRect(0,0, layer->width(), 30, TFT_BLACK);	
	layer->drawFastHLine(0, 30, layer->width(), TFT_LIGHTGRAY);

	// Top bar text	
	layer->setTextSize(1.5); layer->setTextColor(TFT_WHITE);
	layer->setCursor(10, 12); layer->print("ESP-GCS");

	// Restore Default Font	
	layer->setTextSize(1);
	layer->setTextColor(TFT_WHITE);

  // Traffic drawing area
	layer->fillCircle(layer->width()/2, layer->height()/2, layer->height()/2-1, TFT_BLACK);
  layer->drawCircle(layer->width()/2,  layer->height()/2, layer->height()/2-1, TFT_LIGHTGRAY); 

  // Inner Circle Params
	int degMark = 15;
	int ri = layer->height() / 4;	
  int x_center = layer->width() / 2;
  int y_center = layer->height() / 2;

	// Draw inner circle
	for (int i = 0; i<360; i += degMark) {			
		float a = radians(i);
		layer->drawPixel(x_center + cos(a)*ri, y_center + sin(a)*ri, TFT_WHITE);
  }
  
  // draw my aircraft  
  ownship_sprite->pushRotateZoom( layer, x_center, y_center, 0, 1.0, 1.0 );

  // Copy aircraft list quickly under lock
  std::vector<aircraft_data_t> snapshot;
  snapshot.reserve(aircraft_list.size());

  {
    std::lock_guard<std::mutex> lock(aircraft_list_mutex);

    for (const auto& key_val : aircraft_list) {
        snapshot.push_back(key_val.second);   // copy only
    }
  }

  // Render WITHOUT holding the lock
  for (auto& ac : snapshot) {
    aircraft_sprite->pushRotateZoom(layer, ac.x, ac.y, ac.heading, 1.0, 1.0);
  }
}