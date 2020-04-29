//----------------------------------------------------------------------------- 
/** 
 *  
 * \file Leds.h
 * \brief Leds header file
 * \author M.Navarro
 * \date 01/2020
 * 
 */
#ifndef _LEDS_H
#define _LEDS_H

//---------------------------------------------
// Include 
//---------------------------------------------
#include <Arduino.h>


//---------------------------------------------
// Defines
//---------------------------------------------


//---------------------------------------------
// Enum, struct, union
//---------------------------------------------
typedef enum
{
  E_LedMode_Off,
  E_LedMode_RPM,
  E_LedMode_Warning,
  E_LedMode_TurnLeft,
  E_LedMode_TurnRight
}e_ledMode;


//---------------------------------------------
// Type
//---------------------------------------------


//---------------------------------------------
// Public variables
//---------------------------------------------


//---------------------------------------------
// Public Functions
//---------------------------------------------
extern void LEDS_Init();
extern void LEDS_Handle();

extern void LEDS_Mode_Off();
extern void LEDS_Mode_RPM();


#endif
