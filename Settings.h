//----------------------------------------------------------------------------- 
/** 
 *  
 * \file Settings.h
 * \brief Settings header file
 * \author M.Navarro
 * \date 01/2020
 * 
 */
#ifndef _SETTINGS_H
#define _SETTINGS_H

//---------------------------------------------
// Include 
//---------------------------------------------
#include <Arduino.h>


//---------------------------------------------
// Defines
//---------------------------------------------
//#define   SIMU_TEST_GPS

#define   SPLASH_LOGO_DURATION_MS    3000      ///< Duration of the brand logo displayed at boot, in ms

#define   FAST_BLINK_PERIOD 50
#define   SLOW_BLINK_PERIOD 333

#define   PIN_OLED_CLOCK    18
#define   PIN_OLED_DATA     23
#define   PIN_OLED_CS       5
#define   PIN_OLED_DC       19
#define   PIN_OLED_RESET    22

#define   PIN_LEDS          0

#define   PIN_RPM_INPUT     21        ///< Pin used for external RPM calculation



#define MAXRPM_MIN          1000
#define MAXRPM_MAX          15000

#define LEDBRIGHTNESS_MIN   3
#define LEDBRIGHTNESS_MAX   50

#define NB_OF_LOGOS         6

  
//---------------------------------------------
// Enum, struct, union
//---------------------------------------------
typedef struct
{
  int maxRPM;     ///< Max rpm displayed 
  int ledEnabled;
  int ledBrightness;
  bool webServerEnable;
  int mainDisplayStyle;
  int brandLogo;
  char* softVersion;
}s_settings;


//---------------------------------------------
// Type
//---------------------------------------------


//---------------------------------------------
// Public variables
//---------------------------------------------
extern s_settings settings;


//---------------------------------------------
// Public Functions
//---------------------------------------------
extern void Settings_Init();
extern void Settings_Save();

extern void Settings_LEDS_EnableToggle();
extern int Settings_LEDS_IsEnabled();

extern void Settings_WebServer_EnableToggle();
extern int Settings_WebServer_IsEnabled();

extern int Settings_MaxRpm();
extern void Settings_MaxRpm_Dec();
extern void Settings_MaxRpm_Inc();

extern int Settings_LedBrightness();
extern void Settings_LedBrightness_Dec();
extern void Settings_LedBrightness_Inc();

extern int Settings_BrandLogo();
extern void Settings_BrandLogo_Dec();
extern void Settings_BrandLogo_Inc();

extern int Settings_MainDisplayStyle();
extern void Settings_MainDisplayStyle_Dec();
extern void Settings_MainDisplayStyle_Inc();

extern char* Settings_SoftVersion();

#endif
