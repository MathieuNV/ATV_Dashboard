//----------------------------------------------------------------------------- 
/** 
 *  
 * \file Web_Server.cpp
 * \brief Management of web server functions.
 * \author M.Navarro
 * \date 10/2018
 *
 * Allow system to generates WIFI Access point, to send/download files 
 * and control system. 
 */
//-----------------------------------------------------------------------------
// (c) Copyright MN 2018 - All rights reserved
//-----------------------------------------------------------------------------

//---------------------------------------------
// Include 
//---------------------------------------------
#include <WiFi.h>
#include <ESP32WebServer.h>    // https://github.com/Pedroalbuquerque/ESP32WebServer download and place in your Libraries folder
#include <ESPmDNS.h>
#include <Update.h>
#include "FS.h"
#include "SPIFFS.h"

#include "Web_Server.h"
#include "File.h"


//---------------------------------------------
// Defines
//---------------------------------------------
#define DEBUG_WEBSERVER

#define   SERVER_VERSION      "1.0"
#define   SERVER_NAME         "dashboard"   ///< Set your server's logical name here e.g. if myserver then address is http://myserver.local/
                                            ///< if you have 'Bonjour' running or your system supports multicast dns
                                            
//---------------------------------------------
// Variables
//---------------------------------------------
ESP32WebServer server(80);                  ///< Name/port of the server

String htmlContent = "";

const char ssid[]     = "WIFI";
const char password[] = "azertyui";

fs::FS fileSystem = SPIFFS;                 ///< File system to use to read/write file (SPIFFS = internal memory, SD = external SD Card)

File UploadFile;                            ///< File, necessary to handle file uploads


//---------------------------------------------
// Public Functions
//---------------------------------------------
void WebServer_Init();
void WebServer_Handle();


//---------------------------------------------
// Private Functions
//---------------------------------------------
// Request events functions
static void HTML_Request_Page_Home();
static void HTML_Request_Update();
static void HTML_Request_Firmware_Upload_Handle();
static void HTML_Request_File_Download();
static void HTML_Request_File_Upload();
static void HTML_Request_File_Upload_Handle();
static void HTML_Request_File_Delete();
static void HTML_Request_Files_Directory();

// Available html pages
static void HTML_Page_Home();
static void HTML_Page_Update();
static void HTML_Page_Upload();
static void HTML_Page_Files_Directory(fs::FS &fs);
static void HTML_Page_File_Delete(fs::FS &fs, String filename);
static void HTML_Page_SelectInput(String heading1, String command, String arg_calling_name);
static void HTML_Page_InfoMessage(String message, String target);

// Page Content Functions
static void HTML_Append_Header();
static void HTML_Append_Footer();
static void HTML_Append_FileDirectory(fs::FS &fs, const char * dirname, uint8_t levels);

// Upload/download functions
static void HTML_Handle_Firmware_Upload();
static void HTML_Handle_Download(fs::FS &fs, String filename);
static void HTML_Handle_Upload(fs::FS &fs);

// Communication to client functions
static void HTML_Send_Content();
static void HTML_Send_Stop();
static void HTML_Send_Header();


//---------------------------------------------
// Functions declarations
//---------------------------------------------


void WebServer_Init()
{

#ifdef DEBUG_WEBSERVER
  Serial.begin(115200);
#endif

  WiFi.softAP(ssid, password);
  
#ifdef DEBUG_WEBSERVER
  Serial.println("\nWifi AP on air : "+WiFi.SSID()+" Use IP address: "+WiFi.localIP().toString()); 
#endif  
  
  if (!MDNS.begin(SERVER_NAME))
  {
#ifdef DEBUG_WEBSERVER
    Serial.println("Error setting up MDNS responder!"); 
#endif
    ESP.restart(); 
  } 
  
  // Note: Using the ESP32 and SD_Card readers requires a 1K to 4K7 pull-up to 3v3 on the MISO line, otherwise they do-not function.

  ///////////////////////////// Server Commands 
  server.on("/",         HTML_Request_Page_Home);
  server.on("/update",   HTML_Request_Update);
  server.on("/download", HTML_Request_File_Download);
  server.on("/fwupload",  HTTP_POST,[](){ server.send(200);}, HTML_Request_Firmware_Upload_Handle);
  //server.on("/download/CSS.h", File_Download2);
  server.on("/upload",   HTML_Request_File_Upload);
  server.on("/fupload",  HTTP_POST,[](){ server.send(200);}, HTML_Request_File_Upload_Handle);
  //server.on("/stream",   File_Stream);
  server.on("/delete",   HTML_Request_File_Delete);
  server.on("/dir",      HTML_Request_Files_Directory);
  ///////////////////////////// End of Request commands
  server.begin();
  
#ifdef DEBUG_WEBSERVER
  Serial.println("HTTP server started");
#endif
}


void WebServer_Handle()
{
  // Listen for client connections
  server.handleClient(); 

#ifdef DEBUG_WEBSERVER
  if(millis()%5000 == 0)
  {
    Serial.println(".");
  }
#endif
}


static void HTML_Request_Page_Home()
{
  HTML_Page_Home();
}

static void HTML_Request_Update()
{
  HTML_Page_Update();
}


static void HTML_Request_File_Download()
{ // This gets called twice, the first pass selects the input, the second pass then processes the command line arguments
  if (server.args() > 0 ) 
  {
    if (server.hasArg("download")) 
    {
      HTML_Handle_Download(fileSystem, server.arg(0));
      
#ifdef DEBUG_WEBSERVER
      Serial.println(server.arg(0));
#endif
    }
  }
  else
  {
    HTML_Page_SelectInput("Enter filename to download","download","download");
  }
}


static void HTML_Request_File_Upload()
{
  HTML_Page_Upload();
}


static void HTML_Request_File_Upload_Handle()
{
  HTML_Handle_Upload(fileSystem);
}


static void HTML_Request_Firmware_Upload_Handle()
{
  HTML_Handle_Firmware_Upload();
}


static void HTML_Request_File_Delete()
{
  if (server.args() > 0 ) 
  {
    if (server.hasArg("delete")) 
    {
      HTML_Page_File_Delete(fileSystem, server.arg(0));
    }
  }
  else 
  {
    HTML_Page_SelectInput("Select a File to Delete","delete","delete");
  }
}


static void HTML_Request_Files_Directory()
{
  HTML_Page_Files_Directory(fileSystem);
}


static void HTML_Page_Home()
{
  HTML_Send_Header();
 /* 
  htmlContent += "      <a href='/download'><button>Download</button></a>\n");
  htmlContent += "      <a href='/upload'><button>Upload</button></a>\n");
  htmlContent += "      <a href='/stream'><button>Stream</button></a>\n");
  htmlContent += "      <a href='/delete'><button>Delete</button></a>\n");
  htmlContent += "      <a href='/dir'><button>Directory</button></a>\n");*/
  
  htmlContent += "    <script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>\n";
  htmlContent += "    <script>\n";
  htmlContent += "      $('form').load(function(e)\n";
  htmlContent += "      {\n";
  htmlContent += "        e.preventDefault();\n";
  htmlContent += "        var form = $('#upload_form')[0];\n";
  htmlContent += "        var data = new FormData(form);\n";
  htmlContent += "        $.ajax({\n";
  htmlContent += "          url: '/update',\n";
  htmlContent += "          type: 'POST',\n";
  htmlContent += "          data: data,\n";
  htmlContent += "          contentType: false,\n";            
  htmlContent += "          processData:false,\n";
  htmlContent += "          xhr: function(\n";
  htmlContent += "          {\n";
  htmlContent += "            var xhr = new window.XMLHttpRequest();\n";
  htmlContent += "            xhr.upload.addEventListener('progress', function(evt)\n";
  htmlContent += "            {\n";
  htmlContent += "              if (evt.lengthComputable)\n";
  htmlContent += "              {\n";
  htmlContent += "                var per = evt.loaded / evt.total;\n";
  htmlContent += "                $('#prg').html('progress: ' + Math.round(per*100) + '%');\n";
  htmlContent += "              }\n";
  htmlContent += "            }, false);\n";
  htmlContent += "            return xhr;\n";
  htmlContent += "          },\n";
  htmlContent += "          success:function(d, s)\n";
  htmlContent += "          {\n";
  htmlContent += "            console.log('success!')\n";
  htmlContent += "          },\n";
  htmlContent += "          error: function (a, b, c)\n";
  htmlContent += "          {\n";
  htmlContent += "          }\n";
  htmlContent += "        });\n";
  htmlContent += "      });\n";
  htmlContent += "    </script>\n";
  
  htmlContent += "    toto\n";
  
  HTML_Append_Footer();
  HTML_Send_Content();
  HTML_Send_Stop(); // Stop is needed because no content length was sent
}

static void HTML_Page_Update()
{
#ifdef DEBUG_WEBSERVER
  Serial.println("File upload stage-1");
#endif
  
  HTML_Append_Header();
  /*
  htmlContent += "    <h3>Select Firmware file</h3>\n"; 
  htmlContent += "    <FORM action='/fwupload' method='post' enctype='multipart/form-data'>\n";
  htmlContent += "    <button style='width:40%' type='file' name='fwupload' id = 'fwupload' value=''>Select Fimware File</button>";
  htmlContent += "    <br><br>\n";
  htmlContent += "    <button style='width:10%' type='submit'>Proceed Update</button>\n";
  htmlContent += "    <br>\n";
  htmlContent += "    <a href='/'>[Back]</a>\n";
  htmlContent += "    <br><br>\n";*/

  htmlContent += "    <h3>Select Firmware file</h3>\n"; 
  htmlContent += "    <form action='/fwupload' method='post' enctype='multipart/form-data'>\n";
  htmlContent += "      <input class='button' style='width:40%' type='file' name='fwupload' id = 'fwupload' value=''>";
  htmlContent += "        <button style='width:10%' type='submit'>Upload Firmware</button>\n";
  htmlContent += "      </input>\n";
  htmlContent += "    </form>\n";
  htmlContent += "    <br>\n";
  htmlContent += "    <a href='/'>[Back]</a>\n";
  htmlContent += "    <br><br>\n";
  
  HTML_Append_Footer();
  
#ifdef DEBUG_WEBSERVER
  Serial.println("File upload stage-2");
#endif
  
  server.send(200, "text/html",htmlContent);
}


static void HTML_Page_Upload()
{
#ifdef DEBUG_WEBSERVER
  Serial.println("File upload stage-1");
#endif
  
  HTML_Append_Header();
  
  htmlContent += "    <h3>Select File to Upload</h3>\n"; 
  htmlContent += "    <form action='/fupload' method='post' enctype='multipart/form-data'>\n";
  htmlContent += "      <input class='buttons' style='width:40%' type='file' name='fupload' id = 'fupload' value=''>";
  htmlContent += "        <button class='buttons' style='width:10%' type='submit'>Upload File</button>\n";
  htmlContent += "      </input>\n";
  htmlContent += "    </form>\n";
  htmlContent += "    <br>\n";
  htmlContent += "    <a href='/'>[Back]</a>\n";
  htmlContent += "    <br><br>\n";
  
  HTML_Append_Footer();
  
#ifdef DEBUG_WEBSERVER
  Serial.println("File upload stage-2");
#endif
  
  server.send(200, "text/html",htmlContent);
}


static void HTML_Page_Files_Directory(fs::FS &fs)
{
  if (SD_present) 
  { 
    File root = fs.open("/");
    
    if(!root.isDirectory())
    {
#ifdef DEBUG_WEBSERVER
        Serial.println(" - not a directory");
#endif
        
        return;
    }

    
    HTML_Send_Header();
      
    if(!root)
    {
#ifdef DEBUG_WEBSERVER
        Serial.println("- failed to open directory");
#endif

        htmlContent += "    <h3>No Files Found</h3>\n";
        return;
    }
    else 
    {
      /*
      File file = root.openNextFile();
      
      while(file)
      {
          if(file.isDirectory())
          {
              Serial.print("  DIR : ");
              Serial.println(file.name());
              
          } 
          else 
          {
              Serial.print("  FILE: ");
              Serial.print(file.name());
              Serial.print("\tSIZE: ");
              Serial.println(file.size());
          }
          
          file = root.openNextFile();
      }*/
      
      root.rewindDirectory();
      
      htmlContent += "    <h3>SD Card Contents</h3><br>\n";
      htmlContent += "    <table align='center'>\n";
      htmlContent += "      <tr>\n";
      htmlContent += "        <th>Name</th>\n";
      htmlContent += "        <th style='width:20%'>Type</th>\n";
      htmlContent += "        <th>Size</th>";
      htmlContent += "        <th>Download</th>\n";
      htmlContent += "        <th>Delete</th>\n";
      htmlContent += "      </tr>\n";
      
      HTML_Append_FileDirectory(fileSystem, "/",0);
      
      htmlContent += "    </table>\n";
      htmlContent += "    <br>\n";
      HTML_Send_Content();
      
      root.close();
    }
    
    HTML_Append_Footer();
    HTML_Send_Content();
    HTML_Send_Stop();   // Stop is needed because no content length was sent
  } 
  else 
  {
    HTML_Page_InfoMessage("No SD Card present", "/");
  }
}


static void HTML_Page_File_Delete(fs::FS &fs, String filename) 
{
  if (SD_present) 
  { 
    HTML_Send_Header();
    
    File dataFile = fs.open("/"+filename, FILE_READ); // Now read data from SD Card 

#ifdef DEBUG_WEBSERVER
    Serial.print("Deleting file: "); Serial.println(filename);
#endif
    
    if (dataFile)
    {
      if (fs.remove("/"+filename)) 
      {
#ifdef DEBUG_WEBSERVER
        Serial.println("File deleted successfully");
#endif
        
        htmlContent += "    <h3>File '" + filename + "' has been erased</h3>\n"; 
        htmlContent += "    <a href='/dir'>[Back]</a>\n";
        htmlContent += "    <br><br>";
      }
      else
      { 
        htmlContent += "    <h3>File was not deleted - error</h3>\n";
        htmlContent += "    <a href='dir'>[Back]</a>\n";
        htmlContent += "    <br><br>\n";
      }
    } 
    else
    {
      HTML_Page_InfoMessage("File does not exist", "delete");
    }
    
    HTML_Append_Footer(); 
    HTML_Send_Content();
    HTML_Send_Stop();
  } 
  else 
  {
    HTML_Page_InfoMessage("File does not exist", "delete");
  }
} 


static void HTML_Page_SelectInput(String heading1, String command, String arg_calling_name)
{
  HTML_Send_Header();
  
  htmlContent += "    <h3>" + heading1 + "</h3>\n"; 
  htmlContent += "    <form action='/" + command + "' method='post'>\n";
  htmlContent += "      <input type='text' name='" + arg_calling_name +"' value=''>\n";
  htmlContent += "        <br>\n";
  htmlContent += "        <type='submit' name='" + arg_calling_name +"' value=''>\n";
  htmlContent += "        <br><br>\n";
  htmlContent += "      </input>";
  htmlContent += "    </form>\n";
  
  HTML_Append_Footer();
  
  HTML_Send_Content();
  HTML_Send_Stop();
}


static void HTML_Page_InfoMessage(String message, String target)
{
  HTML_Send_Header();
  
  htmlContent += "    <h3>"+message+"</h3>\n"; 
  htmlContent += "    <a href='/" + target + "'>[Back]</a>\n";
  htmlContent += "    <br><br>\n";
  
  HTML_Append_Footer();
  
  HTML_Send_Content();
  HTML_Send_Stop();
}


static void HTML_Handle_Download(fs::FS &fs, String filename)
{
  if (SD_present) 
  { 
    File download = fs.open("/"+filename);
  
    if (download) 
    {
      server.sendHeader("Content-Type", "text/text");
      server.sendHeader("Content-Disposition", "attachment; filename="+filename);
      server.sendHeader("Connection", "close");
      server.streamFile(download, "application/octet-stream");
      download.close();
    } 
    else 
    {
      HTML_Page_InfoMessage("File does not exist", "download");
    }
  } 
  else 
  {
    HTML_Page_InfoMessage("No SD Card present", "/");
  }
}


static void HTML_Handle_Upload(fs::FS &fs)
{
#ifdef DEBUG_WEBSERVER
  Serial.println("File upload stage-3");
#endif
  
  HTTPUpload& uploadfile = server.upload(); // See https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WebServer/srcv
                                            // For further information on 'status' structure, there are other reasons such as a failed transfer that could be used
  if(uploadfile.status == UPLOAD_FILE_START)
  {
#ifdef DEBUG_WEBSERVER
    Serial.println("File upload stage-4");
#endif
    
    String filename = uploadfile.filename;
    
    if(!filename.startsWith("/")) 
    { 
      filename = "/"+filename;
    }
    
#ifdef DEBUG_WEBSERVER
    Serial.print("Upload File Name: "); Serial.println(filename);
#endif
    
    fs.remove(filename);                         // Remove a previous version, otherwise data is appended the file again
    UploadFile = fs.open(filename, FILE_WRITE);  // Open the file for writing in SPIFFS (create it, if doesn't exist)
    filename = String();
  }
  else if (uploadfile.status == UPLOAD_FILE_WRITE)
  {
#ifdef DEBUG_WEBSERVER
    Serial.println("File upload stage-5");
#endif
    
    if(UploadFile)
    {
      UploadFile.write(uploadfile.buf, uploadfile.currentSize); // Write the received bytes to the file
    }
  } 
  else if (uploadfile.status == UPLOAD_FILE_END)
  {
    if(UploadFile)          // If the file was successfully created
    {                                    
      UploadFile.close();   // Close the file again
      
#ifdef DEBUG_WEBSERVER
      Serial.print("Upload Size: "); Serial.println(uploadfile.totalSize);
#endif
      
      htmlContent = "";
      
      HTML_Append_Footer();
      
      htmlContent += "    <h3>File was successfully uploaded</h3>\n"; 
      htmlContent += "    <h2>Uploaded File Name: " + uploadfile.filename+"</h2>\n";
      htmlContent += "    <h2>File Size: " + File_FormatSize(uploadfile.totalSize) + "</h2>\n<br>\n"; 
      
      HTML_Append_Footer();
      
      server.send(200, "text/html", htmlContent);
    } 
    else
    {
      HTML_Page_InfoMessage("Could Not Create Uploaded File (write-protected?)", "upload");
    }
  }
}


static void HTML_Handle_Firmware_Upload()
{
  HTTPUpload& uploadfile = server.upload(); // See https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WebServer/srcv
                                            // For further information on 'status' structure, there are other reasons such as a failed transfer that could be used
  if(uploadfile.status == UPLOAD_FILE_START)
  {
#ifdef DEBUG_WEBSERVER
    Serial.println("File upload stage-4");
    Serial.printf("Update: %s\n", uploadfile.filename.c_str());
#endif      

      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) //start with max available size
      { 
        Update.printError(Serial);
      }
  }
  else if (uploadfile.status == UPLOAD_FILE_WRITE)
  {
#ifdef DEBUG_WEBSERVER
  Serial.println("File upload stage-5");
#endif
  
  // flashing firmware to ESP
  if (Update.write(uploadfile.buf, uploadfile.currentSize) != uploadfile.currentSize) 
  {
    Update.printError(Serial);
  }
} 
else if (uploadfile.status == UPLOAD_FILE_END)
{
#ifdef DEBUG_WEBSERVER
    Serial.print("Upload Size: "); Serial.println(uploadfile.totalSize);
#endif

    if (Update.end(true))//true to set the size to the current progress
    { 
#ifdef DEBUG_WEBSERVER
      Serial.printf("Update Success: %u\nRebooting...\n", uploadfile.totalSize);
#endif
      HTML_Page_InfoMessage("Update Success!\n Rebooting...", "/");
    } 
    else 
    {
#ifdef DEBUG_WEBSERVER
      Update.printError(Serial);
#endif
      HTML_Page_InfoMessage("Update Failed ...\n Rebooting...", "/");
    }
    
    ESP.restart();
  } 
  else
  {
    HTML_Page_InfoMessage("Could Not Create Uploaded File (write-protected?)", "upload");
  }
}



static void HTML_Append_Header() 
{
  htmlContent  = "<!DOCTYPE html>\n";
  htmlContent += "<html>\n";
  htmlContent += "  <head>\n";
  htmlContent += "    <title>Dashboard file Server</title>\n"; // NOTE: 1em = 16px
  htmlContent += "    <meta name='viewport' content='user-scalable=yes,initial-scale=1.0,width=device-width'>\n\n";
  htmlContent += "    <style>\n";
  htmlContent += "      body{max-width:80%;margin:0 auto;font-family:Verdana;font-size:105%;text-align:center;color:#555;background-color:#eeeeee;}\n";
  htmlContent += "      ul{list-style-type:none;margin:0.1em;padding:6px 12px 6px 12px;border-radius:12px 0 12px 0;overflow:hidden;background-color:#bbbbbb;font-size:1em;}\n";
  htmlContent += "      li{float:left;border-radius:12px 0 12px 0;border-right:0.06em solid #bbb;margin:0px 4px 0px 4px;}last-child {border-right:none;font-size:85%}\n";
  htmlContent += "      li a{display: block;border-radius:12px 0 12px 0;background:#dddddd;padding:6px 12px 6px 12px;font:bold 12px Verdana;color:#555;text-decoration:none;font-size:85%box-shadow:1px 1px 3px #999;}\n";
  htmlContent += "      li a:hover{background-color:#f5f5f5;border-radius:12px 0 12px 0;font-size:85%}\n";
  htmlContent += "      section {font-size:0.88em;}\n";
  htmlContent += "      h1{color:#555;border-radius:12px 0 12px 0;font-size:1em;padding:6px 12px 6px 12px;background:#bbbbbb;box-shadow:1px 1px 3px #999;}\n";
  htmlContent += "      h2{color:#555;font-size:1.0em;box-shadow:1px 1px 3px #999;background:#bbbbbb;}\n";
  htmlContent += "      h3{text-align:center;width:50%;height:30px;padding:6px 12px 6px 12px;border-radius:12px 0 12px 0;background:#dddddd;font-size:14px;box-shadow:1px 1px 3px #999;}\n";
  htmlContent += "      input{height:31px; float:left; border-radius:12px 0 12px 0;background: #bbbbbb;color:#555;font:bold 11px Verdana;margin:3px 0px 3px 0px;padding:1px 12px 1px 12px;box-shadow:1px 1px 3px #999;}\n";
  htmlContent += "      table{font-family:Verdana;font-size:0.9em;border-collapse:collapse;width:100%;}\n"; 
  htmlContent += "      th {text-align:center;background-color:#dddddd;border:0.06em solid #dddddd;padding:0.5em;border-bottom:0.06em solid #dddddd;}\n";
  htmlContent += "      td {text-align:left;border:0.06em solid #dddddd;padding:0.3em;border-bottom:0.06em solid #dddddd;}\n";
  htmlContent += "      tr:nth-child(even) {background-color:#f5f5f5;}\n";
  htmlContent += "      .row:after{content:'';display:table;clear:both;}\n";
  htmlContent += "      *{box-sizing:border-box;}\n";
  htmlContent += "      footer{background-color:#f5f5f5; text-align:center;padding:0.3em 0.3em;border-radius:12px 0 12px 0;font-size:60%;}\n";
  htmlContent += "      button{float:left;width:100px; border-radius:12px 0 12px 0;background: #dddddd;border:none;color:#555;font:bold 11px Verdana;margin:1px 6px 1px 6px;padding:3px 12px 3px 12px;box-shadow:1px 1px 3px #999;}\n";
  htmlContent += "      button:hover {background: #f5f5f5;cursor:pointer;}\n";
  htmlContent += "      a{font-size:75%;}\n";
  htmlContent += "      p{font-size:75%;}\n";
  htmlContent += "    </style>\n";
  htmlContent += "  </head>\n\n";
  htmlContent += "  <body>\n";
  htmlContent += "    <h1>File Server " + String(SERVER_VERSION) + "</h1>\n";
}


static void HTML_Append_Footer()
{
  htmlContent += "    <ul>\n";
  htmlContent += "      <li><a href='/'>Home</a></li>\n"; // Lower Menu bar command entries
  htmlContent += "      <li><a href='/update'>Update</a></li>\n"; 
  //htmlContent += "      <li><a href='/download'>Download</a></li>\n"; 
  htmlContent += "      <li><a href='/upload'>Upload</a></li>\n"; 
  //htmlContent += "      <li><a href='/stream'>Stream</a></li>\n"; 
  //htmlContent += "      <li><a href='/delete'>Delete</a></li>\n"; 
  htmlContent += "      <li><a href='/dir'>Directory</a></li>\n";
  htmlContent += "    </ul>\n";
  htmlContent += "  </body>\n";
  htmlContent += "</html>\n";
}


static void HTML_Append_FileDirectory(fs::FS &fs, const char * dirname, uint8_t levels)
{
  File root = fs.open(dirname);
  
  if(!root)
  {
    return;
  }
  
  if(!root.isDirectory())
  {
    return;
  }
  
  File file = root.openNextFile();

  String temp;
  
  while(file)
  {
    if (htmlContent.length() > 1000) 
    {
      HTML_Send_Content();
    }
    
    if(file.isDirectory())
    {
#ifdef DEBUG_WEBSERVER
      Serial.println(String(file.isDirectory()?"Dir ":"File ")+String(file.name()));
#endif

      htmlContent += "      <tr>\n";
      htmlContent += "        <td>"+String(file.isDirectory()?"Dir":"File")+"</td>\n";
      htmlContent += "        <td>"+String(file.name())+"</td>\n";
      htmlContent += "        <td></td>\n";      
      htmlContent += "        <td></td>\n";
      htmlContent += "      </tr>\n";
      
      HTML_Append_FileDirectory(fileSystem, file.name(), levels-1);
    }
    else
    {
      temp = String(file.name());
      temp.remove(0,1);
      
      //Serial.print(String(file.name())+"\t");
      htmlContent += "      <tr>\n";
      htmlContent += "        <td>"+ temp + "</td>\n";  
      //htmlContent += "<tr><td><form name='monForm' method='post' action='/download'>   <a title ='Download File' href='#' onclick='Go()>" + String(file.name())+"</a></form></td>";

#ifdef DEBUG_WEBSERVER
      Serial.println("post value : "+temp);
      Serial.print(String(file.isDirectory()?"Dir ":"File ")+String(file.name())+"\t");
#endif

      htmlContent += "        <td>" + String(file.isDirectory()?"Dir":"File") + "</td>\n";
      htmlContent += "        <td>" + File_FormatSize(file.size()) + "</td>\n";
      htmlContent += "        <td>\n";
      htmlContent += "          <form action='/download' method='post'>\n";
      htmlContent += "            <button type='submit' name='download' value='"+temp+"'>Download</button>\n";
      htmlContent += "          </form>\n";
      htmlContent += "        </td>\n";
      htmlContent += "        <td>\n";
      htmlContent += "          <form action='/delete' method='post'>\n";
      htmlContent += "            <button type='submit' name='delete' value='"+temp+"'>Delete</button>\n";
      htmlContent += "          </form>\n";
      htmlContent += "        </td>\n";
      htmlContent += "      </tr>\n";
      
#ifdef DEBUG_WEBSERVER
      Serial.println(File_FormatSize(file.size()));
#endif
    }
    
    file = root.openNextFile();
  }
  
  file.close();
}


static void HTML_Send_Content()
{
  server.sendContent(htmlContent);
  htmlContent = "";
}


static void HTML_Send_Stop()
{
  server.sendContent("");
  server.client().stop(); // Stop is needed because no content length was sent
}


static void HTML_Send_Header()
{
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate"); 
  server.sendHeader("Pragma", "no-cache"); 
  server.sendHeader("Expires", "-1"); 
  server.setContentLength(CONTENT_LENGTH_UNKNOWN); 
  
  server.send(200, "text/html", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves. 
  
  HTML_Append_Header();
  server.sendContent(htmlContent);
  htmlContent = "";
}


/*
void File_Stream()
{
  if (server.args() > 0 ) 
  {
    if (server.hasArg("stream")) 
    {
      SD_file_stream(server.arg(0));
    }
  }
  else 
  {
    HTML_Page_SelectInput("Enter a File to Stream","stream","stream");
  }
}


void SD_file_stream(String filename) 
{ 
  if (SD_present) 
  { 
    File dataFile = SPIFFS.open("/"+filename, FILE_READ); // Now read data from SD Card

#ifdef DEBUG_WEBSERVER
    Serial.print("Streaming file: "); Serial.println(filename);
#endif

    if (dataFile) 
    { 
      if (dataFile.available()) 
      { // If data is available and present 
        String dataType = "application/octet-stream"; 
        
        if (server.streamFile(dataFile, dataType) != dataFile.size())
        {
#ifdef DEBUG_WEBSERVER
          Serial.print("Sent less data than expected!"); 
#endif
        } 
      }
      
      dataFile.close(); // close the file: 
    } 
    else 
    {
      HTML_Page_ReportFileNotPresent("Cstream");
    }
  } 
  else 
  {
    HTML_Page_ReportSDNotPresent();
  } 
}   */
