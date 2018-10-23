//----------------------------------------------------------------------------- 
/** 
 *  
 * \file Display.h
 * \brief Human/User Interface header file
 * \author M.Navarro
 * \date 10/2018
 *
 */
//-----------------------------------------------------------------------------
// (c) Copyright MN 2018 - All rights reserved
//-----------------------------------------------------------------------------
#ifndef _DISPLAY_H
#define _DISPLAY_H

//---------------------------------------------
// Include 
//---------------------------------------------
#include <TinyGPS++.h>


//---------------------------------------------
// Defines
//---------------------------------------------
#define   SPLASH_LOGO_DURATION_MS             3000      ///< Duration of the brand logo displayed at boot, in ms
#define   TOTAL_DISTANCE_DISPLAY_DURATION_MS  10000     ///< Duraiton of the diaplay of the toal distance parcoured at startup, in ms

#define   SCREEN_WIDTH                        128       ///< In pixels
#define   SCREEN_HEIGHT                       64        ///< in pixels


//---------------------------------------------
// Enum, struct, union
//---------------------------------------------
typedef struct
{
  int maxRPM;   ///< Max rpm displayed (/1000; 7 = 7000 rpm max)
}dispSettings_str;


//---------------------------------------------
// Type
//---------------------------------------------


//---------------------------------------------
// Public variables
//---------------------------------------------


//---------------------------------------------
// Public Functions
//---------------------------------------------
extern void OLED_Init();
extern void OLED_Draw();

#endif
