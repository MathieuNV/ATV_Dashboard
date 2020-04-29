//----------------------------------------------------------------------------- 
/** 
 *  
 * \file Leds.cpp
 * \brief Management of led strip display
 * \author M.Navarro
 * \date 01/2020
 *
 */
//-----------------------------------------------------------------------------
// (c) Copyright MN 2019 - All rights reserved
//-----------------------------------------------------------------------------


//---------------------------------------------
// Include 
//---------------------------------------------
#include "Leds.h"
#include "GPS.h" 
#include "Settings.h" 
#include <FastLED.h>


//---------------------------------------------
// Defines
//---------------------------------------------
#define   NUM_LEDS            12  // Nb of leds of the RGB Strip


//---------------------------------------------
// Enum, struct, union
//---------------------------------------------


//---------------------------------------------
// Variables
//---------------------------------------------
CRGB leds[NUM_LEDS];
e_ledMode ledMode;

long lastTimeBlinkSlow;
long lastTimeBlinkFast;
bool blinkSlow;
bool blinkFast;


//---------------------------------------------
// Public Functions
//---------------------------------------------
void LEDS_Init();
void LEDS_Handle();

void LEDS_Mode_Off();
void LEDS_Mode_RPM();



//---------------------------------------------
// Private Functions
//---------------------------------------------
void LEDS_DisplayRPM(int percent);
void LEDS_Display_TurnLeft();
void LEDS_Display_TurnRight();
void LEDS_Display_Warnings();

void LEDS_AllOff();
void LEDS_AllRed();


//---------------------------------------------
// Functions declarations
//---------------------------------------------
void LEDS_Init()
{
  FastLED.addLeds<TM1809, PIN_LEDS, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(3);

  LEDS_AllOff();
  delay(200);  
  LEDS_AllRed();
  delay(200);    
  LEDS_AllOff();

  ledMode = E_LedMode_RPM;
}


void LEDS_Handle()
{

  if(millis() < SPLASH_LOGO_DURATION_MS)
  {
    return;
  }

  if( millis() > (lastTimeBlinkSlow + SLOW_BLINK_PERIOD))
  {
    blinkSlow = !blinkSlow;
    lastTimeBlinkSlow = millis();
  }

  if( millis() > (lastTimeBlinkFast + FAST_BLINK_PERIOD))
  {
    blinkFast = !blinkFast;
    lastTimeBlinkFast = millis();
  }

  if(settings.ledEnabled)
  {
    switch(ledMode)
    {
      case E_LedMode_RPM:
        LEDS_DisplayRPM((rpm/settings.maxRPM*100.0));
        break;
  
        
      case E_LedMode_Warning:
        LEDS_Display_Warnings();
        break;
  
        
      case E_LedMode_TurnLeft:
        LEDS_Display_TurnLeft();
        break;
  
        
      case E_LedMode_TurnRight:
        LEDS_Display_TurnRight();
        break;
  
      default:
        LEDS_AllOff();      
    }
  }
  else
  {
    LEDS_AllOff();  
  }
}

void LEDS_Mode_Off()
{
  ledMode = E_LedMode_Off;
}

void LEDS_Mode_RPM()
{
  ledMode = E_LedMode_RPM;  
}


void LEDS_Display_TurnLeft()
{
  FastLED.setBrightness(20);
  
  for(int i = 0; i < NUM_LEDS; i++)
  {
    leds[NUM_LEDS-1-i] = CRGB::Black;
  }

  if(blinkSlow)
  {
    leds[NUM_LEDS-1] = CRGB::Green;
    leds[NUM_LEDS-2] = CRGB::Green;//.setRGB( 240, 40, 0);
  }
  
  FastLED.show();
}


void LEDS_Display_TurnRight()
{
  FastLED.setBrightness(20);
  
  for(int i = 0; i < NUM_LEDS; i++)
  {
    leds[NUM_LEDS-1-i] = CRGB::Black;
  }

  if(blinkSlow)
  {
    leds[0] = CRGB::Green;
    leds[1] = CRGB::Green;
  }
  
  FastLED.show();
}


void LEDS_Display_Warnings()
{
  FastLED.setBrightness(20);
  
  for(int i = 0; i < NUM_LEDS; i++)
  {
    leds[NUM_LEDS-1-i] = CRGB::Black;
  }

  if(blinkSlow)
  {
    leds[NUM_LEDS-1] = CRGB::Green;
    leds[NUM_LEDS-2] = CRGB::Green;
    
    leds[0] = CRGB::Orange;
    leds[1] = CRGB::Orange;
  }
  
  FastLED.show();
}


void LEDS_DisplayRPM(int percent)
{
  int nbLedsDisplayed = NUM_LEDS * percent/100.0;
  FastLED.setBrightness(settings.ledBrightness);
  
  if(nbLedsDisplayed >= NUM_LEDS)
  {
    nbLedsDisplayed = NUM_LEDS;

    for(int i = 0; i < nbLedsDisplayed; i++)
    {
      if(blinkFast)
      {
        leds[NUM_LEDS-1-i] = CRGB::Red;
      }
      else
      {
        leds[NUM_LEDS-1-i] = CRGB::Black;
      }
    }
  }
  else
  {
    for(int i = 0; i < nbLedsDisplayed; i++)
    {
      if(nbLedsDisplayed == NUM_LEDS)
      {
        leds[NUM_LEDS-1-i] = CRGB::Red;
      }
      else
      {
        if(i >= NUM_LEDS*80/100.0)
        {
          leds[NUM_LEDS-1-i] = CRGB::Red;
        }
        else if(i >= NUM_LEDS*60/100.0)
        {
          leds[NUM_LEDS-1-i] = CRGB::Orange;
        }
        else
        {
          leds[NUM_LEDS-1-i] = CRGB::Green;
        }
      }
    }
  }

  for(int i = nbLedsDisplayed; i < NUM_LEDS; i++)
  {
    leds[NUM_LEDS-1-i] = CRGB::Black;
  }
  FastLED.show();
}


void LEDS_AllOff()
{
  for(int i = 0; i < NUM_LEDS; i++)
  {
    leds[NUM_LEDS-1-i] = CRGB::Black;
  }
  FastLED.show();
}


void LEDS_AllRed()
{
  for(int i = 0; i < NUM_LEDS; i++)
  {
    leds[NUM_LEDS-1-i] = CRGB::Red;
  }
  FastLED.show();
}
