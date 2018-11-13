//----------------------------------------------------------------------------- 
/** 
 *  
 * \file File.h
 * \brief Management of log files header
 * \author M.Navarro
 * \date 10/2018
 *
 */
//-----------------------------------------------------------------------------
// (c) Copyright MN 2018 - All rights reserved
//-----------------------------------------------------------------------------
#ifndef _FILE_H
#define _FILE_H

//---------------------------------------------
// Include 
//---------------------------------------------
#include "FS.h"
#include "SPIFFS.h"


//---------------------------------------------
// Defines
//---------------------------------------------


//---------------------------------------------
// Enum, struct, union
//---------------------------------------------


//---------------------------------------------
// Type
//---------------------------------------------


//---------------------------------------------
// Public variables
//---------------------------------------------
extern bool SD_present;

extern fs::FS fileSystem;

//---------------------------------------------
// Public Functions
//---------------------------------------------
extern int File_Init();
extern String File_FormatSize(int bytes);


extern int File_Write(fs::FS &fs, const char * path, const char * message);
extern int File_Append(fs::FS &fs, const char * path, const char * message);
extern int File_Rename(fs::FS &fs, const char * path1, const char * path2);
extern int File_Delete(fs::FS &fs, const char * path);
#endif
