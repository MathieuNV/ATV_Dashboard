#include <U8g2lib.h>
#include "Display.h"
#include "GPS.h"
#include "logos.h"



U8G2_SSD1309_128X64_NONAME0_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 18, /* data=*/ 23, /* cs=*/ 5, /* dc=*/ 4, /* reset=*/ 0);  


void OLED_Init();
void OLED_Draw();


void OLED_Init()
{
  u8g2.begin();
  
  u8g2.clearBuffer();
  u8g2.drawXBM(0, 0, 128, 64, logo_suzuki_bits);

  u8g2.sendBuffer();
}



void OLED_Draw()
{
  char buff[32]; 
  
  u8g2.clearBuffer();
  
  // RPM display
  u8g2.drawLine(0, 18, 127, 18);
  
  for(int i = 0; i< 7; i++)
  {
    u8g2.drawLine(16*(i+1), 8, 16*(i+1), 10);
    sprintf(buff, "%d", i+1);
    u8g2.setFont( u8g2_font_micro_tr);
    u8g2.drawStr( 16*(i+1)-1,17 , buff);
  }
  
  for(int i = 0; i< 9; i++)
  {
  u8g2.drawLine(16*i+8, 8, 16*i+8, 8);
  }
  
  u8g2.drawBox(0,0, (int)rpm, 7);
  u8g2.drawLine(maxRpm, 0, maxRpm, 7);
  

  // Speed Display
  if(gps.speed.isValid())
  {
    sprintf(buff, "%3d", (int)(spd+0.5));
  }
  else
  {
    sprintf(buff, "---");
  }

  u8g2.setFont( u8g2_font_helvB24_tn);//u8g2_font_freedoomr25_tn );
  u8g2.drawStr( 0, 48 , buff);
  u8g2.setFont( u8g2_font_5x7_tf);
  u8g2.drawStr( 56,28 , "km/h");


  // Distance display
  u8g2.setFont( u8g2_font_5x7_tf);//u8g2_font_6x10_tf);
  sprintf(buff, "Trip %d.%01d", (int)(trip/1000.0), (int)((trip/1000.0)*10.0)%10);  //  dtostrf(speed, 4, 1, buff);
  u8g2.drawStr( 0, 62 , buff);
  /*sprintf(buff, "Total %d.%01d", (int)total,(int)(total*100)%100);
  u8g2.drawStr( 0,64 , buff);*/


  // Misc display
  u8g2.setFont( u8g2_font_5x7_tf);
  if(gps.time.isValid())
  {
    sprintf( buff, "%02d:%02d ",hours ,minutes);
  }
  else
  {
    sprintf( buff, "--:-- ");
  }
  u8g2.drawStr( 100,64 , buff);
  
  if(gps.altitude.isValid())
  {
    sprintf( buff, "%4dm",alt);
  }
  else
  {
    sprintf( buff, "----m");
  }
  u8g2.drawStr( 100,54 , buff);


  //Engaged gear display
  sprintf(buff, "N" );
  u8g2.setFont(u8g2_font_helvB18_tf );//u8g2_font_helvB24_tn  );//u8g2_font_freedoomr25_tn );// u8g2_font_logisoso26_tn  );
  u8g2.drawStr( 110, 42 , buff);

  u8g2.sendBuffer();
}

