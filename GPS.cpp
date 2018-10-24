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
#include <TimeLib.h> 
#include <HardwareSerial.h>


//---------------------------------------------
// Defines
//---------------------------------------------
#define   GPS_SPEED_THRSLD    2.0

#define   FILTER_PERIOD_MS    1
#define   CYCLE_PERIOD_MS     1

#define   RPM_INPUT_PIN       13

#define   GPS_TIMER_PERIOD_US 1000000

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
float spd;

int alt;
  
int hours;
int minutes;
  
double trip = 0.0;
double total = 12345.6;

portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

volatile int interruptCounter = 0;
volatile int rpmCounter = 0;
volatile long microsRPM = 9999;

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
  
  if(gps.location.isValid())  //if(gps.speed.kmph() >= GPS_SPEED_THRSLD)
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
  //Serial.begin(115200);
  GPS_Serial.begin(GPSBaud, SERIAL_8N1, 16, 17);
  //Serial.begin(115200);
  pinMode(RPM_INPUT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(RPM_INPUT_PIN), externalISR, FALLING);
  

  gpsTimer = timerBegin(0, 80, true);
  timerAttachInterrupt(gpsTimer, &gpsTimerISR, true);
  timerAlarmWrite(gpsTimer, GPS_TIMER_PERIOD_US, true);
  timerAlarmEnable(gpsTimer);
}


void GPS_Process()
{
  static bool timeSet = false;
  static bool firstLocationSet = false;

  /*int toto = random(6000);
  rpm =  rpm * ((float)FILTER_PERIOD_MS / ((float)CYCLE_PERIOD_MS + (float)FILTER_PERIOD_MS)) +  toto * ((float)CYCLE_PERIOD_MS / ((float)CYCLE_PERIOD_MS + (float)FILTER_PERIOD_MS));
*/
  //rpm = 2500;

  // If count on period
  //rpm = (rpmCounter * 1000000.0 / RPM_TIMER_PERIOD_US) * 60.0;

  // If measure period for each rpm
  rpm = (1000000.0/rpmCounter) * 60.0;
  /*Serial.print(rpmCounter);
  Serial.print(" ");
  Serial.println(rpm);*/


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
    if(gps.date.isValid() && gps.time.isValid() && gps.location.isValid())
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
       
  if(gps.speed.isValid())
  {
    spd = gps.speed.kmph();
  }
  
  if(gps.altitude.isValid())
  {
    alt = gps.altitude.meters();
  }

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


