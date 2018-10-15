#include "GPS.h"
#include <TimeLib.h> 
#include <HardwareSerial.h>

#define GPS_SPEED_THRSLD 2.0


static const uint32_t GPSBaud = 921600;
HardwareSerial GPS_Serial(1);
TinyGPSPlus gps;
  
float rpm;
int maxRpm;
float spd;

int alt;
  
int hours;
int minutes;
  
double trip = 0.0;
double total = 12345.6;
 
int test;

#define FILTER_PERIOD_MS  1
#define CYCLE_PERIOD_MS   1


void GPS_Init();
void GPS_Process();
void GPS_Delay(unsigned long ms);

double previousLatitude;
double previousLongitude;


void GPS_Init()
{
  Serial.begin(115200);
  GPS_Serial.begin(GPSBaud, SERIAL_8N1, 16, 17);
}


void GPS_Process()
{
  static bool timeSet = false;
  static bool firstLocationSet = false;

  int toto = random(128);
  rpm =  rpm * ((float)FILTER_PERIOD_MS / ((float)CYCLE_PERIOD_MS + (float)FILTER_PERIOD_MS)) +  toto * ((float)CYCLE_PERIOD_MS / ((float)CYCLE_PERIOD_MS + (float)FILTER_PERIOD_MS));
  rpm = 10;
  

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

  if(!firstLocationSet)
  {
    if(gps.location.isValid())
    {
      previousLatitude = gps.location.lat();
      previousLongitude = gps.location.lng();
      firstLocationSet = true;
    }
  }


  
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
       
  

  spd = gps.speed.kmph();
  alt = gps.altitude.meters();


  if(gps.location.isValid())
  {
    if(gps.speed.kmph() >= GPS_SPEED_THRSLD)
    {
       trip += gps.distanceBetween( gps.location.lat(), gps.location.lng(), previousLatitude, previousLongitude);
       previousLatitude = gps.location.lat();
       previousLongitude = gps.location.lng();
    }
  }
  //trip = 1256235.562;
}
  

// This custom version of delay() ensures that the gps object
// is being "fed".
void GPS_Delay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    while (GPS_Serial.available())
      gps.encode(GPS_Serial.read());
  } while (millis() - start < ms);
}


