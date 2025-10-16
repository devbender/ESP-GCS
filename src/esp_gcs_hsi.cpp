#include "esp_gcs_hsi.h"

ESP_GCS_HSI::ESP_GCS_HSI() { }

ESP_GCS_HSI::ESP_GCS_HSI(esp_gcs_config_t* config) {
    init(config);
};



void ESP_GCS_HSI::init_base_layer(void) {  

  // Background
  base_layer.fillScreen(COLOR_SKY);

  base_layer.setTextSize(2);
  base_layer.setTextColor(COLOR_RED);  
  base_layer.drawCenterString("HSI", base_layer.width()/2, base_layer.height()/2 - 120);
}


void ESP_GCS_HSI::init_top_layer(void) {

  hsi.setColorDepth(4);  
  set_palette_4bit(&hsi);

  hsi.createSprite(fb_width, fb_height);
  hsi.fillSprite(COLOR_TRANSPARENT);
}




void ESP_GCS_HSI::init(esp_gcs_config_t* config) {
  init_fb(config);
  init_base_layer();
  init_top_layer();

  //render(0.00, 0.00);
  base_layer.pushRotateZoom(&top_layer, 0, 1.00,1.00);  
  init_top_layer();
  top_layer.pushSprite(0, 0);
  
  //datalink.init(config);

  log_d("Free RAM: %.2f KB\n", ESP.getFreeHeap() / 1024.0);
}


void ESP_GCS_HSI::render() {
    top_layer.pushSprite(0, 0);
}