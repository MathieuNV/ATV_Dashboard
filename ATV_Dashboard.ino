//----------------------------------------------------------------------------- 
/** 
 *  
 * \file TripMaster_ESP32.ino
 * \brief Main file of the project
 * \author M.Navarro
 * \date 10/2018
 *
 * ATV / Bike GPS based dashboard. 
 * libraries necessary:
 *  -U8G2, for display
 *  -TinyGPSPlus, for GPS data parsing https://github.com/mikalhart/TinyGPSPlus
 *  -TimeLib https://github.com/PaulStoffregen/Time
 * Currently runs on ESP32, but can be compiled for other targets. 
 * 
 */
//-----------------------------------------------------------------------------
// (c) Copyright MN 2018 - All rights reserved
//-----------------------------------------------------------------------------


//---------------------------------------------
// Include 
//---------------------------------------------
#include <Arduino.h>

#include "Settings.h"
#include "GPS.h"
#include "Display.h"
#include "File.h"
#include "Web_Server.h"
#include "GPIO.h"
#include "Leds.h"


//---------------------------------------------
// Variables
//---------------------------------------------


//---------------------------------------------
// Functions declarations
//---------------------------------------------
 
//---------------------------------------------
/// \fn void setup(void)
///
/// \brief Arduino Setup mandatory function. Executed Once.
/// \param None.
/// \return None.
void setup(void)
{
  Serial.begin(115200);

  Settings_Init();

  GPIO_Init();
  LEDS_Init();
  
  GPS_Init();
  OLED_Init();

  File_Init();
  WebServer_Init();
}


//---------------------------------------------
/// \fn void loop(void)
///
/// \brief Arduino Loop mandatory function. Executed Repeatedly.
/// \param None.
/// \return None.
void loop(void) 
{
  GPIO_Handle();
  
  GPS_Process();

  OLED_Handle();
  
  LEDS_Handle();
  
  GPS_Delay(0);

  WebServer_Handle();
}
