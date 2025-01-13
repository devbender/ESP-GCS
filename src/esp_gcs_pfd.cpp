#include "esp_gcs_pfd.h"

                        
ESP_GCS_PFD::ESP_GCS_PFD() {};

void ESP_GCS_PFD::init(esp_gcs_config_t* config) {
  init_fb(config);
  init_base_layer();
  init_top_layer();

  //render(0.00, 0.00);
  base_layer.pushRotateZoom(&top_layer, 0, 1.2,1.2);  
  init_top_layer();
  top_layer.pushSprite(0, 0);
  
  datalink.init(config);

  // set filtering params
  pitch_filter.setSamples(10);
  pitch_filter.setWAParams(0.10, 0.5, 0.85);

  roll_filter.setSamples(10);
  roll_filter.setWAParams(0.10, 0.5, 0.85);

  log_d("Free RAM: %.2f KB\n", ESP.getFreeHeap() / 1024.0);
}



void ESP_GCS_PFD::init_base_layer(void) {  

  // Render Background
  base_layer.fillRect(0,0,                  fb_width, fb_height/2, COLOR_SKY);   // SKY
  base_layer.fillRect(0,fb_height/2,        fb_width, fb_height/2, COLOR_GND);   // GROUND    
  base_layer.drawLine(0, fb_height/2, fb_width, fb_height/2, COLOR_WHITE);       // HORIZON  
  
  render_pitch_scale(&base_layer, 0);

  precalculate_roll_ref_lines();
  render_turn_indicator(&base_layer);

  // red lines
  //base_layer.drawWideLine(0,0, base_layer.width(), base_layer.height(), 2, COLOR_RED);
  //base_layer.drawWideLine(base_layer.width(),0, 0, base_layer.height(), 2, COLOR_RED);

  base_layer.drawLine(0,0, base_layer.width(), base_layer.height(), COLOR_RED);
  base_layer.drawLine(base_layer.width(),0, 0, base_layer.height(), COLOR_RED);

  // data aqc
  base_layer.setTextSize(2);
  base_layer.setTextColor(COLOR_RED);  
  base_layer.drawCenterString("NO DATA", base_layer.width()/2, base_layer.height()/2 - 120);
}


void ESP_GCS_PFD::init_top_layer(void) {

  // Left Level ----------------------------------    
  #define RWH 25,7    
  #define TV1 25,0
  #define TV2 29,3
  #define TV3 25,6

  lvl1.setColorDepth(4);
  set_palette_4bit(&lvl1);
  lvl1.createSprite(30, 7);
  lvl1.fillSprite(COLOR_TRANSPARENT);
  
  lvl1.fillRect(0,0, RWH, COLOR_YELLOW);
  lvl1.fillTriangle(TV1, TV2, TV3, COLOR_YELLOW);

  lvl1.drawLine(0,0, TV1, COLOR_BLACK);
  lvl1.drawLine(0,0, 0,7, COLOR_BLACK);
  lvl1.drawLine(0,6, TV3, COLOR_BLACK);    
  
  lvl1.drawLine(TV1, TV2, COLOR_BLACK);
  lvl1.drawLine(TV2, TV3, COLOR_BLACK);


  // Right Level ----------------------------------
  #define RWH_2 25,8
  #define TV1_2 5,0
  #define TV2_2 0,3
  #define TV3_2 5,6

  lvl2.setColorDepth(4);
  set_palette_4bit(&lvl2);
  lvl2.createSprite(30, 7);
  lvl2.fillSprite(COLOR_TRANSPARENT);
  
  lvl2.fillRect(5,0, RWH_2, COLOR_YELLOW);
  lvl2.fillTriangle(TV1_2, TV2_2, TV3_2, COLOR_YELLOW);

  lvl2.drawLine(29,0, TV1_2, COLOR_BLACK);    
  lvl2.drawLine(29,0, 29,7, COLOR_BLACK);
  lvl2.drawLine(29,6, TV3_2, COLOR_BLACK);
  
  lvl2.drawLine(TV1_2, TV2_2, COLOR_BLACK);
  lvl2.drawLine(TV2_2, TV3_2, COLOR_BLACK);


  // Aircraft -------------------------------------
  #define NOSE    35,0
  #define CENTER  35,7
  #define TAIL    35,12
  #define LWING   0,21
  #define RWING   70,21

  ai.setColorDepth(4);
  set_palette_4bit(&ai);
  ai.createSprite(70, 21);
  ai.fillSprite(COLOR_TRANSPARENT);
  
  //  Triangles
  ai.fillTriangle(NOSE, TAIL, LWING, COLOR_YELLOW);
  ai.fillTriangle(NOSE, TAIL, RWING, COLOR_YELLOW);
  
  // Aircraft Outlines
  ai.drawLine(NOSE, LWING, COLOR_BLACK);
  ai.drawLine(NOSE, RWING, COLOR_BLACK);

  ai.drawLine(LWING, TAIL, COLOR_BLACK);
  ai.drawLine(RWING, TAIL, COLOR_BLACK);

  // Internal black Lines
  ai.drawLine(NOSE, CENTER, COLOR_BLACK);
  ai.drawLine(CENTER, LWING, COLOR_BLACK);
  ai.drawLine(CENTER, RWING, COLOR_BLACK);

  // Turn Angle Indicator Sprite -------------------
  ti.setColorDepth(4);
  set_palette_4bit(&ti);
  ti.createSprite(10, 10);
  ti.fillSprite(COLOR_TRANSPARENT);
  ti.fillTriangle(5,0, 1,8, 9,8, COLOR_WHITE);
  ti.drawTriangle(5,0, 1,8, 9,8, COLOR_WHITE);


  // aircraft indicator
  ai_x = base_layer.width()/2 - ai.width()/2;
  ai_y = base_layer.height()/2;
  ai.setPivot(base_layer.width()/2 - ai_x, base_layer.height()/2 - ai_y);

  ai.pushRotateZoom(&top_layer, 0, aircraft_indicator_size,aircraft_indicator_size, COLOR_TRANSPARENT);

  // left level  
  lvl1_x = base_layer.width()/2 - lvl1.width()/2 - level_offset;
  lvl1_y = base_layer.height()/2 - lvl1.height()/2;
  lvl1.setPivot(base_layer.width()/2 - lvl1_x, base_layer.height()/2 - lvl1_y);

  lvl1.pushRotateZoom(&top_layer, 0, aircraft_level_size,aircraft_level_size, COLOR_TRANSPARENT);

  // right level 
  lvl2_x = base_layer.width()/2 - lvl2.width()/2 + level_offset;
  lvl2_y = base_layer.height()/2 - lvl2.height()/2;
  lvl2.setPivot(base_layer.width()/2 - lvl2_x, base_layer.height()/2 - lvl2_y);
  
  lvl2.pushRotateZoom(&top_layer, 0, aircraft_level_size,aircraft_level_size, COLOR_TRANSPARENT);


  // top turn indicator
  ti_x = base_layer.width()/2 - ti.width()/2;
  ti_y = base_layer.height()/2 - ti.height()/2 - indicator_rad - 10;
  ti.setPivot(base_layer.width()/2 - ti_x, base_layer.height()/2 - ti_y);
  
  ti.pushRotateZoom(&top_layer, 0, 1,1, COLOR_TRANSPARENT);
}
  

void ESP_GCS_PFD::render(float pitch, float roll) {

  float inPitch0 = degrees( pitch );
  float inRoll0 = -degrees( roll );

  log_d("%.2f    %.2f", inPitch0, inRoll0);
  
  base_layer.pushRotateZoom(&top_layer, inRoll0, 1.2,1.2);  

  int ind_w = 80;
  //top_layer.fillRectAlpha(0, 0, ind_w, fb_height, 128, COLOR_BLACK);
  //top_layer.fillRectAlpha(fb_width - ind_w, 0, ind_w, fb_height, 128, COLOR_BLACK);

  top_layer.pushSprite(0, 0);
}




void ESP_GCS_PFD::render_base_layer(mavlink_attitude_t atti) {

  float inPitch0 = degrees(atti.pitch) * 4;
  pitch_filter.add( inPitch0 );
  float inPitch1 = pitch_filter.mavg();
  int inPitch = round( inPitch1 );

  // Render Background
  base_layer.fillRect(0,0,                    fb_width, fb_height/2+inPitch, COLOR_SKY);   // SKY
  base_layer.fillRect(0,fb_height/2+inPitch,  fb_width, fb_height/2-inPitch, COLOR_GND);   // GROUND    
  base_layer.drawLine(0, fb_height/2+inPitch, fb_width, fb_height/2+inPitch, COLOR_WHITE);       // HORIZON

  render_pitch_scale(&base_layer, inPitch);
  render_turn_indicator(&base_layer);  
}


void ESP_GCS_PFD::render_top_layer(mavlink_attitude_t atti) {
  
  float inRoll0 = -degrees( atti.roll );
  roll_filter.add( inRoll0 );
  float inRoll1 = roll_filter.mavg() ; //.wavg();
  int inRoll = round( inRoll1 );

  base_layer.pushRotateZoom(&top_layer, inRoll, 1.2,1.2);  

  ai.pushRotateZoom(&top_layer, 0, aircraft_indicator_size,aircraft_indicator_size, COLOR_TRANSPARENT);
  lvl1.pushRotateZoom(&top_layer, 0, aircraft_level_size,aircraft_level_size, COLOR_TRANSPARENT);
  lvl2.pushRotateZoom(&top_layer, 0, aircraft_level_size,aircraft_level_size, COLOR_TRANSPARENT);
  ti.pushRotateZoom(&top_layer, 0, 1,1, COLOR_TRANSPARENT);

  top_layer.pushSprite(0, 0);
}


void ESP_GCS_PFD::render_all(mavlink_heartbeat_t hb, mavlink_attitude_t atti, mavlink_vfr_hud_t hud) {
  render_base_layer(atti);
  render_top_layer(atti);
}







void ESP_GCS_PFD::init2(esp_gcs_config_t* config) {

  init_fb(config);

  spd.setColorDepth(8);
  spd.createSprite(50, 200);
  //spd.fillRectAlpha(60, 200, 30, 150, 35, TFT_BLACK);
  spd.drawSmoothLine(0, 20, spd.width(), 20, TFT_WHITE);




  // Left Level ----------------------------------    
  #define RWH 25,7    
  #define TV1 25,0
  #define TV2 29,3
  #define TV3 25,6

  #define BG_COLOR 0xEE4C //0xC660 //0xC680
  #define OL_COLOR TFT_BLACK

  lvl1.setColorDepth(8);
  lvl1.createSprite(30, 7);
  lvl1.fillSprite(TFT_TRANSPARENT);
  
  lvl1.fillRect(0,0, RWH, BG_COLOR);
  lvl1.fillTriangle(TV1, TV2, TV3, BG_COLOR);

  lvl1.drawSmoothLine(0,0, TV1, OL_COLOR);
  lvl1.drawSmoothLine(0,0, 0,7, OL_COLOR);
  lvl1.drawSmoothLine(0,6, TV3, OL_COLOR);    
  
  lvl1.drawSmoothLine(TV1, TV2, OL_COLOR);
  lvl1.drawSmoothLine(TV2, TV3, OL_COLOR);


  // Right Level ----------------------------------
  #define RWH_2 25,8
  #define TV1_2 5,0
  #define TV2_2 0,3
  #define TV3_2 5,6

  lvl2.setColorDepth(8);
  lvl2.createSprite(30, 7);
  lvl2.fillSprite(TFT_TRANSPARENT);
  
  lvl2.fillRect(5,0, RWH_2, BG_COLOR);
  lvl2.fillTriangle(TV1_2, TV2_2, TV3_2, BG_COLOR);

  lvl2.drawSmoothLine(29,0, TV1_2, OL_COLOR);    
  lvl2.drawSmoothLine(29,0, 29,7, OL_COLOR);
  lvl2.drawSmoothLine(29,6, TV3_2, OL_COLOR);
  
  lvl2.drawSmoothLine(TV1_2, TV2_2, OL_COLOR);
  lvl2.drawSmoothLine(TV2_2, TV3_2, OL_COLOR);


  // Aircraft -------------------------------------
  #define NOSE    35,0
  #define CENTER  35,7
  #define TAIL    35,12
  #define LWING   0,21
  #define RWING   70,21

  ai.setColorDepth(8);
  ai.createSprite(70, 21);
  ai.fillSprite(TFT_TRANSPARENT);
  
  //  Triangles
  ai.fillTriangle(NOSE, TAIL, LWING, BG_COLOR);
  ai.fillTriangle(NOSE, TAIL, RWING, BG_COLOR);
  
  // Aircraft Outlines
  ai.drawSmoothLine(NOSE, LWING, TFT_BLACK);
  ai.drawSmoothLine(NOSE, RWING, TFT_BLACK);

  ai.drawSmoothLine(LWING, TAIL, TFT_BLACK);
  ai.drawSmoothLine(RWING, TAIL, TFT_BLACK);

  // Internal black Lines
  ai.drawSmoothLine(NOSE, CENTER, TFT_BLACK);
  ai.drawSmoothLine(CENTER, LWING, TFT_BLACK);
  ai.drawSmoothLine(CENTER, RWING, TFT_BLACK);
  
 

  // Turn Angle Indicator Sprite -------------------
  ti.setColorDepth(8);  
  ti.createSprite(10, 10);
  ti.fillSprite(TFT_TRANSPARENT);
  ti.fillTriangle(5,0, 1,8, 9,8, TFT_WHITE);
  ti.drawTriangle(5,0, 1,8, 9,8, TFT_WHITE);


  precalculate_roll_ref_lines();

  // Render Background
  base_layer.fillRect(0,0,                  fb_width, fb_height/2, COLOR_SKY);   // SKY
  base_layer.fillRect(0,fb_height/2,        fb_width, fb_height/2, COLOR_GND);   // GROUND  
  base_layer.drawSmoothLine(0, fb_height/2, fb_width, fb_height/2, COLOR_WHITE); // HORIZON


  //render_pitch_scale(&fb_layer_0, 0);
  //render_turn_indicator(&fb_layer_0);
  //render_aircraft(&fb_layer_0, 0);


  // red lines
  base_layer.drawWideLine(0,0, base_layer.width(), base_layer.height(), 2, TFT_RED);
  base_layer.drawWideLine(base_layer.width(),0, 0, base_layer.height(), 2, TFT_RED);

  // data aqc
  base_layer.setTextSize(2);
  base_layer.setTextColor(TFT_RED);  
  base_layer.drawCenterString("NO DATA", base_layer.width()/2, base_layer.height()/2 - 120);



  // push frame_buffer to LCD
  //frame_buffer.pushRotateZoom(0, 1,1);
  base_layer.pushRotateZoom(0, 1.2,1.2);



  // set filtering params
  pitch_filter.setSamples(5);
  pitch_filter.setWAParams(0.10, 0.5, 0.85);

  roll_filter.setSamples(5);
  roll_filter.setWAParams(0.10, 0.5, 0.85);


  delay(2500);
  datalink.init(config);

} // end init()



void ESP_GCS_PFD::render(int pitch, int roll, bool t) {

  // Render Background
  base_layer.fillRect(0,0,                    fb_width, fb_height/2, TFT_SKY);   // SKY
  base_layer.fillRect(0,fb_height/2,          fb_width, fb_height/2, TFT_GND);   // GROUND
  base_layer.drawFastHLine(0, fb_height/2,    fb_width, TFT_WHITE);            // HORIZON

  base_layer.pushSprite(fb_center_x - 50, fb_center_y);
  base_layer.pushSprite(fb_center_x + 50, fb_center_y);
}



void ESP_GCS_PFD::render2(mavlink_heartbeat_t hb, mavlink_attitude_t atti, mavlink_vfr_hud_t hud) {

  #define PITCH_ANGLE_TO_PX 1

  // ALTITUDE UNITS CONVERSION
  #define M_TO_FT 3.28084
  #define ALT_UNIT_CONV M_TO_FT

  // SPEED UNITS CONVERSION  
  #define MPS_TO_KPH  3.6
  #define MPS_TO_KTS  1.94384
  #define MPS_TO_MPH  2.23694
  #define SPEED_UNIT_CONV MPS_TO_KTS

  int inAlt = round( hud.alt * ALT_UNIT_CONV );  
  int inSpeed = round( hud.groundspeed * SPEED_UNIT_CONV );

  // filters
  float inPitch0 = degrees(atti.pitch);
  float inRoll0 = -degrees(atti.roll);

  // dont draw if no data
  if( inPitch0 == 0.00 && inRoll0 == 0.00) return;

  pitch_filter.add( inPitch0  );
  roll_filter.add( inRoll0 );

  float inPitch1 = pitch_filter.wavg();
  float inRoll1 = roll_filter.wavg();

  int inPitch = round( inPitch1 );
  int inRoll = round( inRoll1 );

  log_d("MEM:%.2f KB    P:%.2f    FP:%.2f    R:%.2f    FR:%.2f", ESP.getFreeHeap() / 1024.0, inPitch0, inPitch1, inRoll0, inRoll1);

  // Render Background
  base_layer.fillRect(0,0,                    fb_width, fb_height/2+inPitch, TFT_SKY);   // SKY
  base_layer.fillRect(0,fb_height/2+inPitch,  fb_width, fb_height/2-inPitch, TFT_GND);   // GROUND  
  base_layer.drawSmoothLine(0, fb_height/2+inPitch,  fb_width, fb_height/2+inPitch, TFT_WHITE);            // HORIZON
  

  render_pitch_scale(&base_layer, inPitch);
  render_turn_indicator(&base_layer);
  render_aircraft(&base_layer, inRoll);
  //render_side_indicators(&frame_buffer, inRoll, inAlt, inSpeed);


  base_layer.pushRotateZoom(inRoll, framebuffer_zoom,framebuffer_zoom);
  //frame_buffer.pushRotateZoom(&final_buffer, 0, 1.2,1.2);

  // lcd.fillRectAlpha(90, 100, 50, 150, 100, TFT_BLACK);
  // lcd.fillRectAlpha(340, 100, 50, 150, 100, TFT_BLACK);

  

  // int ind_w = 80;
  // lcd.fillRectAlpha(0, 0, ind_w, lcd.height(), 128, TFT_BLACK);
  // lcd.fillRectAlpha(lcd.width() - ind_w, 0, ind_w, lcd.height(), 128, TFT_BLACK);

  // int ind_w = 80;
  // final_buffer.fillRectAlpha(0, 0, ind_w, lcd.height(), 128, TFT_BLACK);
  // final_buffer.fillRectAlpha(lcd.width() - ind_w, 0, ind_w, lcd.height(), 128, TFT_BLACK);

  //final_buffer.pushSprite(&lcd, 0,0);
  //final_buffer.pushSprite(fb_center_x, fb_center_y);

}


void ESP_GCS_PFD::render_side_indicators(LGFX_Sprite *sp, int inRoll, int input_altitude, int input_speed) {
  uint8_t alpha_transp = 100;

  int offcenter = 150;
  spd.fillRectAlpha(0, 0,  spd.width(), spd.height(), 128, TFT_BLACK);
  spd.drawRect(0,0, spd.width(), spd.height(), TFT_WHITE);
  
  int spd_x = base_layer.width()/2 - spd.width()/2 - offcenter;
  int spd_y = base_layer.height()/2 - spd.height()/2;
  spd.setPivot(base_layer.width()/2 - spd_x, base_layer.height()/2 - spd_y);
  
  spd.pushRotateZoomWithAA(&base_layer, -inRoll, side_indicators_size,side_indicators_size, TFT_TRANSPARENT);
}




void ESP_GCS_PFD::render_aircraft(LGFX_Sprite *sp, int inRoll) {

  // aircraft
  int ai_x = base_layer.width()/2 - ai.width()/2;
  int ai_y = base_layer.height()/2;
  ai.setPivot(base_layer.width()/2 - ai_x, base_layer.height()/2 - ai_y);
  ai.pushRotateZoomWithAA(&base_layer, -inRoll, aircraft_indicator_size,aircraft_indicator_size, TFT_TRANSPARENT);

  // left level  
  int lvl1_x = base_layer.width()/2 - lvl1.width()/2 - level_offset;
  int lvl1_y = base_layer.height()/2 - lvl1.height()/2;
  lvl1.setPivot(base_layer.width()/2 - lvl1_x, base_layer.height()/2 - lvl1_y);
  lvl1.pushRotateZoomWithAA(&base_layer, -inRoll, aircraft_level_size,aircraft_level_size, TFT_TRANSPARENT);

  // right level 
  int lvl2_x = base_layer.width()/2 - lvl2.width()/2 + level_offset;
  int lvl2_y = base_layer.height()/2 - lvl2.height()/2;
  lvl2.setPivot(base_layer.width()/2 - lvl2_x, base_layer.height()/2 - lvl2_y);
  lvl2.pushRotateZoomWithAA(&base_layer, -inRoll, aircraft_level_size,aircraft_level_size, TFT_TRANSPARENT);
  
  // top turn indicator
  int ti_x = base_layer.width()/2 - ti.width()/2;
  int ti_y = base_layer.height()/2 - ti.height()/2 - indicator_rad + 8;
  ti.setPivot(base_layer.width()/2 - ti_x, base_layer.height()/2 - ti_y);
  ti.pushRotateZoomWithAA(&base_layer, -inRoll, 1,1, TFT_TRANSPARENT);
}


void ESP_GCS_PFD::precalculate_roll_ref_lines() {

  // Turn indicator arc  
  int cX = base_layer.width()/2;   //160;
  int cY = base_layer.height()/2;  //80;

  int S = 7;
  int L = 13;
  
  for(float i=-60; i<=60; i+=5) {
    int x0 = cX + cos( radians(i-90) ) * indicator_rad;
    int y0 = cY + sin( radians(i-90) ) * indicator_rad;
    
    int x11 = cX + cos( radians(i-90) ) * (indicator_rad+S);
    int y11 = cY + sin( radians(i-90) ) * (indicator_rad+S);
    
    int x12 = cX + cos( radians(i-90) ) * (indicator_rad+L);
    int y12 = cY + sin( radians(i-90) ) * (indicator_rad+L);

    //=============================
    if(i==-10) {
      roll_ref_lines[0][0] = x0;
      roll_ref_lines[0][1] = y0;
      roll_ref_lines[0][2] = x11;
      roll_ref_lines[0][3] = y11;
    }
    
    if(i==-20) {
      roll_ref_lines[1][0] = x0;
      roll_ref_lines[1][1] = y0;
      roll_ref_lines[1][2] = x11;
      roll_ref_lines[1][3] = y11;
    }
    
    if(i==-30) {
      roll_ref_lines[2][0] = x0;
      roll_ref_lines[2][1] = y0;
      roll_ref_lines[2][2] = x12;
      roll_ref_lines[2][3] = y12;
    }
    
    if(i==-45) {
      roll_ref_lines[3][0] = x0; 
      roll_ref_lines[3][1] = y0; 
      roll_ref_lines[3][2] = x11;
      roll_ref_lines[3][3] = y11;
    }
    
    if(i==-60) {
      roll_ref_lines[4][0] = x0; 
      roll_ref_lines[4][1] = y0; 
      roll_ref_lines[4][2] = x12;
      roll_ref_lines[4][3] = y12;
    }


    //=============================
    if(i==10) {
      roll_ref_lines[5][0] = x0;
      roll_ref_lines[5][1] = y0;
      roll_ref_lines[5][2] = x11;
      roll_ref_lines[5][3] = y11;
    }
    
    if(i==20) {
      roll_ref_lines[6][0] = x0;
      roll_ref_lines[6][1] = y0;
      roll_ref_lines[6][2] = x11;
      roll_ref_lines[6][3] = y11;
    }
    
    if(i==30) {
      roll_ref_lines[7][0] = x0; 
      roll_ref_lines[7][1] = y0; 
      roll_ref_lines[7][2] = x12;
      roll_ref_lines[7][3] = y12;
    }
    
    if(i==45) {
      roll_ref_lines[8][0] = x0; 
      roll_ref_lines[8][1] = y0; 
      roll_ref_lines[8][2] = x11;
      roll_ref_lines[8][3] = y11;
    }
    
    if(i==60) {
      roll_ref_lines[9][0] = x0; 
      roll_ref_lines[9][1] = y0; 
      roll_ref_lines[9][2] = x12;
      roll_ref_lines[9][3] = y12;
    }    
  }
}



void ESP_GCS_PFD::render_turn_indicator(LGFX_Sprite *sp) {  

  // // Roll Angle Reference Lines (LEFT)
  // sp->drawSmoothLine(roll_ref_lines[0][0], roll_ref_lines[0][1], roll_ref_lines[0][2], roll_ref_lines[0][3], COLOR_WHITE);  
  // sp->drawSmoothLine(roll_ref_lines[1][0], roll_ref_lines[1][1], roll_ref_lines[1][2], roll_ref_lines[1][3], COLOR_WHITE);
  // sp->drawSmoothLine(roll_ref_lines[2][0], roll_ref_lines[2][1], roll_ref_lines[2][2], roll_ref_lines[2][3], COLOR_WHITE);
  // sp->drawSmoothLine(roll_ref_lines[3][0], roll_ref_lines[3][1], roll_ref_lines[3][2], roll_ref_lines[3][3], COLOR_WHITE);
  // sp->drawSmoothLine(roll_ref_lines[4][0], roll_ref_lines[4][1], roll_ref_lines[4][2], roll_ref_lines[4][3], COLOR_WHITE);

  // // Roll Angle Reference Lines (RIGHT)
  // sp->drawSmoothLine(roll_ref_lines[5][0], roll_ref_lines[5][1], roll_ref_lines[5][2], roll_ref_lines[5][3], COLOR_WHITE);
  // sp->drawSmoothLine(roll_ref_lines[6][0], roll_ref_lines[6][1], roll_ref_lines[6][2], roll_ref_lines[6][3], COLOR_WHITE);
  // sp->drawSmoothLine(roll_ref_lines[7][0], roll_ref_lines[7][1], roll_ref_lines[7][2], roll_ref_lines[7][3], COLOR_WHITE);
  // sp->drawSmoothLine(roll_ref_lines[8][0], roll_ref_lines[8][1], roll_ref_lines[8][2], roll_ref_lines[8][3], COLOR_WHITE);
  // sp->drawSmoothLine(roll_ref_lines[9][0], roll_ref_lines[9][1], roll_ref_lines[9][2], roll_ref_lines[9][3], COLOR_WHITE);



  // Roll Angle Reference Lines (LEFT)
  sp->drawLine(roll_ref_lines[0][0], roll_ref_lines[0][1], roll_ref_lines[0][2], roll_ref_lines[0][3], COLOR_WHITE);  
  sp->drawLine(roll_ref_lines[1][0], roll_ref_lines[1][1], roll_ref_lines[1][2], roll_ref_lines[1][3], COLOR_WHITE);
  sp->drawLine(roll_ref_lines[2][0], roll_ref_lines[2][1], roll_ref_lines[2][2], roll_ref_lines[2][3], COLOR_WHITE);
  sp->drawLine(roll_ref_lines[3][0], roll_ref_lines[3][1], roll_ref_lines[3][2], roll_ref_lines[3][3], COLOR_WHITE);
  sp->drawLine(roll_ref_lines[4][0], roll_ref_lines[4][1], roll_ref_lines[4][2], roll_ref_lines[4][3], COLOR_WHITE);

  // Roll Angle Reference Lines (RIGHT)
  sp->drawLine(roll_ref_lines[5][0], roll_ref_lines[5][1], roll_ref_lines[5][2], roll_ref_lines[5][3], COLOR_WHITE);
  sp->drawLine(roll_ref_lines[6][0], roll_ref_lines[6][1], roll_ref_lines[6][2], roll_ref_lines[6][3], COLOR_WHITE);
  sp->drawLine(roll_ref_lines[7][0], roll_ref_lines[7][1], roll_ref_lines[7][2], roll_ref_lines[7][3], COLOR_WHITE);
  sp->drawLine(roll_ref_lines[8][0], roll_ref_lines[8][1], roll_ref_lines[8][2], roll_ref_lines[8][3], COLOR_WHITE);
  sp->drawLine(roll_ref_lines[9][0], roll_ref_lines[9][1], roll_ref_lines[9][2], roll_ref_lines[9][3], COLOR_WHITE);

  
  // Roll Angle Reference (CENTER)
  // #define V1 200,30
  // #define V2 196,21
  // #define V3 204,21

  int sp_center_y = sp->height()/2;
  int sp_center_x = sp->width()/2;


  int v1_x = sp_center_x;   int v1_y = sp_center_y - indicator_rad;
  int v2_x = sp_center_x-4; int v2_y = sp_center_y - indicator_rad - 9;
  int v3_x = sp_center_x+3; int v3_y = sp_center_y - indicator_rad - 9;

  sp->fillTriangle(v1_x,v1_y, v2_x,v2_y, v3_x,v3_y, COLOR_WHITE);  
}



void ESP_GCS_PFD::render_pitch_scale(LGFX_Sprite *sp, int pitchPx) {
  
  int limit1 = 90;
  int limit2 = 60;
  
  int sp_center_y = sp->height()/2;
  int sp_center_x = sp->width()/2;
  int pitch_scale_range_px = 70;

  int pitch_scale_top_px = sp_center_y - pitch_scale_range_px;
  int pitch_scale_bot_px = sp_center_y + pitch_scale_range_px;  
  
  int scale_label_sky = 0;
  int scale_label_gnd = 0;

  int deg_10 = 40;
  int deg_2_5 = 10;
  int deg_5 = 20;

  int txt_lbl_offset = 5;  
  int length_25deg_marks = 12;
  int length_5deg_marks = 30;
  int length_10deg_marks = 50;  int height_10deg_marks = 2;

  sp->setTextSize(1);
  sp->setTextColor(COLOR_WHITE);

  // SKY ================================================================ 
  // 10 deg lines SKY
  for (int i=sp_center_y; i>=pitch_scale_top_px-pitchPx; i-=deg_10) { 
    if(i==sp_center_y || scale_label_sky > limit1);
    else {
      sp->drawRect(sp_center_x - length_10deg_marks/2, i+pitchPx-1, length_10deg_marks, height_10deg_marks, COLOR_WHITE);

      sp->setTextDatum(textdatum_t::top_left);
      //sp->setCursor(sp_center_x - length_10deg_marks/2, i+pitchPx); sp->print(scale_label_sky);

      sp->setTextDatum(textdatum_t::middle_left);
      sp->setCursor(sp_center_x + length_10deg_marks/2, i+pitchPx); sp->print(scale_label_sky);

      sp->setTextDatum(textdatum_t::top_left);
    }
    scale_label_sky+=10;
  }
  
  // 2.5 deg lines SKY
  for (int i=sp_center_y; i>=pitch_scale_top_px-pitchPx; i-=deg_2_5) {
    if(i==sp_center_y || scale_label_sky > limit2);
    else sp->drawFastHLine(sp_center_x - length_25deg_marks/2, i+pitchPx, length_25deg_marks, COLOR_WHITE);
  }
  
  // 5 deg lines SKY
  for (int i=sp_center_y; i>=pitch_scale_top_px-pitchPx; i-=deg_5) {
    if(i==sp_center_y || scale_label_sky > limit2);
    else {      
      sp->drawFastHLine(sp_center_x - length_5deg_marks/2, i+pitchPx, length_5deg_marks, COLOR_WHITE);
    }
  }

 
  // GND ================================================================
  // 10 deg lines GND
  for (int i=sp_center_y; i<=pitch_scale_bot_px-pitchPx; i+=deg_10) {
    if(i==sp_center_y || scale_label_gnd > limit1);
    else {
      sp->drawRect(sp_center_x - length_10deg_marks/2, i+pitchPx-1, length_10deg_marks, height_10deg_marks, COLOR_WHITE); 
      //sp->setCursor(160, i+pitchPx-4); sp->print(scale_label_gnd);
      //sp->setCursor(228, i+pitchPx-4); sp->print(scale_label_gnd);
    }
    scale_label_gnd+=10;
  }

  // 2.5 deg lines GND
  for (int i=sp_center_y; i<=pitch_scale_bot_px-pitchPx; i+=deg_2_5) {
    if(i==sp_center_y || scale_label_gnd > limit2);
    else sp->drawFastHLine(sp_center_x - length_25deg_marks/2, i+pitchPx, length_25deg_marks, COLOR_WHITE);
  }

  // 5 deg lines GND
  for (int i=sp_center_y; i<=pitch_scale_bot_px-pitchPx; i+=deg_5) {
    if(i==sp_center_y || scale_label_gnd > limit2);
    else {            
      sp->drawFastHLine(sp_center_x - length_5deg_marks/2, i+pitchPx, length_5deg_marks, COLOR_WHITE);
    }
  }
}

