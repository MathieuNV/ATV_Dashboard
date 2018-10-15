#include <Arduino.h>

#include "GPS.h"
#include "Display.h"




  
void setup(void)
{
  GPS_Init();
  OLED_Init();
 
  delay(3000);
}


void loop(void) 
{
  GPS_Process();
  OLED_Draw();

  GPS_Delay(20);
}


