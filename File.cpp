//----------------------------------------------------------------------------- 
/** 
 *  
 * \file File.cpp
 * \brief Management of the file system
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
#include "FS.h"
#include "SPIFFS.h"

#include "File.h"


//---------------------------------------------
// Defines
//---------------------------------------------
#define   FORMAT_SPIFFS_IF_FAILED false
#define   SD_CS_pin               5


//---------------------------------------------
// Variables
//---------------------------------------------
bool   SD_present = false;

int File_Init();
String File_FormatSize(int bytes);


//---------------------------------------------
// Public Functions
//---------------------------------------------
int File_Init();
String File_FormatSize(int bytes);
//---------------------------------------------
// Private Functions
//---------------------------------------------


//---------------------------------------------
// Functions declarations
//---------------------------------------------

int File_Init()
{
  if(!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED))
  {
#ifdef DEBUG_FILE
    Serial.println("SPIFFS Mount Failed");
#endif

    return 0;
  }
  else
  {
    SD_present = true;
    return 1;
  }
}


String File_FormatSize(int bytes)
{
  String fsize = "";
  
  if (bytes < 1024)                 fsize = String(bytes)+" B";
  else if(bytes < (1024*1024))      fsize = String(bytes/1024.0,1)+" KB";
  else if(bytes < (1024*1024*1024)) fsize = String(bytes/1024.0/1024.0,1)+" MB";
  else fsize = String(bytes/1024.0/1024.0/1024.0,1)+" GB";
  
  return fsize;
}
