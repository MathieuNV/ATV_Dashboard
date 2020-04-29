//----------------------------------------------------------------------------- 
/** 
 *  
 * \file GPIO.cpp
 * \brief Management of inputs / outputs of the board
 * \author M.Navarro
 * \date 11/2019
 *
 */
//-----------------------------------------------------------------------------
// (c) Copyright MN 2019 - All rights reserved
//-----------------------------------------------------------------------------


//---------------------------------------------
// Include 
//---------------------------------------------
#include "GPIO.h"


//---------------------------------------------
// Defines
//---------------------------------------------
#define   BP_RIGHT_UP_PIN     35
#define   BP_RIGHT_DOWN_PIN   39  // SENSOR_VN
#define   BP_LEFT_UP_PIN      34
#define   BP_LEFT_DOWN_PIN    36  // SENSOR_VP

#define   DEBOUNCE_DELAY_MS   20

#define   BP_PRESSED_LEVEL    0
#define   BP_RELEASED_LEVEL   1

//---------------------------------------------
// Enum, struct, union
//---------------------------------------------
struct S_Button 
{
  const int    pin;
  long         lastDebounceTime;
  bool         lastState;
  bool         state;
  bool         clicked;
};


//---------------------------------------------
// Variables
//---------------------------------------------
S_Button Buttons[NB_OF_BUTTONS]  = {{BP_RIGHT_UP_PIN,   0, false, true, false },
                                    {BP_RIGHT_DOWN_PIN, 0, false, true, false},
                                    {BP_LEFT_UP_PIN,    0, false, true, false},
                                    {BP_LEFT_DOWN_PIN,  0, false, true, false}};

//---------------------------------------------
// Public Functions
//---------------------------------------------
void GPIO_Init();
void GPIO_Handle();

bool GPIO_IsButtonPressed(int buttonIndex);
bool GPIO_IsButtonClicked(int buttonIndex);

/*
void GPIO_LedToggle();
void GPIO_LedSet(bool state);*/


//---------------------------------------------
// Private Functions
//---------------------------------------------


//---------------------------------------------
// Functions declarations
//---------------------------------------------
void GPIO_Init()
{

//  pinMode(LED_PIN, OUTPUT);

  for(int i = 0; i < NB_OF_BUTTONS; i++)
  {
    pinMode(Buttons[i].pin, INPUT);
  }
}


void GPIO_Handle()
{
  for( int i = 0; i < NB_OF_BUTTONS; i++)
  {
    bool readValue = digitalRead(Buttons[i].pin);
    
    if (readValue != Buttons[i].lastState) 
    {
      Buttons[i].lastDebounceTime = millis();
    }
    
    if ((millis() - Buttons[i].lastDebounceTime) > DEBOUNCE_DELAY_MS) 
    {
      if (readValue != Buttons[i].state) 
      {
        Buttons[i].state = readValue;

        if(readValue == BP_RELEASED_LEVEL)
        {
          Buttons[i].clicked = true;  
        }
      }
    }

    Buttons[i].lastState = readValue;
  }
}


bool GPIO_IsButtonPressed(int buttonIndex)
{
  if(buttonIndex < NB_OF_BUTTONS)
  {    
    Buttons[buttonIndex].clicked = false;
    return !Buttons[buttonIndex].state;
  }
  else
  {
    return false;
  }
}


bool GPIO_IsButtonClicked(int buttonIndex)
{
  if(buttonIndex < NB_OF_BUTTONS)
  {
    if(Buttons[buttonIndex].clicked)
    {
      Buttons[buttonIndex].clicked = false;
      return true;
    }
    else
    {
      return false;
    }
  }
  else
  {
    return false;
  }
}


/*
void GPIO_LedToggle()
{
  if(digitalRead(LED_PIN))
  {
    digitalWrite(LED_PIN, LOW);
  }
  else
  {
    digitalWrite(LED_PIN, HIGH);    
  }
}


void GPIO_LedSet(bool state)
{
  if(state)
  {
    digitalWrite(LED_PIN, HIGH);
  }
  else
  {
    digitalWrite(LED_PIN, LOW);    
  }
}*/
