#include "esp_gcs_adsb.h"

ESP_GCS_ADSB::ESP_GCS_ADSB() { }

ESP_GCS_ADSB::ESP_GCS_ADSB(esp_gcs_config_t* config) {
    init(config);
};



void ESP_GCS_ADSB::init_base_layer(void) {  

  // Background
  base_layer.fillScreen(UI_BACKGROUND_COLOR);

  base_layer.setTextSize(2);
  base_layer.setTextColor(COLOR_RED);  
  base_layer.drawCenterString("ADSB", base_layer.width()/2, base_layer.height()/2 - 120);
}


void ESP_GCS_ADSB::init_top_layer(void) {

  adsb.setColorDepth(4);  
  set_palette_4bit(&adsb);

  adsb.createSprite(fb_width, fb_height);
  adsb.fillSprite(COLOR_TRANSPARENT);
}




void ESP_GCS_ADSB::init(esp_gcs_config_t* config) {
  init_fb(config);
  init_base_layer();
  init_top_layer();

  //render(0.00, 0.00);
  base_layer.pushRotateZoom(&top_layer, 0, 1.00,1.00);  
  init_top_layer();
  //top_layer.pushSprite(0, 0);

  render_top_layer();
  
  //datalink.init(config);

  log_d("Free RAM: %.2f KB\n", ESP.getFreeHeap() / 1024.0);
}


void ESP_GCS_ADSB::render_top_layer() {
    //top_layer.pushSprite(0, 0);

    	// Background
	top_layer.fillScreen(UI_BACKGROUND_COLOR);	
		
	// Top Black Bar
	top_layer.fillRect(0,0, top_layer.width(), 30, UI_TOP_BAR_COLOR);	
	top_layer.drawFastHLine(0, 30, top_layer.width(), UI_TOP_BAR_DIV_LINE_COLOR);

	// Top bar text
	//top_layer.setFont();
	top_layer.setTextSize(1.5); top_layer.setTextColor(COLOR_WHITE);
	top_layer.setCursor(10, 12); top_layer.print("ESP-GCS");

	// Restore Default Font
	//top_layer.setFont();
	top_layer.setTextSize(1);
	top_layer.setTextColor(COLOR_WHITE);


    #define tft top_layer

    // Traffic drawing area
	top_layer.fillCircle(top_layer.width()/2, top_layer.height()/2, top_layer.height()/2-1, COLOR_BLACK);
    tft.drawCircle(tft.width()/2,  tft.height()/2, tft.height()/2-1, UI_OUTER_CIRCLE_COLOR);

    int offset = 12;	
	tft.setTextSize(2); tft.setTextColor(COLOR_WHITE);


    // Inner Circle Params
	int degMark = 15;
	int ri = tft.height() / 4;
	
    #define TFT_X_CENTER tft.width()/2
    #define TFT_Y_CENTER tft.height()/2

	// Draw inner circle
	for (int i = 0; i<360; i += degMark) {			
		float a = radians(i);
		tft.drawPixel(TFT_X_CENTER + cos(a)*ri, TFT_Y_CENTER + sin(a)*ri, UI_INNER_CIRCLE_COLOR);
    }

    #define AIRCRAFT_SIZE	6
    #define VECTOR_SIZE 	20
    #define AIRPORT_SIZE	3
    #define AIRCRAFT_TTL 	30

    tft.drawLine(TFT_X_CENTER-AIRCRAFT_SIZE,	TFT_Y_CENTER+AIRCRAFT_SIZE, TFT_X_CENTER,				TFT_Y_CENTER-AIRCRAFT_SIZE, UI_MY_AIRCRAFT_COLOR);
    tft.drawLine(TFT_X_CENTER,					TFT_Y_CENTER-AIRCRAFT_SIZE, TFT_X_CENTER+AIRCRAFT_SIZE,	TFT_Y_CENTER+AIRCRAFT_SIZE, UI_MY_AIRCRAFT_COLOR);		
    tft.drawLine(TFT_X_CENTER-AIRCRAFT_SIZE,	TFT_Y_CENTER+AIRCRAFT_SIZE, TFT_X_CENTER,				TFT_Y_CENTER+3, 			UI_MY_AIRCRAFT_COLOR);
    tft.drawLine(TFT_X_CENTER+AIRCRAFT_SIZE,	TFT_Y_CENTER+AIRCRAFT_SIZE, TFT_X_CENTER,				TFT_Y_CENTER+3, 			UI_MY_AIRCRAFT_COLOR);
    
}

void ESP_GCS_ADSB::render() {
    top_layer.pushSprite(0, 0);    
}