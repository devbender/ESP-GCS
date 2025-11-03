#include "esp_gcs_adsb.h"



ESP_GCS_ADSB::ESP_GCS_ADSB() { 
  init();  
  render_ui_layer(&fb_1);

  render_aircraft(COLOR_RED);
  aircraft.pushRotateZoom(&fb_1, fb_1.width()/2, fb_1.height()/2, 0, 1.0, 1.0);

  
  render_aircraft(COLOR_WHITE);  
  fb_1.pushRotateZoom(&fb_0, 0, 1.0,1.0);
  aircraft.pushRotateZoom(&fb_0, 130, 100, 0, 1.0, 1.0);

  log_d("ADS-B Free RAM: %.2f KB\n", ESP.getFreeHeap() / 1024.0);
}

void ESP_GCS_ADSB::render_aircraft(uint16_t color) {
  cfg_sp(&aircraft, AIRCRAFT_SIZE*2, AIRCRAFT_SIZE*2);

  aircraft.fillTriangle(0, AIRCRAFT_SIZE*2-1,
                        AIRCRAFT_SIZE, 0,
                        AIRCRAFT_SIZE*2-1, AIRCRAFT_SIZE*2-1,
                        color);

  aircraft.fillTriangle(AIRCRAFT_SIZE, AIRCRAFT_SIZE*2-3,
                        AIRCRAFT_SIZE-4, AIRCRAFT_SIZE*2-1,
                        AIRCRAFT_SIZE+4, AIRCRAFT_SIZE*2-1,
                        COLOR_TRANSPARENT);
}

void test() {


}


void ESP_GCS_ADSB::render_ui_layer(LGFX_Sprite *layer) {
  
  // Background
	layer->fillScreen(UI_BACKGROUND_COLOR);	
		
	// Top Black Bar
	layer->fillRect(0,0, layer->width(), 30, UI_TOP_BAR_COLOR);	
	layer->drawFastHLine(0, 30, layer->width(), UI_TOP_BAR_DIV_LINE_COLOR);

	// Top bar text	
	layer->setTextSize(1.5); layer->setTextColor(COLOR_WHITE);
	layer->setCursor(10, 12); layer->print("ESP-GCS");

	// Restore Default Font	
	layer->setTextSize(1);
	layer->setTextColor(COLOR_WHITE);

  // Traffic drawing area
	layer->fillCircle(layer->width()/2, layer->height()/2, layer->height()/2-1, COLOR_BLACK);
  layer->drawCircle(layer->width()/2,  layer->height()/2, layer->height()/2-1, UI_OUTER_CIRCLE_COLOR); 


    // Inner Circle Params
	int degMark = 15;
	int ri = layer->height() / 4;
	
  #define TFT_X_CENTER layer->width()/2
  #define TFT_Y_CENTER layer->height()/2

	// Draw inner circle
	for (int i = 0; i<360; i += degMark) {			
		float a = radians(i);
		layer->drawPixel(TFT_X_CENTER + cos(a)*ri, TFT_Y_CENTER + sin(a)*ri, COLOR_RED); //UI_INNER_CIRCLE_COLOR);
  }
  
  // layer->drawLine(TFT_X_CENTER-AIRCRAFT_SIZE,	TFT_Y_CENTER+AIRCRAFT_SIZE, 
  //                 TFT_X_CENTER,				        TFT_Y_CENTER-AIRCRAFT_SIZE, 
  //                 UI_MY_AIRCRAFT_COLOR);
  
  
  // layer->drawLine(TFT_X_CENTER,               TFT_Y_CENTER-AIRCRAFT_SIZE, 
  //                 TFT_X_CENTER+AIRCRAFT_SIZE,	TFT_Y_CENTER+AIRCRAFT_SIZE, 
  //                 UI_MY_AIRCRAFT_COLOR);		
  
  
  // layer->drawLine(TFT_X_CENTER-AIRCRAFT_SIZE,	TFT_Y_CENTER+AIRCRAFT_SIZE, 
  //                 TFT_X_CENTER,				        TFT_Y_CENTER+3, 			      
  //                 UI_MY_AIRCRAFT_COLOR);
  
  
  // layer->drawLine(TFT_X_CENTER+AIRCRAFT_SIZE,	TFT_Y_CENTER+AIRCRAFT_SIZE, 
  //                 TFT_X_CENTER,				        TFT_Y_CENTER+3, 			      
  //                 UI_MY_AIRCRAFT_COLOR);
    
}



void ESP_GCS_ADSB::render_fb() {
    fb_0.pushSprite(0, 0);
}