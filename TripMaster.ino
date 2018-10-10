#include <Arduino.h>
#include <U8g2lib.h>
#include <TimeLib.h> 
#include <TinyGPS++.h>
#include <SoftwareSerial.h>

#include "logos.h"
/*
#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif*/

U8G2_SSD1309_128X64_NONAME0_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ D5, /* data=*/ D7, /* cs=*/ D8, /* dc=*/ D3, /* reset=*/ D4);  


// The serial connection to the GPS device
static const int RXPin = D2, TXPin = D1;
static const uint32_t GPSBaud = 57600;
SoftwareSerial ss(RXPin, TXPin);
TinyGPSPlus gps;

  
float rpm;
int maxRpm;
float spd;

int alt;
  
int hours;
int minutes;
  
float trip = 1234.5;
float total = 12345.6;
 
int test;

#define FILTER_PERIOD_MS  1
#define CYCLE_PERIOD_MS   1

static void smartDelay(unsigned long ms);

  
void setup(void) {
  u8g2.begin();

  Serial.begin(115200);
  
  u8g2.clearBuffer();
  u8g2.drawXBM(0, 0, 128, 64, logo_suzuki_bits);

  ss.begin(GPSBaud);

  u8g2.sendBuffer();
  delay(3000);

}


void loop(void) 
{
  static bool timeSet = false;
  
  // picture loop  
  u8g2.clearBuffer();
  char buff[32];


  int toto = random(128);
  rpm =  rpm * ((float)FILTER_PERIOD_MS / ((float)CYCLE_PERIOD_MS + (float)FILTER_PERIOD_MS)) +  toto * ((float)CYCLE_PERIOD_MS / ((float)CYCLE_PERIOD_MS + (float)FILTER_PERIOD_MS));

  if(rpm > maxRpm)
  {
    maxRpm = rpm;
  }
  else
  {
    maxRpm-=2;
    if(maxRpm <0)
    {
      maxRpm = 0;
    }
  }
 /*
  if(rpm == 128)
  {
    test = -1;
  }
  else if(rpm == 0)
  {
    test = 1;
  }
  
  rpm += test;*/

  
  if(!timeSet)
  {
    if(gps.date.isValid() && gps.time.isValid())
    {
      int Year = gps.date.year();
      byte Month = gps.date.month();
      byte Day = gps.date.day();
      byte Hour = gps.time.hour();
      byte Minute = gps.time.minute();
      byte Second = gps.time.second();
    
      // Set Time from GPS data string
      setTime(Hour, Minute, Second, Day, Month, Year);
      // Calc current Time Zone time by offset value
      adjustTime(2 * 3600);   
      timeSet = true;   
    }
  }
  
  hours = hour();
  minutes = minute();
       
  

  spd = random(125);//gps.speed.kmph();
  alt = gps.altitude.meters();



  

 // RPM display
 u8g2.drawLine(0, 18, 127, 18);
 
 for(int i = 0; i< 7; i++)
 {
    u8g2.drawLine(16*(i+1), 8, 16*(i+1), 10);
    sprintf(buff, "%d", i+1);
    u8g2.setFont( u8g2_font_micro_tr);
    u8g2.drawStr( 16*(i+1)-1,17 , buff);
 }
 
 for(int i = 0; i< 9; i++)
 {
    u8g2.drawLine(16*i+8, 8, 16*i+8, 8);
 }

 u8g2.drawBox(0,0, (int)rpm, 7);
 u8g2.drawLine(maxRpm, 0, maxRpm, 7);


  // Speed Display
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


  // Distance display
  u8g2.setFont( u8g2_font_5x7_tf);//u8g2_font_6x10_tf);
  sprintf(buff, "Trip %d.%01d", (int)trip, (int)(trip*10)%10);  //  dtostrf(speed, 4, 1, buff);
  u8g2.drawStr( 0, 62 , buff);
  /*sprintf(buff, "Total %d.%01d", (int)total,(int)(total*100)%100);
  u8g2.drawStr( 0,64 , buff);*/


  // Misc display
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
  
  if(gps.altitude.isValid())
  {
    sprintf( buff, "%4dm",alt);
  }
  else
  {
    sprintf( buff, "----m");
  }
  u8g2.drawStr( 100,54 , buff);


  //Engaged gear display
  sprintf(buff, "%d", 5 );
  u8g2.setFont(u8g2_font_profont29_tn);//u8g2_font_helvB24_tn  );//u8g2_font_freedoomr25_tn );// u8g2_font_logisoso26_tn  );
  u8g2.drawStr( 110, 42 , buff);



  u8g2.sendBuffer();
  smartDelay(20);
}


// This custom version of delay() ensures that the gps object
// is being "fed".
static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    while (ss.available())
      gps.encode(ss.read());
  } while (millis() - start < ms);
}
