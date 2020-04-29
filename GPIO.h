//----------------------------------------------------------------------------- 
/** 
 *  
 * \file GPIO.h
 * \brief Gpio header file
 * \author M.Navarro
 * \date 11/2019
 *
 */
//-----------------------------------------------------------------------------
// (c) Copyright MN 2019 - All rights reserved
//-----------------------------------------------------------------------------
#ifndef _GPIO_H
#define _GPIO_H

//---------------------------------------------
// Include 
//---------------------------------------------
#include <Arduino.h>


//---------------------------------------------
// Defines
//---------------------------------------------
#define   BP_RIGHT_UP     0
#define   BP_RIGHT_DOWN   1
#define   BP_LEFT_UP      2
#define   BP_LEFT_DOWN    3
#define   NB_OF_BUTTONS   4


//---------------------------------------------
// Enum, struct, union
//---------------------------------------------


//---------------------------------------------
// Type
//---------------------------------------------


//---------------------------------------------
// Public variables
//---------------------------------------------


//---------------------------------------------
// Public Functions
//---------------------------------------------
extern void GPIO_Init();
extern void GPIO_Handle();

extern bool GPIO_IsButtonPressed(int buttonIndex);
extern bool GPIO_IsButtonClicked(int buttonIndex);
/*extern void GPIO_LedToggle();
extern void GPIO_LedSet(bool state);*/

#endif
