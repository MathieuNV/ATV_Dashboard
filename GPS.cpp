//----------------------------------------------------------------------------- 
/** 
 *  
 * \file GPS.cpp
 * \brief Management of GPS data
 * \author M.Navarro
 * \date 10/2018
 *
 * Original GPS Module used: vk2828u7g5lf
 * Wired on hardware serial port, talking @ 921600bds
 * RPM signal must be wired to an pin; Computation then done by interrupt, 
 * counting time period of each rev of the engine.
 */
//-----------------------------------------------------------------------------
// (c) Copyright MN 2018 - All rights reserved
//-----------------------------------------------------------------------------


//---------------------------------------------
// Include 
//---------------------------------------------
#include "GPS.h"
#include "File.h"
#include "GPIO.h"
#include "Settings.h"
#include <HardwareSerial.h>


//---------------------------------------------
// Defines
//---------------------------------------------
#define   GPS_SPEED_THRSLD      2.0       ///< min speed to compute trip distance (in km/h)

#define   FILTER_PERIOD_MS      1
#define   CYCLE_PERIOD_MS       1

#define   GPS_TIMER_PERIOD_US   1000000   ///< Timer interrupt period; at each interrupt, log GPS data

#define   TRIP_RECORD_DELAY_MS  20000     ///< Delay after fist fix to start data record (in ms)

#define   GPS_CHECK_PERIOD_MS   1000      ///< Process count of bytes received from GPS at this period, to check if connected


//---------------------------------------------
// Enum, struct, union
//---------------------------------------------


//---------------------------------------------
// Variables
//---------------------------------------------
double previousLatitude;
double previousLongitude;

static const uint32_t GPSBaud = 921600;
HardwareSerial GPS_Serial(1);
TinyGPSPlus gps;
  
float rpm;
  
double trip = 0.0;
double total = 12345.6;

portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

volatile int interruptCounter = 0;
volatile int rpmCounter = 0;
volatile long microsRPM = 9999;
volatile bool recordISRappened = false;

bool firstFixDone = false;
bool recordTrip = false;
long firstFixMillis = 0;    ///< time in ms at which fix has been done

char filename[32]; 

// For external RPM interrupt
portMUX_TYPE extISRmux = portMUX_INITIALIZER_UNLOCKED;

// Timer interrupt, te refresh rpm value
hw_timer_t * gpsTimer = NULL;
portMUX_TYPE gpsTimerMux = portMUX_INITIALIZER_UNLOCKED;

gpsHistory_str gpsHistory;


//---------------------------------------------
// Public Functions
//---------------------------------------------
void GPS_Init();
void GPS_Process();
void GPS_Delay(unsigned long ms);


//---------------------------------------------
// Private Functions
//---------------------------------------------


//---------------------------------------------
// Functions declarations
//---------------------------------------------

 
void IRAM_ATTR externalISR() 
{
  portENTER_CRITICAL_ISR(&extISRmux);
  interruptCounter++;  
  rpmCounter = micros() - microsRPM;
  microsRPM = micros();
  portEXIT_CRITICAL_ISR(&extISRmux);
}


void IRAM_ATTR gpsTimerISR() 
{  
  portENTER_CRITICAL_ISR(&gpsTimerMux);

  if(recordTrip)
  {
    recordISRappened = true;
    
    
    /*if(gps.location.isValid())  //if(gps.speed.kmph() >= GPS_SPEED_THRSLD)
    {
      gpsHistory.lat[gpsHistory.pointsIndex] = gps.location.lat();
      gpsHistory.lng[gpsHistory.pointsIndex] = gps.location.lng();
      
      if(gps.speed.isValid())
      {
        gpsHistory.spd[gpsHistory.pointsIndex] = gps.speed.kmph();
      }
      
      if(gps.altitude.isValid())
      {
        gpsHistory.alt[gpsHistory.pointsIndex] = gps.altitude.meters(); 
      }
      
      gpsHistory.pointsIndex++;
    }*/
  }
  portEXIT_CRITICAL_ISR(&gpsTimerMux);
}


//---------------------------------------------
/// \fn void GPS_Init(void)
///
/// \brief Init GPS Elements.
/// \param None.
/// \return None.
void GPS_Init()
{
  GPS_Serial.begin(GPSBaud, SERIAL_8N1, 16, 17);

  pinMode(PIN_RPM_INPUT, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIN_RPM_INPUT), externalISR, FALLING);
  
  gpsTimer = timerBegin(0, 80, true);
  timerAttachInterrupt(gpsTimer, &gpsTimerISR, true);
  timerAlarmWrite(gpsTimer, GPS_TIMER_PERIOD_US, true);
  timerAlarmEnable(gpsTimer);
}


void GPS_Process()
{
  char buff[64];
  int toto = random(9000);
  rpm =  rpm * ((float)FILTER_PERIOD_MS / ((float)CYCLE_PERIOD_MS + (float)FILTER_PERIOD_MS)) +  toto * ((float)CYCLE_PERIOD_MS / ((float)CYCLE_PERIOD_MS + (float)FILTER_PERIOD_MS));

  // If count on period
  //rpm = (rpmCounter * 1000000.0 / RPM_TIMER_PERIOD_US) * 60.0;

  // If measure period for each rpm
  //rpm = (1000000.0/rpmCounter) * 60.0;

  // Wait everything ok to consider fix done
  if(!firstFixDone)
  {
     if( gps.location.isValid() && gps.date.isValid()  && gps.time.isValid()     
      && gps.altitude.isValid() && gps.speed.isValid() && gps.satellites.isValid())
    {
      firstFixDone = true;

      // First location
      previousLatitude  = gps.location.lat();
      previousLongitude = gps.location.lng();

      // Set Time from GPS data string. Will be then updated by ESP32 RTC functions
      setTime(gps.time.hour(), gps.time.minute(), gps.time.second(), gps.date.day(), gps.date.month(), gps.date.year());
      // Calc current Time Zone time by offset value
      adjustTime(1 * 3600);   
      
      firstFixMillis = millis();
    }
  }

  if(!recordTrip)
  {
    if(firstFixDone && (millis() >= (firstFixMillis + TRIP_RECORD_DELAY_MS)) )
    {
      recordTrip = true;
      
      // Creates File to log GPS Data
      sprintf( filename, "/%04d%02d%02d_%02d%02d%002d.csv", year(), month(), day(), hour(), minute(), second());
    
      // = String(year()) + String(month()) +  String(day()) + "_"+ String(hour()) + String(minute()) + String(second()) +".csv";
      Serial.print(filename);
      File_Write( fileSystem, filename, "sep=,\n");
      File_Append(fileSystem, filename, "Latitude, Longitude, Altitude, Speed\n");
    }
  }

  if(recordISRappened)
  {
    gpsHistory.lat[gpsHistory.pointsIndex] = gps.location.lat();
    gpsHistory.lng[gpsHistory.pointsIndex] = gps.location.lng();
    
    gpsHistory.spd[gpsHistory.pointsIndex] = gps.speed.kmph();
    gpsHistory.alt[gpsHistory.pointsIndex] = gps.altitude.meters(); 

    sprintf( buff, "%f, %f, %d, %d\n", gps.location.lat(), gps.location.lng(), (int)gps.altitude.meters(), (int)gps.speed.kmph());
    File_Append(fileSystem, filename, buff);
    recordISRappened = false;
    gpsHistory.pointsIndex++;
  }

  // Trip distance computation; add elapsed distance since previous point
  if(gps.location.isValid())
  {
    if(gps.speed.kmph() >= GPS_SPEED_THRSLD) // Only compute if speed considered as not null
    {
       trip += gps.distanceBetween( gps.location.lat(), gps.location.lng(), previousLatitude, previousLongitude);
       previousLatitude = gps.location.lat();
       previousLongitude = gps.location.lng();
    }
  }
}
  

// This custom version of delay() ensures that the gps object
// is being "fed".
void GPS_Delay(unsigned long ms)
{
  static long checkDataTime;
  unsigned long start = millis();
  
  do 
  {
    while (GPS_Serial.available())
    {
      gps.encode(GPS_Serial.read());
    }
  } while (millis() - start < ms);

  if(millis() > (checkDataTime + GPS_CHECK_PERIOD_MS))
  {
    if(gps.charsProcessed() < 10)
    {
      // Diagnose GPS problem
      //GPIO_LedSet(true);
    }
    else
    {
      //GPIO_LedSet(false);
    }
    checkDataTime = millis();
  } 
}
