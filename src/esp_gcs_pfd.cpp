#include "esp_gcs_pfd.h"

                        
ESP_GCS_PFD::ESP_GCS_PFD() {};


void ESP_GCS_PFD::init(esp_gcs_config_t* config) {

  init_fb(config);


  #define BG_COLOR 0xEE4C //0xC660 //0xC680
  #define OL_COLOR TFT_BLACK

  ai.setColorDepth(8);
  ai.createSprite(70, 21);
  ai.fillSprite(TFT_TRANSPARENT);

  lvl1.setColorDepth(8);
  lvl1.createSprite(30, 7);
  lvl1.fillSprite(TFT_TRANSPARENT);

  lvl2.setColorDepth(8);
  lvl2.createSprite(30, 7);
  lvl2.fillSprite(TFT_TRANSPARENT);


  // Left Level ----------------------------------    
  #define RWH 25,7    
  #define TV1 25,0
  #define TV2 29,3
  #define TV3 25,6
  
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

  // Data indicators
  da.setColorDepth(1);
  da.createSprite(60, 30);
  da.fillSprite(TFT_BLACK);
  da.setTextSize(2);
  da.setTextDatum(MC_DATUM);
  da.setTextColor(TFT_WHITE);
  da.drawRect(0,0, 60,29, TFT_WHITE);

  // Bottom Bar
  br.setColorDepth(8);
  br.createSprite(320, 30);
  br.fillSprite(TFT_BLACK);
  br.setTextSize(2);
  br.setTextDatum(MC_DATUM);    
  br.drawFastHLine(0,0, 320, TFT_WHITE);
  br.drawFastHLine(0,29, 320, TFT_WHITE);

  int spc = 40;    
  br.drawFastVLine(160-spc,0, 29, TFT_WHITE);
  br.drawFastVLine(160+spc,0, 29, TFT_WHITE);


  // Top Bar
  tbr.setColorDepth(8);
  tbr.createSprite(320, 15);
  tbr.fillSprite(TFT_BLACK);    
  tbr.drawFastHLine(0,0, 320, TFT_WHITE);
  tbr.drawFastHLine(0,14, 320, TFT_WHITE);

  tbr.drawFastVLine(80,0, 14, TFT_WHITE);
  tbr.drawFastVLine(160,0, 14, TFT_WHITE);
  tbr.drawFastVLine(240,0, 14, TFT_WHITE);

  tbr.setTextSize(1);
  tbr.setTextDatum(ML_DATUM); 
  
  tbr.setTextColor(TFT_WHITE); tbr.drawString("LINK", 4,8, 1);
  tbr.setTextColor(TFT_GREEN); tbr.drawString("TCP", 40,8, 1);
  
  
  tbr.setTextColor(TFT_WHITE);  tbr.drawString("GPS", 85,8, 1);
  tbr.setTextColor(TFT_RED); tbr.drawString("NO FIX", 112,8, 1);
  
  tbr.setTextColor(TFT_WHITE);  tbr.drawString("WPT", 165,8, 1);
  tbr.setTextColor(TFT_BLUE);  tbr.drawString("0.00", 195,8, 1);
  
  tbr.setTextColor(TFT_WHITE);  tbr.drawString("BAT", 245,8, 1);
  tbr.setTextColor(TFT_GREEN); tbr.drawString("13.92v", 272,8, 1);

  preCalcRollReferenceLines();

  // Render Background
  frame_buffer.fillRect(0,0,                  fb_width, fb_height/2, TFT_SKY);   // SKY
  frame_buffer.fillRect(0,fb_height/2,        fb_width, fb_height/2, TFT_GND);   // GROUND  
  frame_buffer.drawSmoothLine(0, fb_height/2, fb_width, fb_height/2, TFT_WHITE);            // HORIZON


  renderPFDPitchScale(&frame_buffer, 0);
  renderPFDTurnIndicator(&frame_buffer);

  // Aircraft and level indicators ---------------------------------------------      
  int ai_zoom = 1;
  int ai_x = frame_buffer.width()/2 - ai.width()/2;
  int ai_y = frame_buffer.height()/2;
  ai.setPivot(frame_buffer.width()/2 - ai_x, frame_buffer.height()/2 - ai_y);
  ai.pushRotateZoomWithAA(&frame_buffer, -0, ai_zoom,ai_zoom, TFT_TRANSPARENT);


  //lvl1.pushToSprite(&fb, 160-20-50, 120-3, TFT_TRANSPARENT, -inRoll);
  int l1_offset = 60;
  int lvl1_x = frame_buffer.width()/2 - lvl1.width()/2 - l1_offset;
  int lvl1_y = frame_buffer.height()/2 - lvl1.height()/2;
  lvl1.setPivot(lcd.width()/2 - lvl1_x, lcd.height()/2 - lvl1_y);
  lvl1.pushRotated(&frame_buffer, 0, TFT_TRANSPARENT);


  //lvl2.pushToSprite(&fb, 160+50-10, 120-3, TFT_TRANSPARENT, -inRoll);
  int l2_offset = 60;
  int lvl2_x = frame_buffer.width()/2 - lvl2.width()/2 + l2_offset;
  int lvl2_y = frame_buffer.height()/2 - lvl2.height()/2;
  lvl2.setPivot(lcd.width()/2 - lvl2_x, lcd.height()/2 - lvl2_y);
  lvl2.pushRotated(&frame_buffer, 0, TFT_TRANSPARENT);


  // Turn indicator ------------------------------------------------------------
  //ti.pushToSprite(&fb, 155, 32, TFT_TRANSPARENT, -inRoll);  // turn indicator
  int ti_x = frame_buffer.width()/2 - ti.width()/2;
  int ti_y = frame_buffer.height()/2 - ti.height()/2 - indicator_rad + 8;
  ti.setPivot(lcd.width()/2 - ti_x, lcd.height()/2 - ti_y);
  ti.pushRotated(&frame_buffer, -0, TFT_TRANSPARENT);


  // push frame_buffer to LCD
  frame_buffer.pushRotateZoom(0, 1.2,1.2);
}



void ESP_GCS_PFD::render(int pitch, int roll) {

  // Render Background
  frame_buffer.fillRect(0,0,                    fb_width, fb_height/2, TFT_SKY);   // SKY
  frame_buffer.fillRect(0,fb_height/2,          fb_width, fb_height/2, TFT_GND);   // GROUND
  frame_buffer.drawFastHLine(0, fb_height/2,    fb_width, TFT_WHITE);            // HORIZON

  frame_buffer.pushSprite(fb_center_x - 50, fb_center_y);
  frame_buffer.pushSprite(fb_center_x + 50, fb_center_y);
}



void ESP_GCS_PFD::render(mavlink_heartbeat_t hb, mavlink_attitude_t atti, mavlink_vfr_hud_t hud) {

  #define PITCH_ANGLE_TO_PX 1

  // ALTITUDE UNITS CONVERSION
  #define M_TO_FT 3.28084
  #define ALT_UNIT_CONV M_TO_FT

  // SPEED UNITS CONVERSION  
  #define MPS_TO_KPH  3.6
  #define MPS_TO_KTS  1.94384
  #define MPS_TO_MPH  2.23694
  #define SPEED_UNIT_CONV MPS_TO_KTS

  float inPitch0 = degrees(atti.pitch);
  int inPitch = round( inPitch0 ) * PITCH_ANGLE_TO_PX;

  int inRoll =  round( -degrees(atti.roll) );    
  int inAlt = round( hud.alt * ALT_UNIT_CONV );  
  int inSpeed = round( hud.groundspeed * SPEED_UNIT_CONV );   

  // Render Background
  frame_buffer.fillRect(0,0,                    fb_width, fb_height/2+inPitch, TFT_SKY);   // SKY
  frame_buffer.fillRect(0,fb_height/2+inPitch,  fb_width, fb_height/2-inPitch, TFT_GND);   // GROUND  
  frame_buffer.drawSmoothLine(0, fb_height/2+inPitch,  fb_width, fb_height/2+inPitch, TFT_WHITE);            // HORIZON
  

  renderPFDPitchScale(&frame_buffer, inPitch);
  renderPFDTurnIndicator(&frame_buffer);


  // Aircraft and level indicators ---------------------------------------------      
  int ai_x = frame_buffer.width()/2 - ai.width()/2;
  int ai_y = frame_buffer.height()/2;
  ai.setPivot(frame_buffer.width()/2 - ai_x, frame_buffer.height()/2 - ai_y);
  ai.pushRotateZoomWithAA(&frame_buffer, -inRoll, indicator_zoom,indicator_zoom, TFT_TRANSPARENT);

  //lvl1.pushToSprite(&fb, 160-20-50, 120-3, TFT_TRANSPARENT, -inRoll);
  int l1_offset = 60;
  int lvl1_x = frame_buffer.width()/2 - lvl1.width()/2 - l1_offset;
  int lvl1_y = frame_buffer.height()/2 - lvl1.height()/2;
  lvl1.setPivot(lcd.width()/2 - lvl1_x, lcd.height()/2 - lvl1_y);
  lvl1.pushRotateZoomWithAA(&frame_buffer, -inRoll, lvl_zoom,lvl_zoom, TFT_TRANSPARENT);


  //lvl2.pushToSprite(&fb, 160+50-10, 120-3, TFT_TRANSPARENT, -inRoll);  
  int l2_offset = 60;
  int lvl2_x = frame_buffer.width()/2 - lvl2.width()/2 + l2_offset;
  int lvl2_y = frame_buffer.height()/2 - lvl2.height()/2;
  lvl2.setPivot(lcd.width()/2 - lvl2_x, lcd.height()/2 - lvl2_y);
  lvl2.pushRotateZoomWithAA(&frame_buffer, -inRoll, lvl_zoom,lvl_zoom, TFT_TRANSPARENT);


  // Turn indicator ------------------------------------------------------------
  //ti.pushToSprite(&fb, 155, 32, TFT_TRANSPARENT, -inRoll);  // turn indicator
  int ti_x = frame_buffer.width()/2 - ti.width()/2;
  int ti_y = frame_buffer.height()/2 - ti.height()/2 - indicator_rad + 8;
  ti.setPivot(lcd.width()/2 - ti_x, lcd.height()/2 - ti_y);
  ti.pushRotateZoomWithAA(&frame_buffer, -inRoll, 1,1, TFT_TRANSPARENT);

  //frame_buffer.pushSprite( 0, 0 );
  //frame_buffer.pushSprite(fb_center_x, fb_center_y);
  //frame_buffer.pushRotatedWithAA(inRoll);
  //frame_buffer.pushRotated(inRoll);
  //frame_buffer.pushRotateZoomWithAA(inRoll, 1, 1);
  frame_buffer.pushRotateZoom(inRoll, framebuffer_zoom,framebuffer_zoom);
}



void ESP_GCS_PFD::preCalcRollReferenceLines() {

  // Turn indicator arc  
  int cX = frame_buffer.width()/2;   //160;
  int cY = frame_buffer.height()/2;  //80;

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



void ESP_GCS_PFD::renderPFDTurnIndicator(LGFX_Sprite *sp) {  

  // Roll Angle Reference Lines (LEFT)
  sp->drawSmoothLine(roll_ref_lines[0][0], roll_ref_lines[0][1], roll_ref_lines[0][2], roll_ref_lines[0][3], TFT_WHITE);
  sp->drawSmoothLine(roll_ref_lines[1][0], roll_ref_lines[1][1], roll_ref_lines[1][2], roll_ref_lines[1][3], TFT_WHITE);
  sp->drawSmoothLine(roll_ref_lines[2][0], roll_ref_lines[2][1], roll_ref_lines[2][2], roll_ref_lines[2][3], TFT_WHITE);
  sp->drawSmoothLine(roll_ref_lines[3][0], roll_ref_lines[3][1], roll_ref_lines[3][2], roll_ref_lines[3][3], TFT_WHITE);
  sp->drawSmoothLine(roll_ref_lines[4][0], roll_ref_lines[4][1], roll_ref_lines[4][2], roll_ref_lines[4][3], TFT_WHITE);

  // Roll Angle Reference Lines (RIGHT)
  sp->drawSmoothLine(roll_ref_lines[5][0], roll_ref_lines[5][1], roll_ref_lines[5][2], roll_ref_lines[5][3], TFT_WHITE);
  sp->drawSmoothLine(roll_ref_lines[6][0], roll_ref_lines[6][1], roll_ref_lines[6][2], roll_ref_lines[6][3], TFT_WHITE);
  sp->drawSmoothLine(roll_ref_lines[7][0], roll_ref_lines[7][1], roll_ref_lines[7][2], roll_ref_lines[7][3], TFT_WHITE);
  sp->drawSmoothLine(roll_ref_lines[8][0], roll_ref_lines[8][1], roll_ref_lines[8][2], roll_ref_lines[8][3], TFT_WHITE);
  sp->drawSmoothLine(roll_ref_lines[9][0], roll_ref_lines[9][1], roll_ref_lines[9][2], roll_ref_lines[9][3], TFT_WHITE);
  
  // Roll Angle Reference (CENTER)
  // #define V1 200,30
  // #define V2 196,21
  // #define V3 204,21

  int sp_center_y = sp->height()/2;
  int sp_center_x = sp->width()/2;


  int v1_x = sp_center_x;   int v1_y = sp_center_y - indicator_rad;
  int v2_x = sp_center_x-4; int v2_y = sp_center_y - indicator_rad - 9;
  int v3_x = sp_center_x+3; int v3_y = sp_center_y - indicator_rad - 9;

  sp->fillTriangle(v1_x,v1_y, v2_x,v2_y, v3_x,v3_y, TFT_WHITE);  
}



void ESP_GCS_PFD::renderPFDPitchScale(LGFX_Sprite *sp, int pitchPx) {
  
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

 
  // SKY ================================================================ 
  // 10 deg lines SKY
  for (int i=sp_center_y; i>=pitch_scale_top_px-pitchPx; i-=deg_10) { 
    if(i==sp_center_y || scale_label_sky > limit1);
    else {
      sp->drawRect(sp_center_x - length_10deg_marks/2, i+pitchPx-1, length_10deg_marks, height_10deg_marks, TFT_WHITE);

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
    else sp->drawFastHLine(sp_center_x - length_25deg_marks/2, i+pitchPx, length_25deg_marks, TFT_WHITE);
  }
  
  // 5 deg lines SKY
  for (int i=sp_center_y; i>=pitch_scale_top_px-pitchPx; i-=deg_5) {
    if(i==sp_center_y || scale_label_sky > limit2);
    else {      
      sp->drawFastHLine(sp_center_x - length_5deg_marks/2, i+pitchPx, length_5deg_marks, TFT_WHITE);
    }
  }

 
  // GND ================================================================
  // 10 deg lines GND
  for (int i=sp_center_y; i<=pitch_scale_bot_px-pitchPx; i+=deg_10) {
    if(i==sp_center_y || scale_label_gnd > limit1);
    else {
      sp->drawRect(sp_center_x - length_10deg_marks/2, i+pitchPx-1, length_10deg_marks, height_10deg_marks, TFT_WHITE); 
      //sp->setCursor(160, i+pitchPx-4); sp->print(scale_label_gnd);
      //sp->setCursor(228, i+pitchPx-4); sp->print(scale_label_gnd);
    }
    scale_label_gnd+=10;
  }

  // 2.5 deg lines GND
  for (int i=sp_center_y; i<=pitch_scale_bot_px-pitchPx; i+=deg_2_5) {
    if(i==sp_center_y || scale_label_gnd > limit2);
    else sp->drawFastHLine(sp_center_x - length_25deg_marks/2, i+pitchPx, length_25deg_marks, TFT_WHITE);
  }

  // 5 deg lines GND
  for (int i=sp_center_y; i<=pitch_scale_bot_px-pitchPx; i+=deg_5) {
    if(i==sp_center_y || scale_label_gnd > limit2);
    else {            
      sp->drawFastHLine(sp_center_x - length_5deg_marks/2, i+pitchPx, length_5deg_marks, TFT_WHITE);
    }
  }
}

