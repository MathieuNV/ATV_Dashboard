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
#include "GPS.h"


//---------------------------------------------
// Defines
//---------------------------------------------
#define   TOTAL_DISTANCE_DISPLAY_DURATION_MS  10000     ///< Duraiton of the diaplay of the toal distance parcoured at startup, in ms

#define   SCREEN_WIDTH                        128       ///< In pixels
#define   SCREEN_HEIGHT                       64        ///< in pixels

#define   MAX_MENU_ITEMS                      10
#define   MAX_DISPLAY_ITEMS                   5


//---------------------------------------------
// Enum, struct, union
//---------------------------------------------
typedef void (*funPtr)(void);
typedef int (*funPtrInt)(void);

typedef enum 
{
  MAIN_SCREEN = 0,
  MENU_MAIN, 
  MENU_LEDS_SETTINGS, 
  MENU_SETTINGS,
  MENU_MEMORY,
  MENU_SETTINGS_SETMAXRPM,
  MENU_LED_BRIGHTNESS,
  MENU_SETTINGS_BRANDLOGO,
  MENU_SETTINGS_MAINDISPLAYSTYLE,
  MENU_MEMORY_SHOWSIZE,
}e_DispState;


typedef enum 
{
  VOID,
  RETURN,
  FUNCTION,
  VALUE_BOOL,
  VALUE_INT,
  SUBMENU
}e_MenuItemType;


typedef struct
{
  char* name;
  e_MenuItemType type = VOID;
  funPtr func = NULL;
  funPtrInt ptrValue = NULL;  
}s_MenuItem;


typedef struct
{
  char *header;
  funPtr previousMenu = NULL;
  s_MenuItem items[MAX_MENU_ITEMS];
  int currentSelection = 0;
  int firstDisplayElement = 0;
  int nbElements;
}s_Menu;


typedef struct
{
  char* name;
  const uint8_t * image;
}s_logo;

//---------------------------------------------
// Public variables
//---------------------------------------------


//---------------------------------------------
// Public Functions
//---------------------------------------------
extern void OLED_Init();
extern void OLED_Handle();


#endif
