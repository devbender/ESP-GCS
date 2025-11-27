#include "esp_gcs_hal_display_device.h"

DisplayDevice::DisplayDevice(const DisplayConfig& cfg) 
    : config(cfg), initialized(false) {
    lcd.reset(new LGFX_Parallel_9488());
}

DisplayDevice::~DisplayDevice() {
    if (initialized && lcd) {
        lcd->endWrite();
    }
}

bool DisplayDevice::init() {
    if (initialized) return true;
    
    if (!lcd) {
        log_e("LCD object is null");
        return false;
    }
    
    // Configure the panel before initializing
    pinMode(config.pin_cs, OUTPUT);
    pinMode(config.pin_blk, OUTPUT);

    digitalWrite(config.pin_cs, LOW);
    digitalWrite(config.pin_blk, HIGH);

    
    log_i("Calling lcd->init()...");
    if (!lcd->init()) {
        log_e("lcd->init() failed");
        return false;
    }
    log_i("lcd->init() succeeded");

    log_i("Calling lcd->initDMA()...");
    lcd->initDMA();
    log_i("lcd->initDMA() done");
    
    lcd->setRotation(config.rotation);
    lcd->setBrightness(255);
    
    // Clear screen to verify it's working
    lcd->fillScreen(TFT_RED);   delay(200);
    lcd->fillScreen(TFT_GREEN); delay(200);
    lcd->fillScreen(TFT_BLUE);  delay(200);    
    
    lcd->fillScreen(TFT_BLACK);
    lcd->setTextDatum(MC_DATUM); // set datum to middle center
    lcd->drawString("DISPLAY INITIALIZED", lcd->width()/2, lcd->height()/2);
    lcd->setTextDatum(TL_DATUM); // reset datum
    delay(1500);
    
    lcd->startWrite(); // Begin DMA transaction
    
    initialized = true;
    log_i("DisplayDevice fully initialized");
    return true;
}