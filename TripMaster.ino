//----------------------------------------------------------------------------- 
/** 
 *  
 * \file TripMaster_ESP32.ino
 * \brief Main file of the project
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
#include <Arduino.h>

#include "GPS.h"
#include "Display.h"


//---------------------------------------------
// Enum, struct, union
//---------------------------------------------


//---------------------------------------------
// Type
//---------------------------------------------

//---------------------------------------------
// Variables
//---------------------------------------------


//---------------------------------------------
// Public Functions
//---------------------------------------------

//---------------------------------------------
// Private Functions
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
  GPS_Init();
  OLED_Init();
}


//---------------------------------------------
/// \fn void loop(void)
///
/// \brief Arduino Loop mandatory function. Executed Repeatedly.
/// \param None.
/// \return None.
void loop(void) 
{
  GPS_Process();

  OLED_Draw();
  
  GPS_Delay(20);
}


