//----------------------------------------------------------------------------- 
/** 
 *  
 * \file Display.cpp
 * \brief Human/User Interface source file
 * \author M.Navarro
 * \date 10/2018
 *
 */
//-----------------------------------------------------------------------------
// (c) Copyright MN 2018 - All rights reserved
//-----------------------------------------------------------------------------


//---------------------------------------------
// Include 
//---------------------------------------------
#include <U8g2lib.h>
#include "Display.h"
#include "GPS.h"
#include "logos.h"

//---------------------------------------------
// Enum, struct, union
//---------------------------------------------
#define OLED_TIMER_PERIOD_US    10000000


//---------------------------------------------
// Variables
//---------------------------------------------
U8G2_SSD1309_128X64_NONAME0_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 18, /* data=*/ 23, /* cs=*/ 5, /* dc=*/ 4, /* reset=*/ 0);  

dispSettings_str dispSettings;    ///< Display settings, user changeable
int maxRpm;

const uint8_t * logos[20] = { logo_ktm_bits,
                              logo_kawasaki_bits,
                              logo_polaris_bits,
                              logo_yamaha_bits,
                              logo_suzuki_bits};

// Timer interrupt, te refresh rpm value
hw_timer_t * oledTimer = NULL;
portMUX_TYPE oledTimerMux = portMUX_INITIALIZER_UNLOCKED;
volatile int oledTimerTick = 0;

int tata = 0;
  
//---------------------------------------------
// Public Functions
//---------------------------------------------
void OLED_Init();
void OLED_Draw();

// Differents screen that can be displayed
void OLED_Screen_Main(int vOffset);


//---------------------------------------------
// Private Functions
//---------------------------------------------
static void OLED_Display_RPM(int vOffset);
static void OLED_Display_Speed(int vOffset);
static void OLED_Display_TripDistance(int vOffset);
static void OLED_Display_TotalDistance(int vOffset);
static void OLED_Display_Time(int vOffset);
static void OLED_Display_Altitude(int vOffset);
static void OLED_Display_Gear(int vOffset);
static void OLED_Display_Track(int vOffset);

static int OLED_Scroll_Screens(void (*initScreen)(int), void (*finalScreen)(int));

void IRAM_ATTR displayTimerISR();

//---------------------------------------------
// Functions
//---------------------------------------------

void IRAM_ATTR oledTimerISR() 
{
  portENTER_CRITICAL_ISR(&oledTimerMux);
  oledTimerTick = 1;
  portEXIT_CRITICAL_ISR(&oledTimerMux);
}

//---------------------------------------------
/// \fn void OLED_Init(void)
///
/// \brief Init OLED Screen.
/// \param None.
/// \return None.
void OLED_Init()
{
  dispSettings.maxRPM = 8;
  Serial.begin(115200);
  u8g2.begin();

  u8g2.clearBuffer();
  u8g2.drawXBM(0, 0, 128, 64, logo_suzuki_bits);// logo_suzuki_bits);
  u8g2.sendBuffer();

  oledTimer = timerBegin(1, 80, true);
  timerAttachInterrupt(oledTimer, &oledTimerISR, true);
  timerAlarmWrite(oledTimer, OLED_TIMER_PERIOD_US, true);
  timerAlarmEnable(oledTimer);
}



void OLED_Draw()
{  
  static int scrollingOnGoing = 0;
  static int currentScreenNb = 0;
  static void (*currentScreen)(int) = OLED_Screen_Main;
  static void (*nextScreen)(int) = OLED_Display_Track;
  
   // Display main Screen only if splash logo finished 
  if(millis() > SPLASH_LOGO_DURATION_MS)
  {
    u8g2.clearBuffer();

    if(oledTimerTick)
    {
      oledTimerTick = 0;
      scrollingOnGoing = 1;
      
      if(nextScreen == OLED_Display_Track)
      {
        tata = 0;
      }
    }

    if(scrollingOnGoing)
    {
      if(OLED_Scroll_Screens(currentScreen, nextScreen))
      {
        scrollingOnGoing = 0;
        currentScreenNb++;
        if(currentScreenNb > 1)
        {
          currentScreenNb = 0;
        }
        
      }
    }
    else
    {
      if(currentScreenNb == 0 )
      {
         currentScreen = OLED_Screen_Main;
         nextScreen = OLED_Display_Track;
      }
      else
      {
        currentScreen = OLED_Display_Track;
        nextScreen = OLED_Screen_Main;
      }
      
      currentScreen(0);
    }
    /*if( (((int)millis()/1000) % 2) == 0)
    {  
      OLED_Screen_Main();
    }
    else
    {
      OLED_Display_Track();
    }*/
    
    
    u8g2.sendBuffer();
  }
}

static int OLED_Scroll_Screens(void (*initScreen)(int), void (*finalScreen)(int)  )
{
  static int offset = 0;

  initScreen(offset);
  finalScreen(offset-SCREEN_HEIGHT);
  /*OLED_Screen_Main(offset);
  OLED_Display_Track(offset-SCREEN_HEIGHT);*/
      
  offset += 8;
  
  if (offset >= SCREEN_HEIGHT)
  {
    offset = 0;
    return 1;
  }
  else
  {
    return 0;
  }
}

void OLED_Screen_Main(int vOffset)
{

  OLED_Display_RPM(vOffset);
  OLED_Display_Speed(vOffset);

  // Displays total distance for some seconds at startup, then actual trip distance
  if((millis() - SPLASH_LOGO_DURATION_MS) < TOTAL_DISTANCE_DISPLAY_DURATION_MS)
  { 
    OLED_Display_TotalDistance(vOffset);
  }
  else
  {
    OLED_Display_TripDistance(vOffset);
  }

  OLED_Display_Time(vOffset);
  
  OLED_Display_Altitude(vOffset); 
}


// Displays actual RPM, at the top of the screen.
static void OLED_Display_RPM(int vOffset)
{
  char buff[32];
  float coef = (float)SCREEN_WIDTH / (float)dispSettings.maxRPM;
  int bargraphWidth;

  u8g2.drawLine(1, vOffset+18, SCREEN_WIDTH, vOffset+18);

  // Cerates bargraph legend
  for(int i = 0; i < (dispSettings.maxRPM-1); i++)
  {
    u8g2.drawLine((coef*(i+1))+0.5, vOffset+8, (coef*(i+1))+0.5, vOffset+10);   // 500 rpms markers
    sprintf(buff, "%d", i+1);
    u8g2.setFont( u8g2_font_micro_tr);
    u8g2.drawStr( coef*(i+1)-1,vOffset+17 , buff);          //*1000 rpm numbers
  }

  for(int i = 0; i < (dispSettings.maxRPM+1); i++)
  {
    u8g2.drawLine((coef*i)+0.5+(coef/2)+0.5, vOffset+8, (coef*i)+0.5+coef/2+0.5, vOffset+8);    // 500 rpms markers   
  }

  bargraphWidth = (rpm / (dispSettings.maxRPM*1000.0) * SCREEN_WIDTH)+0.5;

   // Compute "Recent max rpm" bar
  if(bargraphWidth >= maxRpm)
  {
    maxRpm = bargraphWidth;
  }
  else
  {
    maxRpm -= 2;
    
    if(maxRpm < bargraphWidth)
    {
      maxRpm = bargraphWidth;
    }
  }

  // RPM Bar
  u8g2.drawBox(0, vOffset+0, bargraphWidth, 7);

  // Recent Max RPM indication
  u8g2.drawLine(maxRpm, vOffset+0, maxRpm, vOffset+7);
}


static void OLED_Display_Speed(int vOffset)
{
  char buff[32];

  if(gps.speed.isValid())
  {
    sprintf(buff, "%3d", (int)(spd+0.5));
  }
  else
  {
    sprintf(buff, "---");
  }

  u8g2.setFont( u8g2_font_helvB24_tn);//u8g2_font_freedoomr25_tn );
  u8g2.drawStr( 0, vOffset+48 , buff);
  u8g2.setFont( u8g2_font_5x7_tf);
  u8g2.drawStr( 56,vOffset+28 , "km/h");
}


//Displays current trip distance
static void OLED_Display_TripDistance(int vOffset)
{
  char buff[32];
  
  u8g2.setFont( u8g2_font_5x7_tf);//u8g2_font_6x10_tf);
  sprintf(buff, "Trip %d.%01d", (int)(trip/1000.0), (int)((trip/1000.0)*10.0)%10);  //  dtostrf(speed, 4, 1, buff);
  u8g2.drawStr( 0, vOffset+64 , buff);
}

//Displays toal distance parcoured
static void OLED_Display_TotalDistance(int vOffset)
{
  char buff[32];
  
  u8g2.setFont( u8g2_font_5x7_tf);//u8g2_font_6x10_tf);
  sprintf(buff, "Total %d.%01d", (int)total,(int)(total*100)%100);
  u8g2.drawStr( 0, vOffset+64 , buff);
}


//Displays current time
static void OLED_Display_Time(int vOffset)
{
  char buff[32];
  
  u8g2.setFont( u8g2_font_5x7_tf);
  
  if(gps.time.isValid())
  {
    sprintf( buff, "%02d:%02d ",hours ,minutes);
  }
  else
  {
    sprintf( buff, "--:-- ");
  }
  u8g2.drawStr( 100, vOffset+64 , buff);
}


static void OLED_Display_Altitude(int vOffset)
{
  char buff[32];
  
  if(gps.altitude.isValid())
  {
    sprintf( buff, "%4dm",alt);
  }
  else
  {
    sprintf( buff, "----m");
  }
  u8g2.drawStr( 100, vOffset+54 , buff);
}


static void OLED_Display_Gear(int vOffset)
{
  char buff[32];
  
  //Engaged gear display
  sprintf(buff, "N" );
  u8g2.setFont(u8g2_font_helvB18_tf );//u8g2_font_helvB24_tn  );//u8g2_font_freedoomr25_tn );// u8g2_font_logisoso26_tn  );
  u8g2.drawStr( 110, vOffset+42 , buff);
}


static void OLED_Display_Track(int vOffset)
{
  char buff[32];
  double latCoef, lngCoef;
  double latCartesian, lngCartesian;
 
/*
  gpsHistory.maxLng = 4.30778685;
  gpsHistory.minLng = 4.199846964;
  
  gpsHistory.maxLat = 45.49161321;
  gpsHistory.minLat = 45.45201852;
*/
/*
  gpsHistory.maxLng = -180;
  gpsHistory.minLng = 180;
  
  gpsHistory.maxLat = -90;
  gpsHistory.minLat = 90;
  
  gpsHistory.pointsIndex = tata;
  tata +=30;
  if(tata >=2038)
  {
    tata = 2038;
  }

  for(int i = 0; i < gpsHistory.pointsIndex; i++)
  {
    if( gpsHistory.minLat > titi[i*2+1])
    {
      gpsHistory.minLat = titi[i*2+1];
    }

    if(gpsHistory.maxLat < titi[i*2+1])
    {
      gpsHistory.maxLat = titi[i*2+1];
    }

    if( gpsHistory.minLng > titi[i*2])
    {
      gpsHistory.minLng = titi[i*2];
    }

    if( gpsHistory.maxLng < titi[i*2])
    {
      gpsHistory.maxLng = titi[i*2];
    }

  }
  */
  lngCoef = gpsHistory.maxLng - gpsHistory.minLng;
  latCoef = gpsHistory.maxLat - gpsHistory.minLat;

  for(int i = 0; i < gpsHistory.pointsIndex; i++)
  {
    /*lngCartesian = (titi[i*2] - gpsHistory.minLng) / lngCoef * (double)SCREEN_WIDTH;
    latCartesian = SCREEN_HEIGHT-((titi[i*2+1] - gpsHistory.minLat) / latCoef * (double)SCREEN_HEIGHT);*/
    latCartesian =  SCREEN_HEIGHT - ((gpsHistory.points[i].lat - gpsHistory.minLat) / latCoef * SCREEN_HEIGHT);
    lngCartesian =  (gpsHistory.points[i].lng - gpsHistory.minLng) / lngCoef * SCREEN_WIDTH;

    u8g2.drawPixel((int)lngCartesian, vOffset+(int)latCartesian);
  }

   u8g2.drawDisc((int)lngCartesian, vOffset+(int)latCartesian, 2);
}
