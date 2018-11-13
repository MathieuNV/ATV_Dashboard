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

#define DEBUG_FILE

//---------------------------------------------
// Variables
//---------------------------------------------
bool   SD_present = false;

int File_Init();
String File_FormatSize(int bytes);

fs::FS fileSystem = SPIFFS;                 ///< File system to use to read/write file (SPIFFS = internal memory, SD = external SD Card)


//---------------------------------------------
// Public Functions
//---------------------------------------------
int File_Init();
String File_FormatSize(int bytes);

int File_Write(fs::FS &fs, const char * path, const char * message);
int File_Append(fs::FS &fs, const char * path, const char * message);
int File_Rename(fs::FS &fs, const char * path1, const char * path2);
int File_Delete(fs::FS &fs, const char * path);

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


int File_Write(fs::FS &fs, const char * path, const char * message)
{
  int success = 1;
  
#ifdef DEBUG_FILE
  Serial.printf("Writing file: %s\r\n", path);
#endif

  File file = fs.open(path, FILE_WRITE);
  
  if(!file)
  {
#ifdef DEBUG_FILE
    Serial.println("- failed to open file for writing");
#endif
    success = 0;
  }
  
  if(file.print(message))
  {
#ifdef DEBUG_FILE
    Serial.println("- file written");
    success = 1;
#endif
  }
  else 
  {
#ifdef DEBUG_FILE
    Serial.println("- frite failed");
#endif
    success = 0;
  }

  file.close();
  
  return success;
  
}


int File_Append(fs::FS &fs, const char * path, const char * message)
{
   int success = 1;
   
#ifdef DEBUG_FILE
  Serial.printf("Appending to file: %s\r\n", path);
#endif

  File file = fs.open(path, FILE_APPEND);
  
  if(!file)
  {
#ifdef DEBUG_FILE
    Serial.println("- failed to open file for appending");
#endif
    success = 0;
  }
  
  if(file.print(message))
  {
#ifdef DEBUG_FILE
    Serial.println("- message appended");
#endif
    success = 1;
  } 
  else 
  {
#ifdef DEBUG_FILE
      Serial.println("- append failed");
#endif
    success = 0;
  }

  file.close();
}


int File_Rename(fs::FS &fs, const char * path1, const char * path2)
{
  int success = 1;
  
#ifdef DEBUG_FILE
  Serial.printf("Renaming file %s to %s\r\n", path1, path2);
#endif

  if (fs.rename(path1, path2)) 
  {
#ifdef DEBUG_FILE
    Serial.println("- file renamed");
#endif
    success = 1;
  }
  else 
  {
#ifdef DEBUG_FILE
    Serial.println("- rename failed");
#endif
    success = 0;
  }

   return success;
}


int File_Delete(fs::FS &fs, const char * path)
{
  int success = 1;
  
#ifdef DEBUG_FILE
  Serial.printf("Deleting file: %s\r\n", path);
#endif

  if(fs.remove(path))
  {
#ifdef DEBUG_FILE
    Serial.println("- file deleted");
#endif
    success = 1;
  }
  else
  {
#ifdef DEBUG_FILE
    Serial.println("- delete failed");
#endif
    success = 0;
  }

  return success;
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
