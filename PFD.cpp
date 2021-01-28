#include "PFD.hpp"

                        
const fmode_t flightModes[] = {
  //{FLIGHTMODE,COLOR,        ARMED,    DISARMED}
  {"STABILIZE", TFT_WHITE,    {209,0},  {81,0}  },
  {"ACRO",      TFT_WHITE,    {209,1},  {81,1}  },
  {"ALT HOLD",  TFT_SKYBLUE,  {209,2},  {81,2}  },
  {"AUTO",      TFT_CYAN,     {0,0},    {89,3}  }, // >> not armable mode
  {"GUIDED",    TFT_ORANGE,   {217,4},  {89,4}  },
  {"LOITER",    TFT_BLUE,     {217,5},  {89,5}  },
  {"RTL",       TFT_CYAN,     {0,0},    {89,6}  }, // >> not armable mode
  {"CIRCLE",    TFT_CYAN,     {0,0},    {89,7}  }, // >> not armable mode
  {"LAND",      TFT_CYAN,     {0,0},    {81,9}  }, // >> not armable mode
  {"DRIFT",     TFT_CYAN,     {0,0},    {81,11} }, // >> not armable mode
  {"SPORT",     TFT_CYAN,     {0,0},    {81,13} }, // >> not armable mode
  {"POS HOLD",  TFT_BLUE,     {217,16}, {89,16} }                          
};


void PFD::init(uint16_t FB_WIDTH, uint16_t FB_HEIGHT) {
  initFB(FB_WIDTH, FB_HEIGHT);
  
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

  lvl1.drawLine(0,0, TV1, OL_COLOR);
  lvl1.drawLine(0,0, 0,7, OL_COLOR);
  lvl1.drawLine(0,6, TV3, OL_COLOR);    
  
  lvl1.drawLine(TV1, TV2, OL_COLOR);
  lvl1.drawLine(TV2, TV3, OL_COLOR);


  // Right Level ----------------------------------
  #define RWH_2 25,8
  #define TV1_2 5,0
  #define TV2_2 0,3
  #define TV3_2 5,6
  
  lvl2.fillRect(5,0, RWH_2, BG_COLOR);
  lvl2.fillTriangle(TV1_2, TV2_2, TV3_2, BG_COLOR);

  lvl2.drawLine(29,0, TV1_2, OL_COLOR);    
  lvl2.drawLine(29,0, 29,7, OL_COLOR);
  lvl2.drawLine(29,6, TV3_2, OL_COLOR);
  
  lvl2.drawLine(TV1_2, TV2_2, OL_COLOR);
  lvl2.drawLine(TV2_2, TV3_2, OL_COLOR);
  
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
  ai.drawLine(NOSE, LWING, OL_COLOR);
  ai.drawLine(NOSE, RWING, OL_COLOR);

  ai.drawLine(LWING, TAIL, OL_COLOR);
  ai.drawLine(RWING, TAIL, OL_COLOR);

  // Internal black Lines
  ai.drawLine(NOSE, CENTER, OL_COLOR);
  ai.drawLine(CENTER, LWING, OL_COLOR);
  ai.drawLine(CENTER, RWING, OL_COLOR);
  
 

  // Turn Angle Indicator Sprite -------------------
  ti.setColorDepth(8);  
  ti.createSprite(10, 10);
  ti.fillSprite(TFT_TRANSPARENT);
  ti.fillTriangle(5,0, 1,8, 9,8, TFT_WHITE);

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


  // Menu Bar
  mbr.setColorDepth(8);
  mbr.createSprite(320, 30);
  mbr.fillSprite( TFT_BLACK );
  mbr.drawFastHLine(0,0, 320, TFT_WHITE);
  mbr.drawFastHLine(0,29, 320, TFT_WHITE);

  mbr.drawFastVLine(105,1, 29, TFT_WHITE);  
  mbr.drawFastVLine(320-105,1, 29, TFT_WHITE);

  mbr.setTextSize(2);
  mbr.setTextDatum(MC_DATUM);
  
  mbr.setTextColor(TFT_WHITE); 
  mbr.drawString("HSI", 53,15, 1);
  mbr.drawString("MENU", 160,15, 1);
  mbr.drawString("ADS-B", 320-53,15, 1);
  

  preCalcRollReferenceLines();
}






void PFD::render(mavlink_heartbeat_t hb, mavlink_attitude_t atti, mavlink_vfr_hud_t hud) {  

  #define PITCH_ANGLE_TO_PX 4

  // ALTITUDE UNITS CONVERSION
  #define M_TO_FT 3.28084
  #define ALT_UNIT_CONV M_TO_FT

  // SPEED UNITS CONVERSION  
  #define MPS_TO_KPH  3.6
  #define MPS_TO_KTS  1.94384
  #define MPS_TO_MPH  2.23694
  #define SPEED_UNIT_CONV MPS_TO_KTS
  
  
  int inPitch = round( degrees(atti.pitch) ) * PITCH_ANGLE_TO_PX;
  int inRoll =  round( -degrees(atti.roll) );    

  //if(hud.alt < 0) hud.alt = 0;
  int inAlt = round( hud.alt * ALT_UNIT_CONV );  
  int inSpeed = round( hud.groundspeed * SPEED_UNIT_CONV );    


  // Render Background
  fb.fillRect(0,0,                    FB_WIDTH, FB_HEIGHT/2+inPitch, SKY);   // SKY
  fb.fillRect(0,FB_HEIGHT/2+inPitch,  FB_WIDTH, FB_HEIGHT/2-inPitch, GND);   // GROUND
  fb.drawFastHLine(0, FB_HEIGHT/2+inPitch,  FB_WIDTH, TFT_WHITE);            // HORIZON 

  
  // Render Overlays
  renderPFDPitchScale(&fb, inPitch);
  renderPFDTurnIndicator(&fb);
  

  
  // Aircraft and level indicators ---------------------------------------------
  ai.pushToSprite(&fb, 125, 119, TFT_TRANSPARENT, -inRoll);
  lvl1.pushToSprite(&fb, 160-20-50, 120-3, TFT_TRANSPARENT, -inRoll);
  lvl2.pushToSprite(&fb, 160+50-10, 120-3, TFT_TRANSPARENT, -inRoll);
  

  // Turn indicator ------------------------------------------------------------
  ti.pushToSprite(&fb, 155, 32, TFT_TRANSPARENT, -inRoll);  // turn indicator 
  
  // TOP Bar -------------------------------------------------------------------
  tbr.pushToSprite(&fb, 0,0, TFT_TRANSPARENT, -inRoll);
  
  // Bottom Bar ----------------------------------------------------------------

  // Handle Flight Modes and Arm/Disarm
  hbmode_t newMode{hb.base_mode, hb.custom_mode};
  
  if( newMode != prevMode) {
    
    // Erase previous modes
    br.fillRect(0,2, 118,26, TFT_BLACK);    // Flight Mode Name
    br.fillRect(201,2, 118,26, TFT_BLACK);  // Arm/Disarm Indicator

    for(int i=0; i<=sizeof(flightModes); i++) {
      
      if( newMode ==  flightModes[i].modeA ) {
        br.setTextColor(flightModes[i].color); br.drawString(flightModes[i].fmode, 60,15, 1);
        br.setTextColor(TFT_GREEN); br.drawString("ARMED", 260,15, 1);
        flashNP(3,200, 0,255,0);
        break;
      }
      
      else if( newMode ==  flightModes[i].modeD ) {
        br.setTextColor(flightModes[i].color); br.drawString(flightModes[i].fmode, 60,15, 1);
        br.setTextColor(TFT_RED); br.drawString("DISARMED", 260,15, 1);
        flashNP(3,200, 255,0,0);
        break;
      }
      else {}
    }      
    prevMode = newMode;
  }

  // Erase Previous Heading
  br.setTextColor(TFT_BLACK);
  br.drawString(txtBuff, 160,15, 1);

  // Print New Heading
  br.setTextColor(TFT_WHITE); 
  sprintf(txtBuff, "%03d", hud.heading);
  br.drawString(txtBuff, 160,15, 1); br.drawCircle(185,8, 2, TFT_WHITE);
  
  // Push bottom bar to FB
  br.pushToSprite(&fb, 0,240-30, TFT_TRANSPARENT, -inRoll);

  // Push menu bar to FB
  if(showMenu) mbr.pushToSprite(&fb, 0,240-30, TFT_TRANSPARENT, -inRoll);
  
  
  // Left Speed Indicator ------------------------------------------------------
  da.fillSprite(TFT_BLACK); 
  da.drawRect(0,0, 60,29, TFT_WHITE);
  da.setTextSize(2); da.drawNumber(inSpeed, 30,15);    
  da.setTextSize(1); da.drawString("kt",50, 22);    
  da.pushToSprite(&fb, 0, 105, TFT_RED, -inRoll);
  

  // Right Altitude Indicator --------------------------------------------------
  da.fillSprite(TFT_BLACK);
  da.drawRect(0,0, 60,29, TFT_WHITE);
  da.setTextSize(2); da.drawNumber(inAlt, 30,15);
  da.setTextSize(1); da.drawString("ft",50, 22);
  da.pushToSprite(&fb, 260, 105, TFT_RED, -inRoll);
  
      
  // Push FB to TFT ------------------------------------------------------------
  fb.pushRotated(inRoll, TFT_TRANSPARENT);                  // framebuffer

  
  // Calculate FPS
  calculateFPS(); 

  // Button Actions  
  BtnUpdate();
}






void PFD::calculateFPS() {
    
  // Calculate FPS
  counter++;  
  if (counter % interval == 0) {
    long millisSinceUpdate = millis() - startMillis;      
    log_d("PFD Running @ %2.4f fps", interval * 1000.0 / millisSinceUpdate);
    startMillis = millis();
  }  
}






void PFD::renderPFDTurnIndicator(TFT_eSprite *sp) {  

  // Roll Angle Reference Lines (LEFT)
  sp->drawLine(RollRefLines[0][0], RollRefLines[0][1], RollRefLines[0][2], RollRefLines[0][3], TFT_WHITE);
  sp->drawLine(RollRefLines[1][0], RollRefLines[1][1], RollRefLines[1][2], RollRefLines[1][3], TFT_WHITE);
  sp->drawLine(RollRefLines[2][0], RollRefLines[2][1], RollRefLines[2][2], RollRefLines[2][3], TFT_WHITE);
  sp->drawLine(RollRefLines[3][0], RollRefLines[3][1], RollRefLines[3][2], RollRefLines[3][3], TFT_WHITE);
  sp->drawLine(RollRefLines[4][0], RollRefLines[4][1], RollRefLines[4][2], RollRefLines[4][3], TFT_WHITE);

  // Roll Angle Reference Lines (RIGHT)
  sp->drawLine(RollRefLines[5][0], RollRefLines[5][1], RollRefLines[5][2], RollRefLines[5][3], TFT_WHITE);
  sp->drawLine(RollRefLines[6][0], RollRefLines[6][1], RollRefLines[6][2], RollRefLines[6][3], TFT_WHITE);
  sp->drawLine(RollRefLines[7][0], RollRefLines[7][1], RollRefLines[7][2], RollRefLines[7][3], TFT_WHITE);
  sp->drawLine(RollRefLines[8][0], RollRefLines[8][1], RollRefLines[8][2], RollRefLines[8][3], TFT_WHITE);
  sp->drawLine(RollRefLines[9][0], RollRefLines[9][1], RollRefLines[9][2], RollRefLines[9][3], TFT_WHITE);
  
  // Roll Angle Reference (CENTER)
  #define V1 200,30
  #define V2 196,21
  #define V3 204,21    
  sp->fillTriangle(V1, V2, V3, TFT_WHITE);
}




void PFD::renderPFDPitchScale(TFT_eSprite *sp, int pitchPx) {
  
  #define limit1 90
  #define limit2 60
  
  int spC = sp->height()/2;
  int topL = spC-70;
  int botL = spC+70;
  int inc = 40;
  int itS = 0;
  int itG = 0;  
 
  // SKY ================================================================ 
  // 10 deg lines SKY
  for (int i=spC; i>=topL-pitchPx; i-=40) { 
    if(i==spC || itS > limit1);
    else {
      sp->drawRect(175, i+pitchPx-1, 50, 2, TFT_WHITE);
      sp->setCursor(160, i+pitchPx-4); sp->print(itS);
      sp->setCursor(228, i+pitchPx-4); sp->print(itS);
    }
    itS+=10;
  }
  
  // 2.5 deg lines SKY
  for (int i=spC; i>=topL-pitchPx; i-=10) {
    if(i==spC || itS > limit2);
    else sp->drawFastHLine(194, i+pitchPx, 12, TFT_WHITE);
  }
  
  // 5 deg lines SKY
  for (int i=spC; i>=topL-pitchPx; i-=20) {
    if(i==spC || itS > limit2);
    else sp->drawFastHLine(185, i+pitchPx, 30, TFT_WHITE);
  }

 
  // GND ================================================================
  // 10 deg lines GND
  for (int i=spC; i<=botL-pitchPx; i+=40) {
    if(i==spC || itG > limit1);
    else {
      sp->drawFastHLine(175, i+pitchPx, 50, TFT_WHITE); 
      sp->setCursor(160, i+pitchPx-4); sp->print(itG);
      sp->setCursor(228, i+pitchPx-4); sp->print(itG);
    }
    itG+=10;
  }

  // 2.5 deg lines GND
  for (int i=spC; i<=botL-pitchPx; i+=10) {
    if(i==spC || itG > limit2);
    else sp->drawFastHLine(194, i+pitchPx, 12, TFT_WHITE);
  }

  // 5 deg lines GND
  for (int i=spC; i<=botL-pitchPx; i+=20) {
    if(i==spC || itG > limit2);
    else sp->drawFastHLine(185, i+pitchPx, 30, TFT_WHITE);
  }
}





void PFD::preCalcRollReferenceLines() {

  // Turn indicator arc
  int r = 90;
  int cX = fb.width()/2;   //160;
  int cY = fb.height()/2;  //80;

  int S = 7;
  int L = 13;
  
  for(float i=-60; i<=60; i+=5) {
    int x0 = cX + cos( radians(i-90) ) * r;
    int y0 = cY + sin( radians(i-90) ) * r;
    
    int x11 = cX + cos( radians(i-90) ) * (r+S);
    int y11 = cY + sin( radians(i-90) ) * (r+S);
    
    int x12 = cX + cos( radians(i-90) ) * (r+L);
    int y12 = cY + sin( radians(i-90) ) * (r+L);

    //=============================
    if(i==-10) {
      RollRefLines[0][0] = x0;
      RollRefLines[0][1] = y0;
      RollRefLines[0][2] = x11;
      RollRefLines[0][3] = y11;
    }
    
    if(i==-20) {
      RollRefLines[1][0] = x0;
      RollRefLines[1][1] = y0;
      RollRefLines[1][2] = x11;
      RollRefLines[1][3] = y11;
    }
    
    if(i==-30) {
      RollRefLines[2][0] = x0;
      RollRefLines[2][1] = y0;
      RollRefLines[2][2] = x12;
      RollRefLines[2][3] = y12;
    }
    
    if(i==-45) {
      RollRefLines[3][0] = x0; 
      RollRefLines[3][1] = y0; 
      RollRefLines[3][2] = x11;
      RollRefLines[3][3] = y11;
    }
    
    if(i==-60) {
      RollRefLines[4][0] = x0; 
      RollRefLines[4][1] = y0; 
      RollRefLines[4][2] = x12;
      RollRefLines[4][3] = y12;
    }


    //=============================
    if(i==10) {
      RollRefLines[5][0] = x0;
      RollRefLines[5][1] = y0;
      RollRefLines[5][2] = x11;
      RollRefLines[5][3] = y11;
    }
    
    if(i==20) {
      RollRefLines[6][0] = x0;
      RollRefLines[6][1] = y0;
      RollRefLines[6][2] = x11;
      RollRefLines[6][3] = y11;
    }
    
    if(i==30) {
      RollRefLines[7][0] = x0; 
      RollRefLines[7][1] = y0; 
      RollRefLines[7][2] = x12;
      RollRefLines[7][3] = y12;
    }
    
    if(i==45) {
      RollRefLines[8][0] = x0; 
      RollRefLines[8][1] = y0; 
      RollRefLines[8][2] = x11;
      RollRefLines[8][3] = y11;
    }
    
    if(i==60) {
      RollRefLines[9][0] = x0; 
      RollRefLines[9][1] = y0; 
      RollRefLines[9][2] = x12;
      RollRefLines[9][3] = y12;
    }    
  }
}
