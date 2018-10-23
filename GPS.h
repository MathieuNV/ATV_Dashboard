//----------------------------------------------------------------------------- 
/** 
 *  
 * \file GPS.h
 * \brief Management of GPS data header file
 * \author M.Navarro
 * \date 10/2018
 *
 */
//-----------------------------------------------------------------------------
// (c) Copyright MN 2018 - All rights reserved
//-----------------------------------------------------------------------------
#ifndef _GPS_H
#define _GPS_H

//---------------------------------------------
// Include 
//---------------------------------------------
#include <TinyGPS++.h>


//---------------------------------------------
// Defines
//---------------------------------------------
#define   LOCATION_HISTORY_SIZE     2000

//---------------------------------------------
// Enum, struct, union
//---------------------------------------------
typedef struct
{
  double lng;     ///< Longitude
  double lat;     ///< Latitude
  int alt;        ///< Altitude, in meters
  int spd;        ///< Speed, since last point, in km/h
}gpsPoint_str;

typedef struct
{
  gpsPoint_str points[LOCATION_HISTORY_SIZE];
  int pointsIndex = 0;

  double minLat = 90.0;
  double maxLat = -90.0;

  double minLng = 180.0;
  double maxLng = -180.0;
}gpsHistory_str;

//---------------------------------------------
// Type
//---------------------------------------------


//---------------------------------------------
// Public variables
//---------------------------------------------
extern TinyGPSPlus gps;


extern float rpm;
extern float spd;

extern int alt;
  
extern int hours;
extern int minutes;
  
extern double trip;
extern double total;
 
extern int test;

extern gpsHistory_str gpsHistory;

extern double titi[4076];

//---------------------------------------------
// Public Functions
//---------------------------------------------
extern void GPS_Init();
extern void GPS_Process();
extern void GPS_Delay(unsigned long ms);

#endif
