//----------------------------------------------------------------------------- 
/** 
 *  
 * \file Display.cpp
 * \brief Human/User Interface source file
 * \author M.Navarro
 * \date 10/2018
 *
 */
//-----------------------------------------------------------------------------
// (c) Copyright MN 2018 - All rights reserved
//-----------------------------------------------------------------------------


//---------------------------------------------
// Include 
//---------------------------------------------
#include <U8g2lib.h>
#include "Display.h"
#include "logos.h"
#include "GPIO.h"
#include "File.h"
#include "Settings.h"

//---------------------------------------------
// Defines
//---------------------------------------------


//---------------------------------------------
// Enum, struct, union
//---------------------------------------------
#define OLED_TIMER_PERIOD_US    10000000
//#define DEBUG

enum E_ScrollState
{
  E_ScrollState_Idle,
  E_ScrollState_ScrollDown,
  E_ScrollState_ScrollUp
};

s_logo logos[NB_OF_LOGOS] = {{"None", logo_none_bits},
                             {"KTM", logo_ktm_bits},
                             {"Kawasaki", logo_kawasaki_bits},
                             {"Polaris", logo_polaris_bits},
                             {"Yamaha", logo_yamaha_bits},
                             {"Suzuki", logo_suzuki_bits}};

//---------------------------------------------
// Variables
//---------------------------------------------
U8G2_SSD1309_128X64_NONAME0_F_4W_SW_SPI u8g2(U8G2_R0, PIN_OLED_CLOCK, PIN_OLED_DATA, PIN_OLED_CS, PIN_OLED_DC, PIN_OLED_RESET);  

int maxRpm;

// Timer interrupt, te refresh rpm value
hw_timer_t * oledTimer = NULL;
portMUX_TYPE oledTimerMux = portMUX_INITIALIZER_UNLOCKED;
volatile int oledTimerTick = 0;

E_ScrollState scrollState = E_ScrollState_Idle;
int currentScreenNb = 0;
int nextScreenNb = 0;
void (*currentScreen)(int);
void (*nextScreen)(int);

e_DispState dispState = MAIN_SCREEN;

// Menus
s_Menu menu_Main;
s_Menu menu_LedsSettings;
s_Menu menu_Settings;
s_Menu menu_Memory;

  
//---------------------------------------------
// Public Functions
//---------------------------------------------
void OLED_Init();
void OLED_Handle();


//---------------------------------------------
// Private Functions
//---------------------------------------------

static void OLED_DisplayMain();
static void OLED_DisplayMenu(s_Menu *menu);
static void OLED_ExitMenu(void);
static void OLED_SetMaxRPM();
static void OLED_SetLedBrightness();
static void OLED_SetBrandLogo();
static void OLED_SetMainDisplayStyle();
static void OLED_MemoryShowSize();

static void MainDisplay(void);
static void MenuMain(void);
static void MenuLedsSettings(void);
static void MenuSettings(void);
static void MenuSetMaxRPM();
static void MenuSetLedBrightness();
static void MenuBrandLogo();
static void MenuSetMainDisplayStyle();
static void MenuMemory();
static void MenuMemory_ShowSize();

// Differents screen that can be displayed
static void OLED_Screen_Main(int vOffset);
static void OLED_Screen_Track(int vOffset);
static void OLED_Screen_Stats(int vOffset);

// Basic Elements composing main screens 
static void OLED_Display_RPM(int xPos, int yPos);
static void OLED_Display_RPM2(int xPos, int yPos);
static void OLED_Display_Speed(int xPos, int yPos);
static void OLED_Display_Speed2(int xPos, int yPos);
static void OLED_Display_TripDistance(int xPos, int yPos);
static void OLED_Display_TotalDistance(int xPos, int yPos);
static void OLED_Display_Time(int xPos, int yPos);
static void OLED_Display_Satellites(int xPos, int yPos);
static void OLED_Display_Altitude(int xPos, int yPos);
static void OLED_Display_Gear(int xPos, int yPos);
static void OLED_Display_Track(int xPos, int yPos, int width, int heigth, double *xDataArray, double *yDataArray, int dataSize);
static void OLED_Display_History(int xPos, int yPos, int width, int heigth, int * dataArray, int dataSize, char * chartName);

static int  OLED_Scroll_Screens();
static void OLED_ScrollDown();
static void OLED_ScrollUp();

void IRAM_ATTR displayTimerISR();


//---------------------------------------------
// Functions
//---------------------------------------------

void IRAM_ATTR oledTimerISR() 
{
  portENTER_CRITICAL_ISR(&oledTimerMux);
  oledTimerTick = 1;
  portEXIT_CRITICAL_ISR(&oledTimerMux);
}

//---------------------------------------------
/// \fn void OLED_Init(void)
///
/// \brief Init OLED Screen.
/// \param None.
/// \return None.
void OLED_Init()
{
  char buff[32];
 
  u8g2.begin();

  u8g2.clearBuffer();
  u8g2.setContrast(10);
  u8g2.drawXBM(0, 0, 128, 64, logos[Settings_BrandLogo()].image);//logo_suzuki_bits);// logo_suzuki_bits);
  u8g2.setFont( u8g2_font_5x7_tf);
  sprintf(buff, "V%s", Settings_SoftVersion());
  u8g2.drawStr( 90, 64 , buff);
  u8g2.sendBuffer();

  currentScreen = OLED_Screen_Main;
  nextScreen = OLED_Screen_Track;

#ifdef SIMU_TEST_GPS
  for(int i = 0; i < 2038; i++)
  {
    gpsHistory.lng[i] = titi[i*2];
    gpsHistory.lat[i] = titi[i*2+1];
    gpsHistory.alt[i] = tete[i];
    gpsHistory.spd[i] = tete[i];
    gpsHistory.pointsIndex++;
  }
#endif


  // Main Menu elements
  
  menu_Main.items[0].name = "Leds settings";
  menu_Main.items[0].type = FUNCTION;
  menu_Main.items[0].func = MenuLedsSettings;
  
  menu_Main.items[1].name = "Settings";
  menu_Main.items[1].type = SUBMENU;
  menu_Main.items[1].func = MenuSettings;

  menu_Main.items[2].name = "Memory";
  menu_Main.items[2].type = SUBMENU;
  menu_Main.items[2].func = MenuMemory;

  menu_Main.previousMenu = MainDisplay;
  menu_Main.nbElements = 3;
  menu_Main.header = "Principal";

  // Leds Menu Elements
  menu_LedsSettings.items[0].name = "Leds Enable";
  menu_LedsSettings.items[0].type = VALUE_BOOL;
  menu_LedsSettings.items[0].func = Settings_LEDS_EnableToggle;
  menu_LedsSettings.items[0].ptrValue = Settings_LEDS_IsEnabled;
  
  menu_LedsSettings.items[1].name = "Leds bright.";
  menu_LedsSettings.items[1].type = VALUE_INT;
  menu_LedsSettings.items[1].func = MenuSetLedBrightness;
  menu_LedsSettings.items[1].ptrValue = Settings_LedBrightness;

  menu_LedsSettings.previousMenu = MenuMain;
  menu_LedsSettings.nbElements = 2;
  menu_LedsSettings.header = "Leds Settings";
  

  // Settings menu elements
  menu_Settings.items[0].name = "Max RPM";
  menu_Settings.items[0].type = VALUE_INT;
  menu_Settings.items[0].func = MenuSetMaxRPM;
  menu_Settings.items[0].ptrValue = Settings_MaxRpm;

  menu_Settings.items[1].name = "Display Style";
  menu_Settings.items[1].type = FUNCTION;
  menu_Settings.items[1].func = MenuSetMainDisplayStyle;

  menu_Settings.items[2].name = "Wifi enable";
  menu_Settings.items[2].type = VALUE_BOOL;
  menu_Settings.items[2].func = Settings_WebServer_EnableToggle;
  menu_Settings.items[2].ptrValue = Settings_WebServer_IsEnabled;

  menu_Settings.items[3].name = "Brand logo";
  menu_Settings.items[3].type = FUNCTION;
  menu_Settings.items[3].func = MenuBrandLogo;

  menu_Settings.previousMenu = MenuMain;
  menu_Settings.header = "Settings";
  menu_Settings.nbElements = 4;

  menu_Memory.items[0].name = "Size";
  menu_Memory.items[0].type = FUNCTION;
  menu_Memory.items[0].func = MenuMemory_ShowSize;

  menu_Memory.previousMenu = MenuMain;
  menu_Memory.header = "Memory";
  menu_Memory.nbElements = 1;
  dispState = MAIN_SCREEN; 
}


void OLED_Handle()
{  
   // Display main Screen only if splash logo finished 
  if(millis() < SPLASH_LOGO_DURATION_MS)
  {
    return;
  }

  u8g2.clearBuffer();
  u8g2.setDrawColor(1);

  switch(dispState)
  {
    case MAIN_SCREEN:  
      OLED_DisplayMain();
      break;
      
    case MENU_MAIN:  
      OLED_DisplayMenu(&menu_Main);
      break;

    case MENU_LEDS_SETTINGS:  
      OLED_DisplayMenu(&menu_LedsSettings);
      break;

    case MENU_SETTINGS:
      OLED_DisplayMenu(&menu_Settings);
      break;

    case MENU_MEMORY:
      OLED_DisplayMenu(&menu_Memory);
      break;      

    case MENU_SETTINGS_SETMAXRPM:
      OLED_SetMaxRPM();
      break;

    case MENU_LED_BRIGHTNESS:
      OLED_SetLedBrightness();
      break;

    case MENU_SETTINGS_BRANDLOGO:
      OLED_SetBrandLogo();
      break;

    case MENU_SETTINGS_MAINDISPLAYSTYLE:
      OLED_SetMainDisplayStyle();
      break;

    case MENU_MEMORY_SHOWSIZE:
      OLED_MemoryShowSize();
      break;
  }

  u8g2.sendBuffer();
}


void MainDisplay(void)
{
   dispState = MAIN_SCREEN; 
}


void MenuLedsSettings(void)
{
   dispState = MENU_LEDS_SETTINGS; 
}


void MenuMain(void)
{
   dispState = MENU_MAIN; 
}


void MenuSettings(void)
{
   dispState = MENU_SETTINGS; 
}


void MenuSetMaxRPM()
{
  dispState = MENU_SETTINGS_SETMAXRPM;
}

void MenuSetLedBrightness()
{
  dispState = MENU_LED_BRIGHTNESS;
}


void MenuBrandLogo()
{
  dispState = MENU_SETTINGS_BRANDLOGO;
}


void MenuSetMainDisplayStyle()
{
  dispState = MENU_SETTINGS_MAINDISPLAYSTYLE;
}

void MenuMemory()
{
  dispState = MENU_MEMORY;
}

void MenuMemory_ShowSize()
{
  dispState = MENU_MEMORY_SHOWSIZE;
}


void OLED_DisplayMenu(s_Menu *menu) 
{
  int i = 0;
  char buff[32];

 // Display header
 
  u8g2.setDrawColor(1);
  u8g2.setFont( u8g2_font_6x10_tf);//u8g2_font_5x7_tf);
  u8g2.drawStr( 10, 11 , menu->header);
  
  u8g2.drawLine(2,2,125,2);
  u8g2.drawLine(2,12,125,12);

  if(menu->currentSelection >= (menu->firstDisplayElement + MAX_DISPLAY_ITEMS) )
  {
    menu->firstDisplayElement = menu->currentSelection - MAX_DISPLAY_ITEMS + 1;
  }
  else if(menu->currentSelection < menu->firstDisplayElement)
  {
    menu->firstDisplayElement = menu->currentSelection;
  }
    
  // Display menu items
  for(i = 0; i < min(menu->nbElements,MAX_DISPLAY_ITEMS); i++)
  {
    if(menu->items[menu->currentSelection].type != VOID)
    {
      if((i + menu->firstDisplayElement) == menu->currentSelection)
      {
        u8g2.drawBox(4,14+10*i,124,10);
        u8g2.setDrawColor(0);
      }
      else
      {
        u8g2.setDrawColor(1);
      }
      
      u8g2.setFont(u8g2_font_6x10_tf);
      u8g2.drawStr(5,22+10*i, menu->items[i+menu->firstDisplayElement].name);
     
      if(menu->items[i+menu->firstDisplayElement].type == SUBMENU)
      {
          u8g2.setFont(u8g2_font_6x12_t_symbols);
          u8g2.drawGlyph(115, 22+10*i, 0x2192);
      }
      else if(menu->items[i+menu->firstDisplayElement].type == VALUE_BOOL)
      {
        u8g2.setFont(u8g2_font_6x12_t_symbols);
        
        if((*menu->items[i+menu->firstDisplayElement].ptrValue)())
        {
          u8g2.drawGlyph(115, 22+10*i, 0x25CF);
        }
        else
        {            
          u8g2.drawGlyph(115, 22+10*i, 0x25CB);
        }
      }
      else if(menu->items[i+menu->firstDisplayElement].type == VALUE_INT)
      {
        sprintf(buff, "%d", (*menu->items[i+menu->firstDisplayElement].ptrValue)());
        
        u8g2.drawStr(120-u8g2.getStrWidth(buff),22+10*i, buff);
      }
    }
  }
  
  if(GPIO_IsButtonClicked(BP_LEFT_UP))
  {
    menu->currentSelection -=1;
    menu->currentSelection = constrain(menu->currentSelection, 0, menu->nbElements-1);
  }
  else if(GPIO_IsButtonClicked(BP_LEFT_DOWN))
  {
    menu->currentSelection += 1;
    menu->currentSelection = constrain(menu->currentSelection, 0, menu->nbElements-1);
  }
  else if(GPIO_IsButtonClicked(BP_RIGHT_UP))
  {
    if(menu->items[menu->currentSelection].func != NULL)
    {
      (*menu->items[menu->currentSelection].func)();
    }    
  }
  else if(GPIO_IsButtonClicked(BP_RIGHT_DOWN))
  {
    (menu->previousMenu)();
  }
}


void OLED_MemoryShowSize()
{
  char buff[32];
  static int dataRead = 0;
  static int totalBytes = 0;
  static int usedBytes = 0;
  
  u8g2.setDrawColor(1);
  u8g2.setFont( u8g2_font_6x10_tf);
  u8g2.drawStr( 10, 11 , "Memory used");
  
  u8g2.drawLine(2,2,125,2);
  u8g2.drawLine(2,12,125,12);
  
  if(!dataRead)
  {
    totalBytes = File_TotalBytes(fileSystem);
    usedBytes = File_UsedBytes(fileSystem);
    Serial.println(totalBytes);
    Serial.println(usedBytes);
    dataRead = 1;
  }
  
  sprintf(buff, "%s / %s",File_FormatSize(usedBytes), File_FormatSize(totalBytes));
  u8g2.drawStr( 10, 30 , buff);

  u8g2.drawFrame(12, 40, 100, 15);
  u8g2.drawBox(14, 42, 100.0*usedBytes/totalBytes, 11);
    
  if(GPIO_IsButtonClicked(BP_RIGHT_UP))
  {
    MenuMemory();
    dataRead = 0;
  }
  else if(GPIO_IsButtonClicked(BP_RIGHT_DOWN))
  {
    MenuMemory();
    dataRead = 0;
  }
}


void OLED_SetMaxRPM()
{
  char buff[32];
  
  u8g2.setDrawColor(1);
  u8g2.setFont( u8g2_font_6x10_tf);
  u8g2.drawStr( 10, 11 , "Set Value");
  
  u8g2.drawLine(2,2,125,2);
  u8g2.drawLine(2,12,125,12);
  
  sprintf(buff, "%d", Settings_MaxRpm());
  u8g2.drawStr(5, 40, "Max RPM:");
  u8g2.drawStr(100-u8g2.getStrWidth(buff)/2+2,40, buff );

  u8g2.setFont(u8g2_font_6x12_t_symbols);
  u8g2.drawGlyph(100, 30, 0x25b2);
  u8g2.drawGlyph(100, 50, 0x25bc);

  if(GPIO_IsButtonClicked(BP_LEFT_UP))
  {
    Settings_MaxRpm_Inc();
  }
  else if(GPIO_IsButtonClicked(BP_LEFT_DOWN))
  {
    Settings_MaxRpm_Dec();
  }
  else if(GPIO_IsButtonClicked(BP_RIGHT_UP))
  {
    Settings_Save();
    MenuSettings();
  }
  else if(GPIO_IsButtonClicked(BP_RIGHT_DOWN))
  {
    Settings_Save();
    MenuSettings();
  }
}


void OLED_SetLedBrightness()
{
  char buff[32];
  
  u8g2.setDrawColor(1);
  u8g2.setFont( u8g2_font_6x10_tf);
  u8g2.drawStr( 10, 11 , "Set Value");
  
  u8g2.drawLine(2,2,125,2);
  u8g2.drawLine(2,12,125,12);
  
  sprintf(buff, "%d", Settings_LedBrightness());
  u8g2.drawStr(5, 40, "Led bright.:");
  u8g2.drawStr(100-u8g2.getStrWidth(buff)/2+2,40, buff );

  u8g2.setFont(u8g2_font_6x12_t_symbols);
  u8g2.drawGlyph(100, 30, 0x25b2);
  u8g2.drawGlyph(100, 50, 0x25bc);

  if(GPIO_IsButtonClicked(BP_LEFT_UP))
  {
    Settings_LedBrightness_Inc();
  }
  else if(GPIO_IsButtonClicked(BP_LEFT_DOWN))
  {
    Settings_LedBrightness_Dec();
  }
  else if(GPIO_IsButtonClicked(BP_RIGHT_UP))
  {
    Settings_Save();
    MenuLedsSettings();
  }
  else if(GPIO_IsButtonClicked(BP_RIGHT_DOWN))
  {
    Settings_Save();
    MenuLedsSettings();
  }
}


void OLED_SetBrandLogo()
{
  char buff[32];
  
  u8g2.setDrawColor(1);
  u8g2.setFont( u8g2_font_6x10_tf);
  u8g2.drawStr( 10, 11 , "Set Logo");
  
  u8g2.drawLine(2,2,125,2);
  u8g2.drawLine(2,12,125,12);
  
  sprintf(buff, "%s", logos[Settings_BrandLogo()].name);
  u8g2.drawStr(5, 40, "Brand:");
  u8g2.drawStr(100-u8g2.getStrWidth(buff)/2+2,40, buff );

  u8g2.setFont(u8g2_font_6x12_t_symbols);
  u8g2.drawGlyph(100, 30, 0x25b2);
  u8g2.drawGlyph(100, 50, 0x25bc);

  if(GPIO_IsButtonClicked(BP_LEFT_UP))
  {
    Settings_BrandLogo_Inc();
  }
  else if(GPIO_IsButtonClicked(BP_LEFT_DOWN))
  {
    Settings_BrandLogo_Dec();
  }
  else if(GPIO_IsButtonClicked(BP_RIGHT_UP))
  { 
    Settings_Save();
    MenuSettings();
  }
  else if(GPIO_IsButtonClicked(BP_RIGHT_DOWN))
  {
    Settings_Save();
    MenuSettings();
  }
}


void OLED_SetMainDisplayStyle()
{
  char buff[32];
  
  u8g2.setDrawColor(1);
  u8g2.setFont( u8g2_font_6x10_tf);
  u8g2.drawStr( 10, 11 , "Set Display style");
  
  u8g2.drawLine(2,2,125,2);
  u8g2.drawLine(2,12,125,12);
  
  sprintf(buff, "%d", Settings_MainDisplayStyle());
  u8g2.drawStr(5, 40, "Style:");
  u8g2.drawStr(100-u8g2.getStrWidth(buff)/2+2,40, buff );

  u8g2.setFont(u8g2_font_6x12_t_symbols);
  u8g2.drawGlyph(100, 30, 0x25b2);
  u8g2.drawGlyph(100, 50, 0x25bc);

  if(GPIO_IsButtonClicked(BP_LEFT_UP))
  {
    Settings_MainDisplayStyle_Inc();
  }
  else if(GPIO_IsButtonClicked(BP_LEFT_DOWN))
  {
    Settings_MainDisplayStyle_Dec();
  }
  else if(GPIO_IsButtonClicked(BP_RIGHT_UP))
  { 
    Settings_Save();
    MenuSettings();
  }
  else if(GPIO_IsButtonClicked(BP_RIGHT_DOWN))
  {
    Settings_Save();
    MenuSettings();
  }
}


void OLED_DisplayMain()
{
  if(scrollState != E_ScrollState_Idle)
  {
    if(OLED_Scroll_Screens())
    {
      scrollState = E_ScrollState_Idle;
      currentScreenNb = nextScreenNb;
      currentScreen = nextScreen;
    }
  }
  else
  {
    if(GPIO_IsButtonClicked(BP_LEFT_DOWN))
    {
      OLED_ScrollDown();
    }
    else if(GPIO_IsButtonClicked(BP_LEFT_UP))
    {
      OLED_ScrollUp();
    }
    else if(GPIO_IsButtonClicked(BP_RIGHT_UP))
    {
      MenuMain();
    }

    // Draw currentScreen
    currentScreen(0);
  }
}


void OLED_ScrollDown()
{
  if(scrollState == E_ScrollState_Idle)
  {
    scrollState = E_ScrollState_ScrollDown;
    
    if(currentScreenNb == 0 )
    {
      nextScreenNb = 1;
      currentScreen = OLED_Screen_Main;
      nextScreen = OLED_Screen_Track;
    }
    else if(currentScreenNb == 1)
    {
      nextScreenNb = 2;
      currentScreen = OLED_Screen_Track;
      nextScreen = OLED_Screen_Stats;
    }
    else
    {
      nextScreenNb = 0;
      currentScreen = OLED_Screen_Stats;
      nextScreen = OLED_Screen_Main;
    }
  }
}

void OLED_ScrollUp()
{
  if(scrollState == E_ScrollState_Idle)
  {
    scrollState = E_ScrollState_ScrollUp;
    
    if(currentScreenNb == 0 )
    {
       nextScreenNb = 2;
       currentScreen = OLED_Screen_Main;
       nextScreen = OLED_Screen_Stats;
    }
    else if(currentScreenNb == 1)
    {
      nextScreenNb = 0;
      currentScreen = OLED_Screen_Track;
      nextScreen = OLED_Screen_Main;
    }
    else
    {
      nextScreenNb = 1;
      currentScreen = OLED_Screen_Stats;
      nextScreen = OLED_Screen_Track;
    }
  }
}


static int OLED_Scroll_Screens()
{
  static int offset = 0;

  if(scrollState == E_ScrollState_ScrollDown)
  {
    currentScreen(offset);
    nextScreen(offset-SCREEN_HEIGHT);
  }
  else if(scrollState == E_ScrollState_ScrollUp)
  {
    currentScreen(-offset);
    nextScreen(SCREEN_HEIGHT-offset);
  }
  else
  {
    return 1;
  }

  offset += 8;
  
  if (offset >= SCREEN_HEIGHT)
  {
    offset = 0;
    return 1;
  }
  else
  {
    return 0;
  }
}


static void OLED_Screen_Main(int vOffset)
{  
  if(Settings_MainDisplayStyle() == 0)
  {
    OLED_Display_RPM(0, vOffset+0);
    OLED_Display_Speed(0, vOffset+48);

      // Displays total distance for some seconds at startup, then actual trip distance
    if((millis() - SPLASH_LOGO_DURATION_MS) < TOTAL_DISTANCE_DISPLAY_DURATION_MS)
    { 
      OLED_Display_TotalDistance(0, vOffset+64);
    }
    else
    {
      OLED_Display_TripDistance(0, vOffset+64);
    }
  
    OLED_Display_Time(100, vOffset+64);
    
    OLED_Display_Altitude(100, vOffset+54); 
  
    OLED_Display_Satellites(88, vOffset+64);
  
    OLED_Display_Gear(107, vOffset+42);
  }
  else
  {
    OLED_Display_RPM2(42, vOffset+12);
    OLED_Display_Speed2(0, vOffset+52);   
    
    // Displays total distance for some seconds at startup, then actual trip distance
    if((millis() - SPLASH_LOGO_DURATION_MS) < TOTAL_DISTANCE_DISPLAY_DURATION_MS)
    { 
      OLED_Display_TotalDistance(0, vOffset+64);
    }
    else
    {
      OLED_Display_TripDistance(0, vOffset+64);
    }
  
    OLED_Display_Time(100, vOffset+64);
    
    OLED_Display_Altitude(100, vOffset+54); 
  
    OLED_Display_Satellites(88, vOffset+64);
  
    OLED_Display_Gear(0, vOffset+20); 
  }
}


static void OLED_Screen_Track(int vOffset)
{
  u8g2.drawFrame(0,vOffset,128,64);
  OLED_Display_Track(0, vOffset, 128, 64, gpsHistory.lng, gpsHistory.lat, gpsHistory.pointsIndex);
}


static void OLED_Screen_Stats(int vOffset)
{
  OLED_Display_History(20, 5+vOffset, 100, 25, gpsHistory.alt, gpsHistory.pointsIndex, "Alt");
  OLED_Display_History(20, 35+vOffset, 100, 25, gpsHistory.spd, gpsHistory.pointsIndex, "Spd");
}


// Displays actual RPM, at the top of the screen.
static void OLED_Display_RPM(int xPos, int yPos)
{
  char buff[32];
  float coef = (float)SCREEN_WIDTH / (settings.maxRPM/1000.0);
  int bargraphWidth;

  u8g2.drawLine(xPos, yPos+18, SCREEN_WIDTH, yPos+18);

  // Creates bargraph legend
  for(int i = 0; i < ((settings.maxRPM/1000)-1); i++)
  {
    u8g2.drawFrame(xPos+((coef*(i+1))+0.5), yPos+8, 1, 2);  // USE FRAME WITH width of 1 to avoid line issues...
    sprintf(buff, "%d", i+1);
    u8g2.setFont( u8g2_font_micro_tr);
    u8g2.drawStr( xPos + (coef*(i+1)-1),yPos+17 , buff);          //*1000 rpm numbers
  }

  for(int i = 0; i < ((settings.maxRPM/1000)+1); i++)
  {
    u8g2.drawPixel(xPos + ((coef*i)+0.5+(coef/2)+0.5), yPos+8);    // 500 rpms markers   
  }

  bargraphWidth = (rpm / (settings.maxRPM) * SCREEN_WIDTH)+0.5;

  // Compute "Recent max rpm" bar
  if(bargraphWidth >= maxRpm)
  {
    maxRpm = bargraphWidth;
  }
  else
  {
    maxRpm -= 2;
    
    if(maxRpm < bargraphWidth)
    {
      maxRpm = bargraphWidth;
    }
  }

  // RPM Bar
  u8g2.drawBox(xPos+0, yPos+0, bargraphWidth, 7);

  // Recent Max RPM indication
  u8g2.drawLine(xPos + maxRpm, yPos+0, xPos + maxRpm, yPos+7);
}


static void OLED_Display_RPM2(int xPos, int yPos)
{
  char buff[32];
  static int filteredRpm = 0;

  filteredRpm = filteredRpm*((float)10 / ((float)1 + (float)10)) +  rpm * ((float)1 / ((float)1 + (float)10));

  sprintf(buff, "%5d rpm", (int)(filteredRpm));

  u8g2.setFont( u8g2_font_8x13B_tr  );
  u8g2.drawStr( xPos, yPos , buff);
  //u8g2.drawStr( xPos+90, yPos , "rpm");
}


static void OLED_Display_Speed(int xPos, int yPos)
{
  char buff[32];

  if(gps.speed.isValid())
  {
    sprintf(buff, "%d", (int)(gps.speed.kmph()+0.5));
  }
  else
  {
    sprintf(buff, "---");
  }

  u8g2.setFont( u8g2_font_fub25_tn  );//u8g2_font_helvB24_tn);//u8g2_font_freedoomr25_tn );
  u8g2.drawStr( xPos+56-u8g2.getStrWidth(buff), yPos , buff);
  u8g2.setFont( u8g2_font_5x7_tf);
  u8g2.drawStr( xPos+60, yPos-20 , "km/h");
}


static void OLED_Display_Speed2(int xPos, int yPos)
{
  char buff[32];

  u8g2.setFont( u8g2_font_fub35_tn  );// u8g2_font_7Segments_26x42_mn  );//u8g2_font_logisoso38_tn  );//u8g2_font_fur35_tn );
  
  if(gps.speed.isValid())
  {
    sprintf(buff, "%d", (int)(gps.speed.kmph()+0.5));
  }
  else
  {
    sprintf(buff, "---");
  }

  u8g2.drawStr( xPos+80-u8g2.getStrWidth(buff), yPos , buff);
  
  u8g2.setFont( u8g2_font_8x13B_tr  );
  u8g2.drawStr( xPos+90, yPos-26 , "km/h");
}


//Displays current trip distance
static void OLED_Display_TripDistance(int xPos, int yPos)
{
  char buff[32];
  
  u8g2.setFont( u8g2_font_5x7_tf);//u8g2_font_6x10_tf);
  sprintf(buff, "Trip %d.%01d", (int)(trip/1000.0), (int)((trip/1000.0)*10.0)%10);  //  dtostrf(speed, 4, 1, buff);
  u8g2.drawStr( xPos, yPos, buff);
}

//Displays toal distance parcoured
static void OLED_Display_TotalDistance(int xPos, int yPos)
{
  char buff[32];
  
  u8g2.setFont( u8g2_font_5x7_tf);//u8g2_font_6x10_tf);
  sprintf(buff, "Total %d.%01d", (int)total,(int)(total*100)%100);
  u8g2.drawStr( xPos, yPos , buff);
}


//Displays current time
static void OLED_Display_Time(int xPos, int yPos)
{
  char buff[32];
  
  u8g2.setFont( u8g2_font_5x7_tf);
  
  if(gps.time.isValid())
  {
    sprintf( buff, "%02d:%02d ", hour(), minute());
  }
  else
  {
    sprintf( buff, "--:-- ");
  }
  u8g2.drawStr( xPos, yPos, buff);
}


static void OLED_Display_Altitude(int xPos, int yPos)
{
  char buff[32];
  
  if(gps.altitude.isValid())
  {
    sprintf( buff, "%4dm", (int)gps.altitude.meters());
  }
  else
  {
    sprintf( buff, "----m");
  }
  u8g2.drawStr( xPos, yPos, buff);
}


static void OLED_Display_Satellites(int xPos, int yPos)
{
  char buff[32];
  
  if(gps.satellites.isValid())
  {
    sprintf( buff, "%d", gps.satellites.value());
  }
  else
  {
    sprintf( buff, "-");
  }
  u8g2.drawStr( xPos-22, yPos, buff);

  if(recordTrip)
  {
    u8g2.setFont(u8g2_font_open_iconic_play_1x_t);
    u8g2.drawGlyph(xPos-10, yPos+1, 0x46);
  }
  
  if(firstFixDone)
  {
    u8g2.setFont(u8g2_font_open_iconic_check_1x_t);
    u8g2.drawGlyph(xPos, yPos+1, 0x40);
    //u8g2.drawDisc();
  }

  
}


static void OLED_Display_Gear(int xPos, int yPos)
{
  char buff[32];
  
  //Engaged gear display
  sprintf(buff, "N" );
  u8g2.setFont(u8g2_font_helvB18_tf );//u8g2_font_helvB24_tn  );//u8g2_font_freedoomr25_tn );// u8g2_font_logisoso26_tn  );
  u8g2.drawStr( xPos, yPos , buff);
}


static void OLED_Display_Track(int xPos, int yPos, int width, int heigth, double *xDataArray, double *yDataArray, int dataSize)
{
  char buff[32];
  double xCartesian = 0;
  double yCartesian = yPos;
  int hMargin = 4, vMargin = 4;
  double minX= 180, maxX = -180, minY= 90, maxY = -90;
  double xCoef, yCoef;

  for(int i = 0; i < dataSize; i++)
  {
    if( minY > yDataArray[i])
    {
      minY = yDataArray[i];
    }

    if(maxY < yDataArray[i])
    {
      maxY = yDataArray[i];
    }

    if( minX > xDataArray[i])
    {
      minX = xDataArray[i];
    }

    if( maxX < xDataArray[i])
    {
      maxX = xDataArray[i];
    }
  }
  
  xCoef = maxX - minX;
  yCoef = maxY - minY;

  for(int i = 0; i < dataSize; i++)
  {
    yCartesian = yPos + heigth - vMargin -((yDataArray[i] - minY) / yCoef * (double)(heigth-(vMargin*2)));
    xCartesian = xPos + hMargin +(xDataArray[i] - minX) / xCoef * (double)(width-(hMargin*2));
    
    /*
    yCartesian =  SCREEN_HEIGHT - vMargin - ((yDataArray[i] - minY) / yCoef * (double)(SCREEN_HEIGHT-(vMargin*2)));
    xCartesian =  hMargin + (xDataArray[i] - minX) / xCoef * (double)(SCREEN_WIDTH-(hMargin*2));
*/
    u8g2.drawPixel((int)xCartesian, (int)yCartesian);
  }

   u8g2.drawDisc((int)xCartesian, (int)yCartesian, 2);
}


static void OLED_Display_History(int xPos, int yPos, int width, int heigth, int * dataArray, int dataSize, char * chartName)
{
  char buff[32];
  int indexCoef;
  int valCoef;
  int minVal = 10000, maxVal = 0;
  int hMargin = 2, vMargin = 3;

  int textWidth = 13;

  // Prepare necessary values for plotting
  for(int i = 0; i < dataSize; i++)
  {
    if(dataArray[i] >= maxVal)
    {
      maxVal = dataArray[i];
    }
    else if(dataArray[i] <= minVal)
    {
      minVal = dataArray[i];
    }
  }
  
  valCoef = maxVal - minVal;
  indexCoef = dataSize / (width-2*hMargin-textWidth);
  
  if(indexCoef < 1)
  {
    indexCoef = 1;
  }

  // Display chart axis
  u8g2.drawFrame(xPos+textWidth, yPos, 1, heigth);  // USE FRAME WITH width of 1 to avoid line issues...
  u8g2.drawLine(xPos+textWidth, yPos+heigth, xPos+width, yPos+heigth);
  //Serial.print(yPos+heigth);
   
  // Display Scale
  u8g2.setFont( u8g2_font_micro_tr);
  sprintf(buff, "%d", (int)maxVal);
  u8g2.drawStr( xPos, yPos+7 , buff);  
  sprintf(buff, "%d", (int)minVal);
  u8g2.drawStr( xPos, yPos+heigth-1 , buff); 

  // Chart Name
  sprintf(buff, "%s", chartName); 
  u8g2.drawStr( xPos-6, yPos+heigth/2+3 , buff); 

  // Plot values
  for(int i = 0; i < dataSize; i++)//width-1; i++)
  {
    u8g2.drawPixel(xPos+i/indexCoef+1+hMargin+textWidth, (yPos+heigth-hMargin-1) - ((float)(dataArray[i]-minVal)/(float)valCoef)*(float)(heigth-1-2*hMargin));
  } 
}
