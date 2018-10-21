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


//---------------------------------------------
// Public Functions
//---------------------------------------------
void OLED_Init();
void OLED_Draw();

// Differents screen that can be displayed
void OLED_Screen_Main();


//---------------------------------------------
// Private Functions
//---------------------------------------------
static void OLED_Display_RPM();
static void OLED_Display_Speed();
static void OLED_Display_TripDistance();
static void OLED_Display_TotalDistance();
static void OLED_Display_Time();
static void OLED_Display_Altitude();
static void OLED_Display_Gear();


//---------------------------------------------
// Functions
//---------------------------------------------

//---------------------------------------------
/// \fn void OLED_Init(void)
///
/// \brief Init OLED Screen.
/// \param None.
/// \return None.
void OLED_Init()
{
  dispSettings.maxRPM = 8;
  
  u8g2.begin();

  u8g2.clearBuffer();
  u8g2.drawXBM(0, 0, 128, 64, logo_suzuki_bits);// logo_suzuki_bits);
  u8g2.sendBuffer();
}



void OLED_Draw()
{
   // Display main Screen only if splash logo finished 
  if(millis() > SPLASH_LOGO_DURATION_MS)
  {
    u8g2.clearBuffer();
  
    OLED_Screen_Main();
  
    u8g2.sendBuffer();
  }
}


void OLED_Screen_Main()
{

  OLED_Display_RPM();
  OLED_Display_Speed();

  // Displays total distance for some seconds at startup, then actual trip distance
  if((millis() - SPLASH_LOGO_DURATION_MS) < TOTAL_DISTANCE_DISPLAY_DURATION_MS)
  { 
    OLED_Display_TotalDistance();
  }
  else
  {
    OLED_Display_TripDistance();
  }

  OLED_Display_Time();
  
  OLED_Display_Altitude(); 
}


// Displays actual RPM, at the top of the screen.
static void OLED_Display_RPM()
{
  char buff[32];
  float coef = (float)SCREEN_WIDTH / (float)dispSettings.maxRPM;
  int bargraphWidth;

  u8g2.drawLine(1, 18, SCREEN_WIDTH, 18);

  // Cerates bargraph legend
  for(int i = 0; i < (dispSettings.maxRPM-1); i++)
  {
    u8g2.drawLine((coef*(i+1))+0.5, 8, (coef*(i+1))+0.5, 10);   // 500 rpms markers
    sprintf(buff, "%d", i+1);
    u8g2.setFont( u8g2_font_micro_tr);
    u8g2.drawStr( coef*(i+1)-1,17 , buff);          //*1000 rpm numbers
  }

  for(int i = 0; i < (dispSettings.maxRPM+1); i++)
  {
    u8g2.drawLine((coef*i)+0.5+(coef/2)+0.5, 8, (coef*i)+0.5+coef/2+0.5, 8);    // 500 rpms markers   
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
  u8g2.drawBox(0, 0, bargraphWidth, 7);

  // Recent Max RPM indication
  u8g2.drawLine(maxRpm, 0, maxRpm, 7);
}


static void OLED_Display_Speed()
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
  u8g2.drawStr( 0, 48 , buff);
  u8g2.setFont( u8g2_font_5x7_tf);
  u8g2.drawStr( 56,28 , "km/h");
}


//Displays current trip distance
static void OLED_Display_TripDistance()
{
  char buff[32];
  
  u8g2.setFont( u8g2_font_5x7_tf);//u8g2_font_6x10_tf);
  sprintf(buff, "Trip %d.%01d", (int)(trip/1000.0), (int)((trip/1000.0)*10.0)%10);  //  dtostrf(speed, 4, 1, buff);
  u8g2.drawStr( 0, 64 , buff);
}

//Displays toal distance parcoured
static void OLED_Display_TotalDistance()
{
  char buff[32];
  
  u8g2.setFont( u8g2_font_5x7_tf);//u8g2_font_6x10_tf);
  sprintf(buff, "Total %d.%01d", (int)total,(int)(total*100)%100);
  u8g2.drawStr( 0,64 , buff);
}


//Displays current time
static void OLED_Display_Time()
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
  u8g2.drawStr( 100,64 , buff);
}


static void OLED_Display_Altitude()
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
  u8g2.drawStr( 100,54 , buff);
}


static void OLED_Display_Gear()
{
  char buff[32];
  
  //Engaged gear display
  sprintf(buff, "N" );
  u8g2.setFont(u8g2_font_helvB18_tf );//u8g2_font_helvB24_tn  );//u8g2_font_freedoomr25_tn );// u8g2_font_logisoso26_tn  );
  u8g2.drawStr( 110, 42 , buff);
}
