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
    
    log_i("Initializing lcd...");    
    
    if (!lcd->init()) {
        log_e("lcd->init() failed");
        return false;
    }    
    log_i("lcd->init() OK");
    
    lcd->initDMA();
    lcd->setRotation(config.rotation);
    
    lcd->fillScreen(TFT_BLACK);
    lcd->setTextDatum(MC_DATUM); // set datum to middle center
    lcd->setTextColor(TFT_WHITE);
    lcd->setTextSize(2);
    lcd->drawString("ESP GCS v0.1a", lcd->width()/2, lcd->height()/2);
    
    lcd->setTextSize(1); // reset text size
    lcd->setTextDatum(TL_DATUM); // reset datum    
    
    delay(2000);
    
    lcd->startWrite(); // Begin DMA transaction    
    initialized = true;    
    log_i("DisplayDevice fully initialized");    

    return true;
}