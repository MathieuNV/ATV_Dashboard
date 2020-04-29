//----------------------------------------------------------------------------- 
/** 
 *  
 * \file Settings.cpp
 * \brief Management of settings
 * \author M.Navarro
 * \date 10/2018
 *
 * Uses 
 */
//-----------------------------------------------------------------------------
// (c) Copyright MN 2018 - All rights reserved
//-----------------------------------------------------------------------------


//---------------------------------------------
// Include 
//---------------------------------------------
#include <Preferences.h>
#include "Settings.h"
#include "Web_Server.h"


//---------------------------------------------
// Defines
//---------------------------------------------


//---------------------------------------------
// Enum, struct, union
//---------------------------------------------


//---------------------------------------------
// Variables
//---------------------------------------------
s_settings settings;
Preferences prefs;


//---------------------------------------------
// Public Functions
//---------------------------------------------
void Settings_Init();

void Settings_Save();
void Settings_Load();

void Settings_LEDS_EnableToggle();
int Settings_LEDS_IsEnabled();

void Settings_WebServer_EnableToggle();
int Settings_WebServer_IsEnabled();

int Settings_MaxRpm();
void Settings_MaxRpm_Dec();
void Settings_MaxRpm_Inc();

int Settings_LedBrightness();
void Settings_LedBrightness_Dec();
void Settings_LedBrightness_Inc();

int Settings_BrandLogo();
void Settings_BrandLogo_Dec();
void Settings_BrandLogo_Inc();

int Settings_MainDisplayStyle();
void Settings_MainDisplayStyle_Dec();
void Settings_MainDisplayStyle_Inc();

char* Settings_SoftVersion();

//---------------------------------------------
// Private Functions
//---------------------------------------------


//---------------------------------------------
// Functions declarations
//---------------------------------------------

void Settings_Init()
{
  prefs.begin("saveData");
    
  settings.maxRPM = 8000;
  settings.ledBrightness = 3;
  settings.ledEnabled = false;
  settings.webServerEnable = false;
  settings.brandLogo = 0;
  settings.mainDisplayStyle = 0;
   
  Settings_Load();

  settings.softVersion = "1.01.A";
}

void Settings_Save()
{
  prefs.putBytes("saveData", &settings, sizeof(settings)); 
}
  
void Settings_Load()
{
  size_t dataLen = prefs.getBytesLength("saveData");
  
  Serial.print("nb d'octets lus ");
  Serial.println(dataLen);
  
  char buffer[dataLen];
  prefs.getBytes("saveData", buffer, dataLen);
  
  if(dataLen != sizeof(s_settings))
  {
    Serial.print("Data is not correct size (expected ");
    Serial.print(sizeof(s_settings));
    Serial.println(" bytes). Re-save data.");
    Settings_Save();
    return;
  }
  s_settings *temp = (s_settings *) buffer;

  settings.maxRPM = temp->maxRPM; 
  settings.ledEnabled = temp->ledEnabled;
  settings.ledBrightness = temp->ledBrightness;
  settings.webServerEnable = temp->webServerEnable;
  settings.mainDisplayStyle = temp->mainDisplayStyle;
  settings.brandLogo = temp->brandLogo;
}


void Settings_LEDS_EnableToggle()
{
  settings.ledEnabled = !settings.ledEnabled;
  Settings_Save();
}


int Settings_LEDS_IsEnabled()
{
  return (int)settings.ledEnabled;
}


void Settings_WebServer_EnableToggle()
{
  if( settings.webServerEnable)
  {
    settings.webServerEnable = false;
    WebServer_Stop();
  }
  else
  {
    settings.webServerEnable = true;
    WebServer_Init();
  }
}

int Settings_WebServer_IsEnabled()
{
  return (int)settings.webServerEnable;
}

int Settings_MaxRpm()
{
  return settings.maxRPM;
}


void Settings_MaxRpm_Dec()
{
  settings.maxRPM -= 1000;
  
  if(settings.maxRPM < MAXRPM_MIN)
  {
    settings.maxRPM = MAXRPM_MIN;
  }
}


void Settings_MaxRpm_Inc()
{
  settings.maxRPM += 1000;
  
  if(settings.maxRPM > MAXRPM_MAX)
  {
    settings.maxRPM = MAXRPM_MAX;
  }
}


int Settings_LedBrightness()
{
  return settings.ledBrightness;
}

void Settings_LedBrightness_Dec()
{
  settings.ledBrightness -= 5;
  
  if(settings.ledBrightness < LEDBRIGHTNESS_MIN)
  {
    settings.ledBrightness = LEDBRIGHTNESS_MIN;
  }
}

void Settings_LedBrightness_Inc()
{
  settings.ledBrightness += 5;
  
  if(settings.ledBrightness > LEDBRIGHTNESS_MAX)
  {
    settings.ledBrightness = LEDBRIGHTNESS_MAX;
  }
}


int Settings_BrandLogo()
{
  return settings.brandLogo;
}


void Settings_BrandLogo_Dec()
{
  settings.brandLogo -= 1;
  
  if(settings.brandLogo < 0)
  {
    settings.brandLogo = 0;
  }
}


void Settings_BrandLogo_Inc()
{
  settings.brandLogo += 1;
  
  if(settings.brandLogo > (NB_OF_LOGOS-1))
  {
    settings.brandLogo = NB_OF_LOGOS-1;
  }
}

int Settings_MainDisplayStyle()
{
  return settings.mainDisplayStyle;
}

void Settings_MainDisplayStyle_Dec()
{
  settings.mainDisplayStyle -= 1;
  
  if(settings.mainDisplayStyle < 0)
  {
    settings.mainDisplayStyle = 0;
  }
}

void Settings_MainDisplayStyle_Inc()
{
  settings.mainDisplayStyle += 1;
  
  if(settings.mainDisplayStyle > (1))
  {
    settings.mainDisplayStyle = 1;
  }
}

char* Settings_SoftVersion()
{
  return settings.softVersion;
}
