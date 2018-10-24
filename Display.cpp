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
//#define DEBUG

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

  
//---------------------------------------------
// Public Functions
//---------------------------------------------
void OLED_Init();
void OLED_Draw();


//---------------------------------------------
// Private Functions
//---------------------------------------------
// Differents screen that can be displayed
static void OLED_Screen_Main(int vOffset);
static void OLED_Screen_Track(int vOffset);
static void OLED_Screen_Stats(int vOffset);

// Basic Elements composing main screens 
static void OLED_Display_RPM(int xPos, int yPos);
static void OLED_Display_Speed(int xPos, int yPos);
static void OLED_Display_TripDistance(int xPos, int yPos);
static void OLED_Display_TotalDistance(int xPos, int yPos);
static void OLED_Display_Time(int xPos, int yPos);
static void OLED_Display_Altitude(int xPos, int yPos);
static void OLED_Display_Gear(int xPos, int yPos);
static void OLED_Display_Track(int xPos, int yPos, int width, int heigth, double *xDataArray, double *yDataArray, int dataSize);
static void OLED_Display_History(int xPos, int yPos, int width, int heigth, int * dataArray, int dataSize, char * chartName);

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
  //Serial.begin(115200);
  u8g2.begin();

  u8g2.clearBuffer();
  u8g2.drawXBM(0, 0, 128, 64, logo_suzuki_bits);// logo_suzuki_bits);
  u8g2.sendBuffer();

  oledTimer = timerBegin(1, 80, true);
  timerAttachInterrupt(oledTimer, &oledTimerISR, true);
  timerAlarmWrite(oledTimer, OLED_TIMER_PERIOD_US, true);
  timerAlarmEnable(oledTimer);

#ifdef DEBUG
  for(int i = 0; i < 2038; i++)
  {
    gpsHistory.lng[i] = titi[i*2];
    gpsHistory.lat[i] = titi[i*2+1];
    gpsHistory.alt[i] = tete[i];
    gpsHistory.spd[i] = tete[i];
    gpsHistory.pointsIndex++;
  }
#endif
}


void OLED_Draw()
{  
  static int scrollingOnGoing = 0;
  static int currentScreenNb = 0;
  static void (*currentScreen)(int) = OLED_Screen_Main;
  static void (*nextScreen)(int) = OLED_Screen_Track;
  
   // Display main Screen only if splash logo finished 
  if(millis() > SPLASH_LOGO_DURATION_MS)
  {
    u8g2.clearBuffer();

    if(oledTimerTick)
    {
      oledTimerTick = 0;
      scrollingOnGoing = 1;
    }

    if(scrollingOnGoing)
    {
      if(OLED_Scroll_Screens(currentScreen, nextScreen))
      {
        scrollingOnGoing = 0;
        currentScreenNb++;
        if(currentScreenNb > 2)
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
         nextScreen = OLED_Screen_Track;
      }
      else if(currentScreenNb == 1)
      {
        currentScreen = OLED_Screen_Track;
        nextScreen = OLED_Screen_Stats;
      }
      else
      {
        currentScreen = OLED_Screen_Stats;
        nextScreen = OLED_Screen_Main;
      }

      // Draw currentScreen
      currentScreen(0);
    }
 
    u8g2.sendBuffer();
  }
}


static int OLED_Scroll_Screens(void (*initScreen)(int), void (*finalScreen)(int)  )
{
  static int offset = 0;

  initScreen(offset);
  finalScreen(offset-SCREEN_HEIGHT);
 
  offset += 16;
  
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


static void OLED_Screen_Main(int vOffset)
{
  OLED_Display_RPM(0, vOffset+0);
  OLED_Display_Speed(0, vOffset+48);

  // Displays total distance for some seconds at startup, then actual trip distance
  if((millis() - SPLASH_LOGO_DURATION_MS) < TOTAL_DISTANCE_DISPLAY_DURATION_MS)
  { 
    OLED_Display_TotalDistance(0, vOffset+64);
  }
  else
  {
    OLED_Display_TripDistance(0, vOffset+64);
  }

  OLED_Display_Time(100, vOffset+64);
  
  OLED_Display_Altitude(100, vOffset+54); 

  OLED_Display_Gear(110, 42);
}


static void OLED_Screen_Track(int vOffset)
{
  u8g2.drawFrame(0,vOffset,128,64);
  OLED_Display_Track(0, vOffset, 128, 64, gpsHistory.lng, gpsHistory.lat, gpsHistory.pointsIndex);
}


static void OLED_Screen_Stats(int vOffset)
{
  OLED_Display_History(20, 5+vOffset, 100, 25, gpsHistory.alt, gpsHistory.pointsIndex, "Alt");
  OLED_Display_History(20, 35+vOffset, 100, 25, gpsHistory.spd, gpsHistory.pointsIndex, "Spd");
}


// Displays actual RPM, at the top of the screen.
static void OLED_Display_RPM(int xPos, int yPos)
{
  char buff[32];
  float coef = (float)SCREEN_WIDTH / (float)dispSettings.maxRPM;
  int bargraphWidth;

  u8g2.drawLine(xPos, yPos+18, SCREEN_WIDTH, yPos+18);

  // Cerates bargraph legend
  for(int i = 0; i < (dispSettings.maxRPM-1); i++)
  {
    u8g2.drawLine(xPos+((coef*(i+1))+0.5), yPos+8, xPos + ((coef*(i+1))+0.5), yPos+10);   // 500 rpms markers
    sprintf(buff, "%d", i+1);
    u8g2.setFont( u8g2_font_micro_tr);
    u8g2.drawStr( xPos + (coef*(i+1)-1),yPos+17 , buff);          //*1000 rpm numbers
  }

  for(int i = 0; i < (dispSettings.maxRPM+1); i++)
  {
    u8g2.drawLine(xPos + ((coef*i)+0.5+(coef/2)+0.5), yPos+8, xPos +((coef*i)+0.5+coef/2+0.5), yPos+8);    // 500 rpms markers   
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
  u8g2.drawBox(xPos+0, yPos+0, bargraphWidth, 7);

  // Recent Max RPM indication
  u8g2.drawLine(xPos + maxRpm, yPos+0, xPos + maxRpm, yPos+7);
}


static void OLED_Display_Speed(int xPos, int yPos)
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
  u8g2.drawStr( xPos, yPos , buff);
  u8g2.setFont( u8g2_font_5x7_tf);
  u8g2.drawStr( xPos+56, yPos-20 , "km/h");
}


//Displays current trip distance
static void OLED_Display_TripDistance(int xPos, int yPos)
{
  char buff[32];
  
  u8g2.setFont( u8g2_font_5x7_tf);//u8g2_font_6x10_tf);
  sprintf(buff, "Trip %d.%01d", (int)(trip/1000.0), (int)((trip/1000.0)*10.0)%10);  //  dtostrf(speed, 4, 1, buff);
  u8g2.drawStr( xPos, yPos, buff);
}

//Displays toal distance parcoured
static void OLED_Display_TotalDistance(int xPos, int yPos)
{
  char buff[32];
  
  u8g2.setFont( u8g2_font_5x7_tf);//u8g2_font_6x10_tf);
  sprintf(buff, "Total %d.%01d", (int)total,(int)(total*100)%100);
  u8g2.drawStr( xPos, yPos , buff);
}


//Displays current time
static void OLED_Display_Time(int xPos, int yPos)
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
  u8g2.drawStr( xPos, yPos, buff);
}


static void OLED_Display_Altitude(int xPos, int yPos)
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
  u8g2.drawStr( xPos, yPos, buff);
}


static void OLED_Display_Gear(int xPos, int yPos)
{
  char buff[32];
  
  //Engaged gear display
  sprintf(buff, "N" );
  u8g2.setFont(u8g2_font_helvB18_tf );//u8g2_font_helvB24_tn  );//u8g2_font_freedoomr25_tn );// u8g2_font_logisoso26_tn  );
  u8g2.drawStr( xPos, yPos , buff);
}


static void OLED_Display_Track(int xPos, int yPos, int width, int heigth, double *xDataArray, double *yDataArray, int dataSize)
{
  char buff[32];
  double xCartesian, yCartesian;
  int hMargin = 4, vMargin = 4;
  double minX= 180, maxX = -180, minY= 90, maxY = -90;
  double xCoef, yCoef;

  for(int i = 0; i < dataSize; i++)
  {
    if( minY > yDataArray[i])
    {
      minY = yDataArray[i];
    }

    if(maxY < yDataArray[i])
    {
      maxY = yDataArray[i];
    }

    if( minX > xDataArray[i])
    {
      minX = xDataArray[i];
    }

    if( maxX < xDataArray[i])
    {
      maxX = xDataArray[i];
    }
  }
  
  xCoef = maxX - minX;
  yCoef = maxY - minY;

  for(int i = 0; i < dataSize; i++)
  {
    yCartesian = yPos + heigth - vMargin -((yDataArray[i] - minY) / yCoef * (double)(heigth-(vMargin*2)));
    xCartesian = xPos + hMargin +(xDataArray[i] - minX) / xCoef * (double)(width-(hMargin*2));
    
    /*
    yCartesian =  SCREEN_HEIGHT - vMargin - ((yDataArray[i] - minY) / yCoef * (double)(SCREEN_HEIGHT-(vMargin*2)));
    xCartesian =  hMargin + (xDataArray[i] - minX) / xCoef * (double)(SCREEN_WIDTH-(hMargin*2));
*/
    u8g2.drawPixel((int)xCartesian, (int)yCartesian);
  }

   u8g2.drawDisc((int)xCartesian, (int)yCartesian, 2);
}


static void OLED_Display_History(int xPos, int yPos, int width, int heigth, int * dataArray, int dataSize, char * chartName)
{
  char buff[32];
  int indexCoef;
  int valCoef;
  int minVal = 10000, maxVal = 0;
  int hMargin = 2, vMargin = 3;

  int textWidth = 13;

  // Prepare necessary values for plotting
  for(int i = 0; i < dataSize; i++)
  {
    if(dataArray[i] >= maxVal)
    {
      maxVal = dataArray[i];
    }
    else if(dataArray[i] <= minVal)
    {
      minVal = dataArray[i];
    }
  }
  
  valCoef = maxVal - minVal;
  indexCoef = dataSize / (width-2*hMargin-textWidth);
  
  if(indexCoef < 1)
  {
    indexCoef = 1;
  }

  // Display chart axis
  u8g2.drawLine(xPos+textWidth, yPos, xPos+textWidth, yPos+heigth);
  u8g2.drawLine(xPos+textWidth, yPos+heigth, xPos+width, yPos+heigth);
  
  // Display Scale
  u8g2.setFont( u8g2_font_micro_tr);
  sprintf(buff, "%d", (int)maxVal);
  u8g2.drawStr( xPos, yPos+7 , buff);  
  sprintf(buff, "%d", (int)minVal);
  u8g2.drawStr( xPos, yPos+heigth-1 , buff); 

  // Chart Name
  sprintf(buff, "%s", chartName); 
  u8g2.drawStr( xPos-6, yPos+heigth/2+3 , buff); 

  // Plot values
  for(int i = 0; i < dataSize; i++)//width-1; i++)
  {
    u8g2.drawPixel(xPos+i/indexCoef+1+hMargin+textWidth, (yPos+heigth-hMargin-1) - ((float)(dataArray[i]-minVal)/(float)valCoef)*(float)(heigth-1-2*hMargin));
  } 
}

