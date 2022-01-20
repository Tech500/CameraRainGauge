///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
//                       Version  3.5 CameraRainGauge.ino 12/27/2021 @ 15:35 EST  Developed by William  M. Lucid
//                       Sketch folder:  C:\Users\1234\Desktop\Brownout_CameraRainGauge\DOW_Brownout_CameraRainGauge

//                       Licensed under GNU GPL v3.0, https://www.gnu.org/licenses/gpl.html
//
//                       C:\Users\1234\Documents\Alfa\RainGauge\RainGauge.ino   Dated: 10/12/2021
//
//
//                       Environmental Calculations for Dewpoint, Heatindex, and Sea level Barometric Pressure.
//
//                       Smart delay for GPS  Much improved GPS ability to obtain data.
//
//                       Developing AsyncWebServer 11/07/2019; modifying with fileRead and not found function.  Adding wifi log of reconnects.
//
//                       Portion of NTP time code was developed from code provided by schufti  --of ESP8266Community
//
//                       Original listFiles and readFile functions by martinayotte of ESP8266 Community Forum.  Function readFile modified by RichardS of ESP8266 Community Forum; for ESP32.
//
//                       Thank you Pavel for your help with modifying readFile function; enabling use with AsyncWebServer!
//
//                       Time keeping functions uses NTP Time.
//
//                       GPS and rain gauge code developed by Muhammad Haroon.  Thank you Muhammad.
//
//                       Previous projects:  https://github.com/tech500
//
//                       Project is Open-Source, requires one BME280 breakout board, a NEO m8n GPS Module, and a "HiLetgo ESP-WROOM-32 ESP32 ESP-32S Development Board"
//
//                       http://weather-3.ddns.net/Weather  Project web page  --Servered from ESP32.
//
//                       https://observations-weather.000webhostapp.com/index.php  --Project: served by "free" Domain hosting service
//
//                       BME280 Caliabration:
//                       "Tech Note 142 – Calibrate a BME280/680 Pressure Sensor or Barometer"  --G6EJD - David   (BME280 in this sketch is calibrated for Indianapolis, IND)
//                       https://www.youtube.com/embed/Wq-Kb7D8eQ4?list=LL
//
//
//                       Note:  Uses ESP32 core by ESP32 Community, version 1.0.4; from "Arduino IDE, Board Manager."   Arduino IDE; use Board:  "Node32s" for the "HiLetGo" ESP32 Board.
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// ********************************************************************************
// ********************************************************************************
//
//   See library downloads for each library license.
//
// ********************************************************************************
// ********************************************************************************



#include "EEPROM.h"  //Part of version 1.0.4 ESP32 Board Manager install
#include <WiFi.h>   //Part of version 1.0.4 ESP32 Board Manager install
#include <HTTPClient.h>  //Part of version 1.0.4 ESP32 Board Manager install
#include <AsyncTCP.h>  //https://github.com/me-no-dev/AsyncTCP
#include <ESPAsyncWebServer.h>  //https://github.com/me-no-dev/ESPAsyncWebServer
#include <ESPmDNS.h> //Part of version 1.0.4 ESP32 Board Manager install
#include <SimpleFTPServer.h>  //https://github.com/nailbuster/esp8266FTPServer  -->Needed for ftp transfers
#include <HTTPClient.h>   //Part of version 1.0.4 ESP32 Board Manager install  ----> Used for Domain Web Interace
#include <WiFiUdp.h>  //1.0.4 ESP32 Board Manager install
#include <sys/time.h>  // struct timeval --> Needed to sync time
#include <time.h>   // time() ctime() --> Needed to sync time
#include "FS.h"
#include "SPIFFS.h"
#include "Update.h"  //1.0.4 ESP32 Board Manager install
#include <ThingSpeak.h>   //https://github.com/mathworks/thingspeak-arduino . Get it using the Library Manager
#include <TinyGPS++.h> //http://arduiniana.org/libraries/tinygpsplus/ Used for GPS parsing
#include <BME280I2C.h>   //Use the Arduino Library Manager, get BME280 by Tyler Glenn
//Addition information on this library:  https://github.com/finitespace/BME280
#include <EnvironmentCalculations.h>  //Use the Arduino Library Manager, get BME280 by Tyler Glenn
#include <Wire.h>    //Part of version 1.0.4 ESP32 Board Manager install  -----> Used for I2C protocol
#include <Ticker.h>  //Part of version 1.0.4 ESP32 Board Manager install  -----> Used for watchdog ISR
#include <rom/rtc.h>
//#include <LiquidCrystal_I2C.h>   //https://github.com/esp8266/Basic/tree/master/libraries/LiquidCrystal optional
#include "variableInput.h"  //Packaged with project download.  Provides editing options; without having to search 2000+ lines of code.

// Replace with your network details
//const char* host;

// Replace with your network details
//const char* ssid;
//const char* password;

#include "index1.h"  //Weather HTML; do not remove

#include "index2.h"  //SdBrowse HTML; do not remove

#include "index3.h"  //Graphs HTML; do not remove

#include "index4.h"  //Restarts server; do not remove

#include  "index5.h"  //Contactus.HTML

#include  "index6.h"  //display LOG file with hyperlink

#include "index7.h"  //display video RTSP Stream

IPAddress ipREMOTE;

///Are we currently connected?
boolean connected = false;

///////////////////////////////////////////////
WiFiUDP udp;
// local port to listen for UDP packets
//const int udpPort = 1337;
char incomingPacket[255];
char replyPacket[] = "Hi there! Got the message :-)";
//const char * udpAddress1;
//const char * udpAddress2;

#define TZ "EST+5EDT,M3.2.0/2,M11.1.0/2"

////////////////////////////////////////////////

WiFiClient client;

////////////////////////// Web Server /////////////////
//WiFiServer server(PORT);
///////////////////////////////////////////////////////

////////////////////////// FTP Server /////////////////
FtpServer ftpSrv;
///////////////////////////////////////////////////////

////////////////////////// AsyncWebServer ////////////
AsyncWebServer serverAsync(PORT);
AsyncWebSocket ws("/ws"); // access at ws://[esp ip]/ws
AsyncEventSource events("/events"); // event source (Server-Sent events)
//////////////////////////////////////////////////////

//////////////////  OTA Support ////////////////////////////////////

//const char* http_username = "____";
//const char* http_password = "_____";

//flag to use from web update to reboot the ESP
bool shouldReboot = false;
int logon;

void onRequest(AsyncWebServerRequest *request)
{
  //Handle Unknown Request
  request->send(404);
}

void onBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{
  //Handle body
}

void onUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{
  //Handle upload
}

void onEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len)
{
  //Handle WebSocket event
}

//////////////////////////////// End OEA Support //////////////////////

static const uint32_t GPSBaud = 9600;                   // Ublox GPS default Baud Rate is 9600

const double Home_LAT = 88.888888;                      // Your Home Latitude --edit with your data
const double Home_LNG = 88.888888;                      // Your Home Longitude --edit with your data
const char* WiFi_hostname = "esp32";

#define RXD2 17
#define TXD2 16

HardwareSerial uart(2);  //change to uart(2) <--GPIO pins 16 and 17

TinyGPSPlus gps;

Ticker secondTick;

portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

volatile int watchdogCounter;
volatile int watchDog = 0;

void IRAM_ATTR ISRwatchdog()
{

  portENTER_CRITICAL_ISR(&mux);
  
  watchdogCounter++;
  
  if (watchdogCounter >= 75) 
  {

    watchDog = 1;

  }

  portEXIT_CRITICAL_ISR(&mux);

}

int brownout = 0;

int DOW, MONTH, DATE, YEAR, HOUR, MINUTE, SECOND;

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

char strftime_buf[64];

String dtStamp(strftime_buf);

String lastUpdate;

unsigned long delayTime;

int lc = 0;
time_t tnow = 0;

//BME280 

// Assumed environmental values:
float referencePressure = 1021.1;  // hPa local QFF (official meteor-station reading) ->  KEYE, Indianapolis, IND 
float outdoorTemp = 35.6;           // °F  measured local outdoor temp.
float barometerAltitude = 250.698;  // meters ... map readings + barometer position  -> 824 Feet  Garmin, GPS measured Altitude.


BME280I2C::Settings settings(
   BME280::OSR_X1,
   BME280::OSR_X1,
   BME280::OSR_X1,
   BME280::Mode_Forced,
   BME280::StandbyTime_1000ms,
   BME280::Filter_16,
   BME280::SpiEnable_False,
   BME280I2C::I2CAddr_0x76
);

BME280I2C bme(settings);

float temp(NAN), temperature, hum(NAN), pres(NAN),heatIndex, dewPoint, absHum, altitude, seaLevel;

float currentPressure;
float pastPressure;
float difference;   //change in barometric pressure drop; greater than .020 inches of mercury.
float heat;   //Conversion of heatIndex to Farenheit
float dew;    //Conversion of dewPoint to Farenheit
float altFeet;   //Conversion of altitude to Feet

void getWeatherData()
{
  BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
  BME280::PresUnit presUnit(BME280::PresUnit_hPa);

  bme.read(pres, temp, hum, tempUnit, presUnit);

  EnvironmentCalculations::AltitudeUnit envAltUnit  =  EnvironmentCalculations::AltitudeUnit_Meters;
  EnvironmentCalculations::TempUnit     envTempUnit =  EnvironmentCalculations::TempUnit_Celsius;

  delay(300);

  /// To get correct local altitude/height (QNE) the reference Pressure
  ///    should be taken from meteorologic messages (QNH or QFF)
  /// To get correct seaLevel pressure (QNH, QFF)
  ///    the altitude value should be independent on measured pressure.
  /// It is necessary to use fixed altitude point e.g. the altitude of barometer read in a map
    absHum = EnvironmentCalculations::AbsoluteHumidity(temp, hum, envTempUnit);
  altitude = EnvironmentCalculations::Altitude(pres, envAltUnit, referencePressure, outdoorTemp, envTempUnit);
  dewPoint = EnvironmentCalculations::DewPoint(temp, hum, envTempUnit);
  heatIndex = EnvironmentCalculations::HeatIndex(temp, hum, envTempUnit);
  seaLevel = EnvironmentCalculations::EquivalentSeaLevelPressure(barometerAltitude, temp, pres, envAltUnit, envTempUnit);
  heat = (heatIndex * 1.8) + 32;
  dew = (dewPoint * 1.8) + 32;
  altFeet = 843;


  temperature = (temp * 1.8) + 32;  //Convert to Fahrenheit

  currentPressure = seaLevel * 0.02953;   //Convert from hPa to in Hg.
  
}

int i;
int count;
int countFiles;

//unsigned long int a;  <----------------------What is this?

char* fileList[30];

int error = 0;
int flag = 0;
int wait = 0;
int reconnect;

int started;   //Used to tell if Server has started

//use I2Cscanner to find LCD display address, in this case 3F   //https://github.com/todbot/arduino-i2c-scanner/
//LiquidCrystal_I2C lcd(0x3F,16,2);  // set the LCD address to 0x3F for a 16 chars and 2 line display

//#define sonalert 9  // pin for Piezo buzzer

#define online 19  //pin for online LED indicator

//int switchRelay;  //switch camera power on or off to conserve battery

//long int id = 1;  //Increments record number

char* filelist[12];

String logging;

char *filename;
String fn;
String uncfn;
String urlPath; 

char str[] = {0};

String fileRead;

char MyBuffer[17];

String PATH;

//String publicIP;   //in-place of xxx.xxx.xxx.xxx put your Public IP address inside quotes

//define LISTEN_PORT;  // in-place of yyyy put your listening port number
// The HTTP protocol uses port 80 by default.

/*
  This is the ThingSpeak channel number for the MathwWorks weather station
  https://thingspeak.com/channels/YourChannelNumber.  It senses a number of things and puts them in the eight
  field of the channel:

  Field 1 - Temperature (Degrees C )
  Field 2 - Humidity (%RH)
  Field 3 - Barometric Pressure (hpa)
  Field 4 - Rain Last 5 Minutes  (mm)
*/


/* You only need to format SPIFFS the first time you run a
   test or else use the SPIFFS plugin to create a partition
   https://github.com/me-no-dev/arduino-esp32fs-plugin */

//Hardware pin definitions
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// digital I/O pins

float gpslng = 0.0, gpslat = 0.0, gpsalt = 0.0;
int gpssat = 0;

//Variables used for GPS
//unsigned long age;
int gps_year, gps_month, gps_day;
int seconds_new, minutes_new, hours_new, gps_year_new, gps_month_new, gps_day_new;

//Calibrate rain bucket here
//Rectangle raingauge from Sparkfun.com weather sensors
//float rain_bucket_mm = 0.011*25.4;//Each dump is 0.011" of water
//DAVISNET Rain Collector 2
//float rain_bucket_mm = 0.01*25.4;//Each dump is 0.01" of water  //Convert inch to millmeter (0.01 * 25.4)

// volatiles are subject to modification by IRQs
//volatile unsigned long raintime, rainlast, raininterval, rain, Rainindtime, Rainindlast;  // For Rain
//int addr=0;

#define eeprom_size 512

String eepromstring = "0.00";

//for loop
//int i;

unsigned long lastSecond, last5Minutes;
float lastPulseCount;
int currentPulseCount;
float rain5min;
float rainFall;
float rainHour;
float rainDay;
float daysRain;

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//Interrupt routines (these are called by the hardware interrupts, not by the main code)
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#define FIVEMINUTES (300*1000L)
#define REEDPIN 34   //was 32 Touch pin
#define REEDINTERRUPT 0

volatile int pulseCount_ISR = 0;



void IRAM_ATTR reedSwitch_ISR()
{
  static unsigned long lastReedSwitchTime;
  // debounce for a quarter second = max. 4 counts per second
  if (labs(millis() - lastReedSwitchTime) > 250)
  {
    portENTER_CRITICAL_ISR(&mux);
    pulseCount_ISR++;

    lastReedSwitchTime = millis();
    portEXIT_CRITICAL_ISR(&mux);
  }

}

void setup(void)
{

  Serial.begin(9600);

  while (!Serial) {}

  started = 0;   //Server started reset

  Serial.println("");
  Serial.println("\n\nVersion  3.5 CameraRainGauge.ino 12/27/2021 @ 15:35 EST");
  Serial.println("Please wait; for network connection...");
  Serial.println("");

  if (rtc_get_reset_reason(0) == 12)  //RTCWDT_BROWN_OUT_RESET) 
  {
  
	  brownout = 1;

    Serial.println("Brownout reset previous to boot");
  
  }

  wifi_Start();

  started = 1;

  Wire.begin(21, 22);

  pinMode(25, OUTPUT);

  secondTick.attach(1, ISRwatchdog);  //watchdog ISR increase watchdogCounter by 1 every 1 second

  pinMode(online, OUTPUT);  //Set pinMode to OUTPUT for online LED

  ///////////////////////// FTP /////////////////////////////////
  //FTP Setup, ensure SPIFFS is started before ftp;
  ////////////////////////////////////////////////////////////////
#ifdef ESP32       //esp32 we send true to format spiffs if cannot mount
  if (SPIFFS.begin(true))
  {
#elif defined ESP8266
  if (SPIFFS.begin())

#endif
    Serial.println("SPIFFS opened!");
    Serial.println("");
    ftpSrv.begin(ftpUser, ftpPassword);    //username, password for ftp.  set ports in ESP8266FtpServer.h  (default 21, 50009 for PASV)

  }
  /////////////////////// End FTP//////////////////////////////


  serverAsync.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    PATH = "/FAVICON";
    //accessLog();
    if (! flag == 1)
    {
      request->send(SPIFFS, "/favicon.png", "image/png");

    }
    //end();
  });

  /*
  
  serverAsync.on("/", HTTP_GET, [](AsyncWebServerRequest * request)
  {

    PATH = "/";
    accessLog();

    ipREMOTE = request->client()->remoteIP();

    if (! flag == 1)
    {
      request->send_P(200, PSTR("text/html"), HTML1, processor1);
    }
    end();
  });

  */

  serverAsync.on("/Weather", HTTP_GET, [](AsyncWebServerRequest * request)
  {

    PATH = "/Weather";
    accessLog();
    
    if (! flag == 1)
    {

      request->send_P(200, PSTR("text/html"), HTML1, processor1);

    

      ipREMOTE = request->client()->remoteIP();

    }
    end();

    //switchRelay = 0;  //turn off camera

  });


  serverAsync.on("/SdBrowse", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    
    PATH = "/SdBrowse";
    accessLog();

    if (! flag == 1)
    {
      request->send_P(200, PSTR("text/html"), HTML2, processor2);
     
    }
    end();
  });

  
  serverAsync.on("/Show", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    
    if (! flag == 1)
    {
      
      request->send_P(200, PSTR("text/html"), HTML6, processor6);
      
    }
    //end();
  });
  
  serverAsync.on("/RTSP", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    PATH = "/RTSP";
    accessLog();
    if (! flag == 1)
    {
      
      request->send_P(200, PSTR("text/html"), HTML7, processor7);
      
    }

    end();

    //switchRelay = 1;  // turn on camera

  });
  

  serverAsync.on("/Graphs", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    PATH = "/Graphs";
    accessLog();
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", HTML3, processor3);
    response->addHeader("Server", "ESP Async Web Server");
    if (! flag == 1)
    {
      request->send(response);

    }
    end(); 
  });

  serverAsync.on("/Contactus", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    PATH = "/Contactus";
    accessLog();
    if (! flag == 1)
    {
      request->send_P(200, PSTR("text/html"), HTML5, processor5);

    }
    end();
  });

  /*
       serverAsync.on("/ACCESS.TXT", HTTP_GET, [](AsyncWebServerRequest * request)
       {
            PATH = "/ACCESS";
            accessLog();
            AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", HTML3);
            response->addHeader("Server","ESP Async Web Server");
            if(! flag == 1)
            {
                 request->send(SPIFFS, "/ACCESS610.TXT");

            }
            end();
       });
  */

  serverAsync.on("/get-file", HTTP_GET, [](AsyncWebServerRequest *request){
  PATH = fn;
  accessLog();
  if (! flag == 1)
  {

      request->send(SPIFFS, fn, "text/txt");

  }
  end();

  });

  serverAsync.on("/RESTART", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    PATH = "/RESTART";
    accessLog();
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", HTML4);
    response->addHeader("Server", "ESP Async Web Server");
    if (! flag == 1)
    {
      request->send(response);

    }
    Serial2.flush();
    end();
    ESP.restart();

  });

  ///////////////////// OTA Support //////////////////////

  //attach.AsyncWebSocket
  ws.onEvent(onEvent);
  serverAsync.addHandler(&ws);

  // attach AsyncEventSource
  serverAsync.addHandler(&events);

  // respond to GET requests on URL /heap
  serverAsync.on("/heap", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send(200, "text/plain", String(ESP.getFreeHeap()));
  });

  // upload a file to /upload
  serverAsync.on("/upload+-", HTTP_POST, [](AsyncWebServerRequest * request)
  {
    request->send(200);
  }, onUpload);

  // send a file when /index is requested
  serverAsync.on("/index", HTTP_ANY, [](AsyncWebServerRequest * request)
  {
    request->send(SPIFFS, "/index.htm");
  });

  // HTTP basic authentication
  serverAsync.on("/login", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    PATH = "/login";
    accessLog();
    if (!request->authenticate(http_username, http_password))
      return request->requestAuthentication();
    request->send(200, "text/plain", "Login Success; upload firmware!");
    logon = 1;
    end();
  });

  // Simple Firmware Update Form
  serverAsync.on("/update", HTTP_GET, [](AsyncWebServerRequest * request)
  {

    PATH = "/update";
    accessLog();
    if (logon == 1)
    {
      request->send(200, "text/html", "<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>");
      logon = 0;
      end();
    }
    else
    {
      request->send(404); //Sends 404 File Not Found
      logon = 0;
      end();
    }


  });

  serverAsync.on("/update", HTTP_POST, [](AsyncWebServerRequest * request)
  {
    shouldReboot = !Update.hasError();

    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", shouldReboot ? "OK" : "FAIL");
    response->addHeader("Connection", "close");
    request->send(response);
  }, [](AsyncWebServerRequest * request, String filename, size_t index, uint8_t *data, size_t len, bool final)
  {

    if (!index)
    {
      Serial.printf("Update Start: %s\n", filename.c_str());
      //Update.runAsync(true);
      if (!Update.begin((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000))
      {
        Update.printError(Serial);
      }
    }
    if (!Update.hasError())
    {
      if (Update.write(data, len) != len)
      {
        Update.printError(Serial);
        end();
      }
    }
    if (final)
    {
      if (Update.end(true))
      {
        Serial.printf("Update Success: %uB\n", index + len);
        end();
      }
      else
      {
        Update.printError(Serial);
      }
    }


  });


  // attach filesystem root at URL /fs
  serverAsync.serveStatic("/fs", SPIFFS, "/");

  // Catch-All Handlers
  // Any request that can not find a Handler that canHandle it
  // ends in the callbacks below.
  serverAsync.onNotFound(onRequest);
  serverAsync.onFileUpload(onUpload);
  serverAsync.onRequestBody(onBody);

  //serverAsync.begin();

  ///////////////////////// End OTA Support /////////////////////////////

  serverAsync.onNotFound(notFound);  

  Serial2.end();

  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);

  configTime(0, 0, udpAddress1, udpAddress2);
  setenv("TZ", "EST+5EDT,M3.2.0/2,M11.1.0/2", 3);   // this sets TZ to Indianapolis, Indiana
  tzset();


  Serial.print("wait for first valid timestamp ");

  while (time(nullptr) < 100000ul)
  {
    Serial.print(".");
    delay(5000);
  }

  Serial.println("\nSystem Time set");

  getDateTime();

  Serial.println(dtStamp);

  pinMode(REEDPIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(REEDPIN), reedSwitch_ISR, FALLING);

  // initialize EEPROM with predefined size
  EEPROM.begin(eeprom_size);

  //RESET EEPROM CONTENT - ONLY EXECUTE ONE TIME - AFTER COMMENT

  /*

       Uncomment to 'clear'.eeprom values.

       Serial.println("CLEAR ");
       eepromClear();
       Serial.println("SET ");
       eepromSet("rain5min", "0.00");
       eepromSet("rainDay", "0.00");
       eepromSet("rainHour", "0.00");
       Serial.println("LIST ");
       Serial.println(eepromList());
  */

  //END - RESET EEPROM CONTENT - ONLY EXECUTE ONE TIME - AFTER COMMENT
  //eepromClear();

  //GET STORED RAINCOUNT IN EEPROM
  Serial.println("");
  Serial.println("GET EEPROM --Setup");
  eepromstring = eepromGet("rainDay");
  rainDay = eepromstring.toFloat();
  Serial.print("RAINDAY VALUE FROM EEPROM: ");
  Serial.println(rainDay);

  eepromstring = eepromGet("rainHour");
  rainHour = eepromstring.toFloat();
  Serial.print("RAINHOUR VALUE FROM EEPROM: ");
  Serial.println(rainHour);

  eepromstring = eepromGet("rain5min");
  rain5min = eepromstring.toFloat();
  Serial.print("rain5min VALUE FROM EEPROM: ");
  Serial.println(rain5min);
  Serial.println("");
  //END - GET STORED RAINCOUNT IN EEPROM
  
  while (!bme.begin()) 
  {
    Serial.println("Could not find BME280 sensor!");
    delayTime = 1000;
  }

  // bme.chipID(); // Deprecated. See chipModel().
  switch (bme.chipModel())
  {
    case BME280::ChipModel_BME280:
      Serial.println("Found BME280 sensor! Success.");
      break;
    case BME280::ChipModel_BMP280:
      Serial.println("Found BMP280 sensor! No Humidity available.");
      break;
    default:
      Serial.println("Found UNKNOWN sensor! Error!");
  }

  
  
  Serial.println("Getting GPS Ready...");

  delay(60 * 1000);  //delay for GPS to power-up; when powered off.  <-------------------------------------------------

  //SPIFFS.format();

  //lcdDisplay();      //   LCD 1602 Display function --used for inital display

  //ThingSpeak.begin(client);

  //delay(30 * 1000);  //Used to test reconnect WiFi routine.  Will produce one entry for each disconnect in "WIFI.TXT."

  //WiFi.disconnect();  //Used to test reconnect WiFi routine.  Will produce one entry for eac disconnect in "WIFI.TXT."

  //delay(75 * 1000);  //Uncomment to test watchdog

  //Serial.println("Delay elapsed");

  serverAsync.begin();

  getWeatherData();

  watchdogCounter = 0;
  
}

void loop() 
{

  //udp only send data when connected
  if (connected)
  {

    //Send a packet
    udp.beginPacket(udpAddress1, udpPort);
    udp.printf("Seconds since boot: %u", millis() / 1000);
    udp.endPacket();
  }

  delay(1);

  /*

  if(switchRelay == 1)
  {

    digitalWrite(online, HIGH);

    //Need to turn off camera in case "Weather" is not reqested!!!

    getDateTime();
    
    if((switchRelay === 1 ) && (MINUTE % 3 == 0))
    {

      digitalWrite(online, LOW);

    }

  }
  
  if(switchRelay == 0)
  {

     digitalWrite(online, LOW);

  }

  */


  if(brownout == 1)
  {
        
    watchdogCounter = 0;  //Resets the watchdogCounter
    
    getDateTime();
    
    //Open a "WIFI.TXT" for appended writing.   Client access ip address logged.
    File logFile = SPIFFS.open("/WIFI.TXT", "a");

    if (!logFile)
    {
      Serial.println("File: '/WIFI.TXT' failed to open");
    }
    
    logFile.print("ESP32 Restart caused by Brownout Detection...\n");  

    logFile.close();

    brownout = 2;
    
  }

  if ((started == 1) || brownout == 2)
  {

    watchdogCounter = 0;  //Resets the watchdogCounter

    delay(1000);

    getDateTime();

    digitalWrite(online, HIGH);  //Indicates when Server is "Ready" for Browser Requests.
    
    delay(3000);

    getWeatherData();
   
    smartDelay(1000);

    if (millis() > 5000 && gps.charsProcessed() < 10)
    {

      Serial.println(F("No GPS data received: check wiring"));

      smartDelay(1000);

    }
    else
    {
      
      gpslat = gps.location.lat();
      gpslng = gps.location.lng();
      //gpsalt = gps.altitude.feet();
      
      Serial.println("GPS Ready");  

    }

    digitalWrite(online, LOW);    

    brownout = 0;
            
  }
  
  if ((WiFi.status() != WL_CONNECTED) || (WiFi.status() == WL_NO_SSID_AVAIL )  || (WiFi.status() == WL_CONNECTION_LOST))
  {

    watchdogCounter = 0;  //Resets the watchdogCounter
    
    getDateTime();

    //Open a "WIFI.TXT" for appended writing.   
    File logFile = SPIFFS.open("/WIFI.TXT", "a");

    if (!logFile)
    {
      Serial.println("File: '/WIFI.TXT' failed to open");
    }

    logFile.print("WiFi Disconnected: ");

    logFile.print(dtStamp);

    //logFile.print("   Connection result: ");

    //logFile.println(WiFi.waitForConnectResult());
    logFile.printf("   Connection result: %d\n", WiFi.waitForConnectResult());
    
    logFile.close();

    wifi_Start();

    brownout = 0;

    watchDog = 0;
    
  }

  if (reconnect == 1)
  {

    watchdogCounter = 0;  //Resets the watchdogCounter

    //Open "WIFI.TXT" for appended writing.   Client access ip address logged.
    File logFile = SPIFFS.open("/WIFI.TXT", "a");

    if (!logFile)
    {
      Serial.println("File: '/WIFI.TXT' failed to open");
    }

    if((started == 1) || brownout == 1)
    {
      
      getDateTime();
      
      if(started == 1)
      {
      
        logFile.print("ESP32 Started:        ");  //Created for Power on restarts and log Brown out restarts too... 

      }
      else
      {
      
         logFile.print("WiFi Reconnected:    ");
        
      }
      
      logFile.print(dtStamp);

      logFile.print("   Connection result: ");

      logFile.println(WiFi.waitForConnectResult());

      logFile.println("");

      logFile.close();

      reconnect = 0;

      brownout = 0;

      watchDog = 0;

    }

  }

  if ( watchDog == 1)
	{

		portENTER_CRITICAL(&mux);
		watchdogCounter--;
		portEXIT_CRITICAL(&mux);

		logWatchdog();
		
	}

  for (int x = 1; x < 5000; x++)
  {
    ftpSrv.handleFTP();
  }

  ///////////////////////// OTA Support ///////////////////////

  if (shouldReboot)
  {
    Serial.println("Rebooting...");
    delay(100);
    ESP.restart();
  }
  static char temp[128];
  sprintf(temp, "Seconds since boot: %u", millis() / 1000);
  events.send(temp, "time"); //send event "time"

  //////////////////// End OTA Support /////////////////////////

  // each second read and reset pulseCount_ISR
  if (millis() - lastSecond >= 1000)
  {
    lastSecond += 1000;
    portENTER_CRITICAL(&mux);
    currentPulseCount += pulseCount_ISR; // add to current counter
    pulseCount_ISR = 0; // reset ISR counter
    rainFall = currentPulseCount * .047; //Amout of rain in one bucket dump.
    portEXIT_CRITICAL(&mux);

  }

  // each 5 minutes save data to another counter
  if (millis() - last5Minutes >= FIVEMINUTES)
  {

    rain5min = rainFall;
    rainHour = rainHour + rainFall;  //accumulaing 5 minute rainfall for 1 hour then reset -->rainHour Rainfall
    rainDay = rainDay + rainFall;  //aacumulating 1 day rainfall
    last5Minutes += FIVEMINUTES; // remember the time
    lastPulseCount += currentPulseCount; // add to last period Counter
    currentPulseCount = 0;; // reset counter for current period

  }

  getDateTime();

  //Executes 5 Minute Routine.
  if ((MINUTE % 5 == 0) && (SECOND == 0))
  {

    flag = 1;

    delay(2000);

    Serial.println("");
    Serial.println("Five Minute routine");
    Serial.println(dtStamp);

    //STORE RAINCOUNT IN EEPROM
    Serial.println("SET EEPROM rainHour");
    eepromstring = String(rainHour, 2);
    eepromSet("rainHour", eepromstring);
    //END - STORE RAINCOUNT IN EEPROM

    //STORE RAINCOUNT IN EEPROM
    Serial.println("SET EEPROM rainDay");
    eepromstring = String(rainDay, 2);
    eepromSet("rainDay", eepromstring);
    //END - STORE RAINCOUNT IN EEPROM

    //STORE RAINCOUNT IN EEPROM
    Serial.println("SET EEPROM rain5min");
    eepromstring = String(rain5min, 2);
    eepromSet("rain5min", eepromstring);
    //END - STORE RAINCOUNT IN EEPROM

    rainFall = 0;
    rain5min = 0;

    flag = 0;

    watchDog = 0;

    //Executes 15 Minute routine and one Five Minute Rountine.
    if ((MINUTE % 15 == 0) && (SECOND < 5)) 
    {

      flag = 1;

      Serial.println("");
      Serial.println("Fifthteen minute routine");
      Serial.println(dtStamp);

      delayTime = 1000;

      getWeatherData();

      lastUpdate = dtStamp;   //store dtstamp for use on dynamic web page
      updateDifference();  //Get Barometric Pressure difference
      logtoSD();   //Output to LittleFS  --Log to LittleFS on 15 minute interval.
      delay(10);  //Be sure there is enough LittleFS write time
      webInterface();
      speak();

      flag = 0;

      watchDog = 0;

    }

  }
   
  if ((MINUTE == 59) && (SECOND == 59)) // one hour counter
  {
    rainHour = 0;
    rain5min = 0;
    rainFall = 0;
  }

  getDateTime();
  
   if ((HOUR == 23) && (MINUTE == 57) && (SECOND == 0) && (DOW == 6))
   {
	   
     removeFiles();  //Removes "LOGxxyyzzzz.TXT" files from Sunday thru Saturday. do before Midnight Saturday.

   }	 

   getDateTime();	    

  if ((HOUR == 23) && (MINUTE == 59) && (SECOND == 0)) //Start new kog file..
  {

    fileStore();

    SPIFFS.remove("/WIFI.TXT");

    rain5min = 0;
    rainFall = 0;
    rainHour = 0;
    rainDay = 0;
    daysRain = 0;

    delay(1000);

  }

  started = 0;

  watchdogCounter = 0;  //reset watchdogCounter

}

String processor1(const String& var)
{

  //index1.h

  if (var == F("LASTUPDATE"))
    return lastUpdate;

  if (var == F("GPSLAT"))
    return String(gpslat, 5);

  if (var == F("GPSLNG"))
    return String(gpslng, 5);

  if (var == F("TEMP"))
    return String(temperature, 1);

  if (var == F("HEATINDEX"))
    return String(heat, 1);

  if (var == F("DEWPOINT"))
    return String(dew, 1);

  if (var == F("HUM"))
    return String(hum);

  if (var == F("SEALEVEL"))
    return String(currentPressure, 2);

  if (var == F("DIF"))
    return String(difference, 3);


  if (var == F("RAINDAY"))
    return String(rainDay);

  if (var == F("RAINHOUR"))
    return String(rainHour);

  if (var == F("RAINFALL"))
    return String(rainFall);

  if (var == F("DTSTAMP"))
    return dtStamp;

  if (var == F("LINK"))
    return linkAddress;

  if (var == F("CLIENTIP"))
    return ipREMOTE.toString().c_str();

  return String();

}

String processor2(const String& var)
{

  //index2.h

  String str;

  File root = SPIFFS.open("/");

  root.rewindDirectory();  //added 6/17/2021

  File file = root.openNextFile();

  
  //file.seek(0, SeekSet);

  while (file)
  {

    if (strncmp(file.name(), "/LOG", 4) == 0)
    {
      
      str += "<a href=\"";
      str += file.name();
      str += "\">";
      str += file.name();
      str += "</a>";
      str += "    ";
      str += file.size();
      str += "<br>\r\n";

    }

    file = root.openNextFile();
  }

  root.close();

  root.rewindDirectory();

  if (var == F("URLLINK"))
    return  str;

  if (var == F("LINK"))
    return linkAddress;

  if (var == F("FILENAME"))
    return  file.name();

  return String();

}

String processor3(const String& var)
{

  //index3.h

  if (var == F("LINK"))
    return linkAddress;

  return String();

}

String processor4(const String& var)
{

  //index4.h

  if (var == F("LINK"))
    return linkAddress;

  return String();

}

String processor5(const String& var)
{

  //index5.h

   return String();

}

String processor6(const String& var)
{

  //index6.h

  if (var == F("FN"))
    return fn;

  if (var == F("LINK"))
    return linkAddress;

  return String();

}

String processor7(const String& var)
{

  //index7.h

  if (var == F("LINK"))
    return linkAddress;

  return String();

}

void accessLog() 
{

  digitalWrite(online, HIGH);  //turn on online LED indicator

  getDateTime();

  String ip2String = ipREMOTE.toString().c_str();   //client remote IP address
  String returnedIP = ip2String;
  
  Serial.println("");
  Serial.println("Client connected:  " + dtStamp);
  Serial.print("Client IP:  ");
  Serial.println(returnedIP);
  Serial.print("Path:  ");
  Serial.println(PATH);
  Serial.println(F("Processing request"));

  //Open a "access.txt" for appended writing.   Client access ip address logged.
  File logFile = SPIFFS.open(Restricted, FILE_APPEND);

  if (!logFile)
  {
    Serial.println("File 'ACCESS.TXT'failed to open");
  }

  if ((ip1String == ip2String) || (ip2String == "0.0.0.0") || (ip2String == "(IP unset)"))
  {

    //Serial.println("HOST IP Address match");
    logFile.close();

  }
  else
  {

    Serial.println("Log client ip address");

    logFile.print("Accessed:  ");
    logFile.print(dtStamp);
    logFile.print(" -- Client IP:  ");
    logFile.print(returnedIP);
    logFile.print(" -- ");
    logFile.print("Path:  ");
    logFile.print(PATH);
    logFile.println("");
    logFile.close();

  }

}

void beep(unsigned char delayms)
{

  //     wait for a delayms ms
  //     digitalWrite(sonalert, HIGH);
  //     delayTime = 3000;
  //     digitalWrite(sonalert, LOW);

}

//----------------------------- EEPROM -------------- Muhammad Haroon --------------------------------

void eepromSet(String name, String value)
{
  Serial.println("eepromSet");

  String list = eepromDelete(name);
  String nameValue = "&" + name + "=" + value;
  //Serial.println(list);
  //Serial.println(nameValue);
  list += nameValue;
  for (int i = 0; i < list.length(); ++i)
  {
    EEPROM.write(i, list.charAt(i));
  }
  EEPROM.commit();
  Serial.print(name);
  Serial.print(":");
  Serial.println(value);

  delayTime = 1000;

}


String eepromDelete(String name)
{
  Serial.println("eepromDelete");

  int nameOfValue;
  String currentName = "";
  String currentValue = "";
  int foundIt = 0;
  char letter;
  String newList = "";
  for (int i = 0; i < 512; ++i)
  {
    letter = char(EEPROM.read(i));
    if (letter == '\n')
    {
      if (foundIt == 1)
      {
      }
      else if (newList.length() > 0)
      {
        newList += "=" + currentValue;
      }
      break;
    }
    else if (letter == '&')
    {
      nameOfValue = 0;
      currentName = "";
      if (foundIt == 1)
      {
        foundIt = 0;
      }
      else if (newList.length() > 0)
      {
        newList += "=" + currentValue;
      }

    }
    else if (letter == '=')
    {
      if (currentName == name)
      {
        foundIt = 1;
      }
      else
      {
        foundIt = 0;
        newList += "&" + currentName;
      }
      nameOfValue = 1;
      currentValue = "";
    }
    else
    {
      if (nameOfValue == 0)
      {
        currentName += letter;
      }
      else
      {
        currentValue += letter;
      }
    }
  }

  for (int i = 0; i < 512; ++i)
  {
    EEPROM.write(i, '\n');
  }
  EEPROM.commit();
  for (int i = 0; i < newList.length(); ++i)
  {
    EEPROM.write(i, newList.charAt(i));
  }
  EEPROM.commit();
  Serial.println(name);
  Serial.println(newList);
  return newList;
}

void eepromClear()
{
  Serial.println("eepromClear");
  for (int i = 0; i < 512; ++i)
  {
    EEPROM.write(i, '\n');
  }
}

String eepromList()
{
  Serial.println("eepromList");
  char letter;
  String list = "";
  for (int i = 0; i < 512; ++i)
  {
    letter = char(EEPROM.read(i));
    if (letter == '\n')
    {
      break;
    }
    else
    {
      list += letter;
    }
  }
  Serial.println(list);
  return list;
}

String eepromGet(String name)
{
  Serial.println("eepromGet");

  int nameOfValue;
  String currentName = "";
  String currentValue = "";
  int foundIt = 0;
  String value = "";
  char letter;
  for (int i = 0; i < 512; ++i)
  {
    letter = char(EEPROM.read(i));
    if (letter == '\n')
    {
      if (foundIt == 1)
      {
        value = currentValue;
      }
      break;
    }
    else if (letter == '&')
    {
      nameOfValue = 0;
      currentName = "";
      if (foundIt == 1)
      {
        value = currentValue;
        break;
      }
    }
    else if (letter == '=')
    {
      if (currentName == name)
      {
        foundIt = 1;
      }
      else
      {
      }
      nameOfValue = 1;
      currentValue = "";
    }
    else
    {
      if (nameOfValue == 0)
      {
        currentName += letter;
      }
      else
      {
        currentValue += letter;
      }
    }
  }
  Serial.print(name);
  Serial.print(":");
  Serial.println(value);
  return value;
}

void seteeprom()
{

  eepromstring = String(rainDay, 2);
  eepromSet("rainDay", eepromstring);

  rain5min = 0;

  eepromstring = String(rainHour, 2);
  eepromSet("rainHour", eepromstring);

  eepromstring = String(rain5min, 2);
  eepromSet("rain5min", eepromstring);


  //END - STORE RAINCOUNT IN EEPROM

}

//------------------------------- end EEPROM --------- Muhammad Haroon -------------------------------------

void end()
{

  delay(1000);

  digitalWrite(online, LOW);   //turn-off online LED indicator

  getDateTime();

  Serial.println("Client closed:  " + dtStamp);

}

void fileStore()   //If Midnight, rename "LOGXXYYZZ.TXT" to ("log" + month + day + ".txt") and create new, empty "LOGXXYYZZ.TXT"
{

  int temp;
  String Date;
  String Month;

  temp = (DATE);
  if (temp < 10)
  {
    Date = ("0" + (String)temp);
  }
  else
  {
    Date = (String)temp;
  }

  temp = (MONTH);
  if (temp < 10)
  {
    Month = ("0" + (String)temp);
  }
  else
  {
    Month = (String)temp;
  }

  String logname;  //file format /LOGxxyyzzzz.txt
  logname = "/LOG";
  logname += Month; ////logname += Clock.getMonth(Century);
  logname += Date;   ///logname += Clock.getDate();
  logname += YEAR;
  logname += ".TXT";

  //Open file for appended writing
  File log = SPIFFS.open(logname.c_str(), FILE_APPEND);

  if (!log)
  {
    Serial.println("File open failed");
  }

}

void removeFiles()
{

  File root = SPIFFS.open("/");

  File file = root.openNextFile();

  while(file)
  {

      if(strncmp(file.name(), "/LOG", 4) == 0)
      {      
             
        i++;
	      count = countFiles++;
        filelist[i] = strdup(file.name());
        
        /*
        Serial.println("");
        Serial.print(filelist[i]);
        Serial.print("  ");
        Serial.print(i);
        */        
        
      }     

      file = root.openNextFile();       

  }

  Serial.println("\n");   

  if(count > 0)
  {
      
      for(i = 1;i < count + 2; i++)  //Delete only first four files; keep from getting too many log files.
      {
        
        SPIFFS.remove(filelist[i]);
        Serial.print("Removed:  ");
        Serial.print(filelist[i]);
        Serial.print("  ");
        //Serial.print(i);
        Serial.println("");
  
      }

      i = 0;         

  }

  count = 0;      
      
}


//////////////////////////////////
//Get Date and Time
//////////////////////////////////
String getDateTime()
{
  struct tm *ti;

  tnow = time(nullptr) + 1;
  //strftime(strftime_buf, sizeof(strftime_buf), "%c", localtime(&tnow));
  ti = localtime(&tnow);
  DOW = ti->tm_wday;
  YEAR = ti->tm_year + 1900;
  MONTH = ti->tm_mon + 1;
  DATE = ti->tm_mday;
  HOUR  = ti->tm_hour;
  MINUTE  = ti->tm_min;
  SECOND = ti->tm_sec;

  strftime(strftime_buf, sizeof(strftime_buf), "%a , %m/%d/%Y , %H:%M:%S %Z", localtime(&tnow));
  dtStamp = strftime_buf;
  return (dtStamp);

}

//////////////////////////////////////////////////////
//Pressure difference for fifthteen minute interval
/////////////////////////////////////////////////////
float updateDifference()  //Pressure difference for fifthteen minute interval
{


  //Function to find difference in Barometric Pressure
  //First loop pass pastPressure and currentPressure are equal resulting in an incorrect difference result.  Output "...Processing"
  //Future loop passes difference results are correct

  difference = currentPressure - pastPressure;  //This will be pressure from this pass thru loop, pressure1 will be new pressure reading next loop pass
  if (difference == currentPressure)
  {
    difference = 0;
  }
  
  return (difference); //Barometric pressure change in inches of Mecury

}

////////////////////
//Write to SPIFSS
///////////////////
float logtoSD()   //Output to SPIFFS every fifthteen minutes
{


  //getDateTime();

  int header = 0;;
  
  int tempy;
  String Date;
  String Month;

  tempy = (DATE);
  if (tempy < 10)
  {
    Date = ("0" + (String)tempy);
  }
  else
  {
    Date = (String)tempy;
  }

  tempy = (MONTH);
  if (tempy < 10)
  {
    Month = ("0" + (String)tempy);
  }
  else
  {
    Month = (String)tempy;
  }

  String logname;
  logname = "/LOG";
  logname += Month; ////logname += Clock.getMonth(Century);
  logname += Date;   ///logname += Clock.getDate();
  logname += YEAR;
  logname += ".TXT";

  // Open a "log.txt" for appended writing
  //File log = SPIFFS.open(logname.c_str(), FILE_APPEND);
  File log = SPIFFS.open(logname.c_str(), FILE_APPEND);

  if (!log)
  {
    Serial.println("file 'LOG.TXT' open failed");
  }

  if ((HOUR == 0) && (MINUTE == 0) && (SECOND < 3)) //Create header
  {
      header = 1;
  }
  
  if(header == 1)
  { 
    
      log.println("");
      log.print("Lat: ");
      log.print(gpslat, 5);
      log.print(" , ");
      log.print("Long: ");
      log.print(gpslng, 5);
      log.print("\tElevation: 843 Feet");
      log.print("\tIndianapolis, IN  ,  ");
      log.println(dtStamp);
      log.println("");
      
      header = 0;

  }

  log.print(lastUpdate);
  log.print(" , ");

  log.print("Temperature:  ");
  log.print(temperature, 1);
  log.print(" F. , ");

  log.print("Heatindex:  ");
  log.print(heat, 1);
  log.print(" F. , ");

  log.print("Humidity:  ");
  log.print(hum, 1);
  log.print(" % , ");

  log.print("Dewpoint:  ");
  log.print(dew, 1);
  log.print(" F. , ");

  log.print("Barometer:  ");
  log.print(currentPressure, 3);
  log.print(" inHg. ");
  log.print(" , ");

      if (pastPressure == currentPressure)
      {
        log.print("0.000");
        log.print(" Difference ");
        log.print(" ,");
      }
      else
      {
        log.print(difference, 3);
        log.print(" Diff. inHg ");
        log.print(", ");
      }

  log.print(" Day ");
  log.print(rainDay, 2);
  log.print(" ,");

  log.print(" Hour ");
  log.print(rainHour, 2);
  log.print(" , ");

  log.print(" Five Minute ");
  log.print(rain5min, 2);
  log.println("");

  //Increment Record ID number
  //id++;

  Serial.println("");

  Serial.println("Data written to  " + logname + "  " + dtStamp);

  log.close();

  pastPressure = currentPressure;   //Store currentPressure in variable pastPressure.

  if (difference >= .020) //After testing and observations of Data; rapid change in Weather
  {
    // Open a "Differ.txt" for appended writing --records Barometric Pressure change difference and time stamps
    File diffFile = SPIFFS.open("DIFFER.TXT", FILE_APPEND);

    if (!diffFile)
    {
      Serial.println("file 'DIFFER.TXT' open failed");  
    }

    Serial.println("");
    Serial.print("Difference greater than .020 inches of Mecury ,  ");
    Serial.print(difference, 3);
    Serial.print("  ,");
    Serial.print(dtStamp);

    diffFile.println("");
    diffFile.print("Difference greater than .020 inches of Mecury,  ");
    diffFile.print(difference, 3);
    diffFile.print("  ,");
    diffFile.print(dtStamp);
    diffFile.close();

  }    beep(50);  //Duration of Sonalert tone

  return(pastPressure);
}

void logWatchdog()
{

  yield();

  Serial.println("");
  Serial.println("Watchdog event triggered.");

  // Open a "WIFI.TXT" for appended writing
  File log = SPIFFS.open("/WIFI.TXT", "a");

  if (!log)
  {
    Serial.println("file 'WIFI.TXT' open failed");
  }

  getDateTime();

  log.print("Watchdog Event:  ");
  log.print("  ");
  log.print(dtStamp);
  log.println("\n");
  log.close();

  Serial.println("Watchdog Restart  " + dtStamp);
  Serial.println("\n");

  ESP.restart();

}

String notFound(AsyncWebServerRequest *request)
{

  digitalWrite(online, HIGH);   //turn-on online LED indicator

  if (! request->url().endsWith(F(".TXT")))
  {
    request->send(404);
  }
  else
  {
    if (request->url().endsWith(F(".TXT")))
    {
      //.endsWith(F(".txt")))

      // here comes some mambo-jambo to extract the filename from request->url()
      int fnsstart = request->url().lastIndexOf('/');

      fn = request->url().substring(fnsstart);

      uncfn = fn.substring(1);

      urlPath = linkAddress + "/" + uncfn;            

    }    

  }
  
  request->redirect("/Show");

  digitalWrite(online, LOW);   //turn-off online LED indicator 

  return fn; 

}

////////////////////////////////////////
//ThingSpeak.com --Graphing and iftmes
///////////////////////////////////////
void speak()
{

  char t_buffered1[14];
  dtostrf(temp, 7, 1, t_buffered1);

  char t_buffered2[14];
  dtostrf(hum, 7, 1, t_buffered2);

  char t_buffered3[14];
  dtostrf(currentPressure, 7, 1, t_buffered3);

  char t_buffered4[14];
  dtostrf(rain5min, 7, 1, t_buffered4);

  // Write to ThingSpeak. There are up to 8 fields in a channel, allowing you to store up to 8 different
  // pieces of information in a channel.  Here, we write to field 1.
  // Write to ThingSpeak. There are up to 8 fields in a channel, allowing you to store up to 8 different
  // pieces of information in a channel.  Here, we write to field 1.
  ThingSpeak.setField(1, t_buffered1);  //Temperature
  ThingSpeak.setField(2, t_buffered2);  //Humidity
  ThingSpeak.setField(3, t_buffered3);  //Barometric Pressure
  ThingSpeak.setField(4, t_buffered4);  //Dew Point F.

  // Write the fields that you've set all at once.
  //ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

  Serial.println("");
  Serial.println("Sent data to Thingspeak.com  " + dtStamp + "\n");

}

// This custom version of delay() ensures that the gps object
// is being "fed".
static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do
  {
    while (Serial2.available())
      gps.encode(Serial2.read());
    // TinyGPS has told us we have received a valid GPS fix. We can now query it for position/course/time/etc. information.
    gpslat = gps.location.lat();
    gpslng = gps.location.lng();
    gpsalt = gps.altitude.feet();
  } while (millis() - start < ms);
}

//////////////////////////////////////////////////////////////////////////////////
//Hosted Domain, web page -code sends data for Dynamic web page every 15 Minutes
/////////////////////////////////////////////////////////////////////////////////
void webInterface()
{

  gpslat = gps.location.lat();
  gpslng = gps.location.lng();
  
  char glat[10]; // Buffer big enough for 9-character float
  dtostrf(gpslat, 9, 4, glat); // Leave room for too large numbers!

  char glng[10]; // Buffer big enough for 9-character float
  dtostrf(gpslng, 9, 4, glng); // Leave room for too large numbers!

  char fahr[7];// Buffer big enough for 9-character float
  dtostrf(temperature, 6, 1, fahr); // Leave room for too large numbers!

  char heatindex[7];// Buffer big enough for 9-character float
  dtostrf(heat, 6, 1, heatindex); // Leave room for too large numbers!

  char humidity[7]; // Buffer big enough for 9-character float
  dtostrf(hum, 6, 1, humidity); // Leave room for too large numbers!

  char dewpoint[7]; // Buffer big enough for 9-character float
  dtostrf(dew, 6, 1, dewpoint); // Leave room for too large numbers!

  char barometric[9]; // Buffer big enough for 7-character float
  dtostrf(currentPressure, 8, 3, barometric); // Leave room for too large numbers!

  char diff[9]; // Buffer big enough for 7-character float
  dtostrf(difference, 8, 3, diff); // Leave room for too large numbers!

  char rain5[10]; // Buffer big enough for 9-character float
  dtostrf(rain5min, 6, 3, rain5); // Leave room for too large numbers!

  char rain60[10]; // Buffer big enough for 9-character float
  dtostrf(rainHour, 6, 3, rain60); // Leave room for too large numbers!

  char rain24[10]; // Buffer big enough for 9-character float
  dtostrf(rainDay, 6, 3, rain24); // Leave room for too large numbers!

  char alt[9]; // Buffer big enough for 9-character float
  dtostrf(altFeet, 8, 1, alt); // Leave room for too large numbers!

  String data = "&last="                  +  (String)lastUpdate

                + "&glat="                +  glat

                + "&glng="                +  glng

                + "&fahr="                +  fahr

                + "&heatindex="           +  heatindex

                + "&humidity="            +  humidity

                + "&dewpoint="            +  dewpoint

                + "&barometric="          +  barometric

                + "&diff="                +  diff

                + "&rain5="               +  rain5

                + "&rain60="              +  rain60

                + "&rain24="              +  rain24

                + "&alt="                 +  alt;




  if (WiFi.status() == WL_CONNECTED)
  {
    //Check WiFi connection status

    HTTPClient http;    //Declare object of class HTTPClient

    http.begin(sendData);      //Specify request destination
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");  //Specify content-type header

    int httpCode = http.POST(data);   //Send the request
    String payload = http.getString();                  //Get the response payload

    if (httpCode == 200)
    {
      Serial.print("");
      Serial.print("HttpCode:  ");
      Serial.print(httpCode);   //Print HTTP return code
      Serial.print("  Data echoed back from Hosted website  " );
      Serial.println("");
      Serial.println(payload);    //Print payload response

      http.end();  //Close HTTPClient http
    }
    else
    {
      Serial.print("");
      Serial.print("HttpCode:  ");
      Serial.print(httpCode);   //Print HTTP return code
      Serial.print("  Domain website data update failed.  ");
      Serial.println("");

      http.end();   //Close HTTPClient http
    }

  }
  else
  {

    Serial.println("Error in WiFi connection");

  }

}

void wifi_Start()
{

  //WiFi.mode(WIFI_OFF);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  //WiFi.persistent(true);

  Serial.println();
  Serial.print("MAC: ");
  Serial.println(WiFi.macAddress());

  // We start by connecting to a WiFi network
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  
  //setting the addresses
  IPAddress ip;
  IPAddress gateway;
  IPAddress subnet;
  IPAddress dns;

  WiFi.config(ip, gateway, subnet, dns);
 
  Serial.println("network...");

  while(WiFi.status() == WL_CONNECTION_LOST) {};
  
  while(WiFi.status() == WL_NO_SSID_AVAIL)
  {
     
    delay(10 * 1000);
    
    Serial.println("No SSID availible");
    
    watchdogCounter = 0;   
             
  }

  watchdogCounter = 0;  //Resets the watchdogCounter

  Serial.printf("\nConnection result: %d\n", WiFi.waitForConnectResult());
  Serial.print("IP Address:  ");
  Serial.println(WiFi.localIP());
  Serial.print("Signal strength: ");
  Serial.println(WiFi.RSSI());
  Serial.println("WiFi Connected");

  reconnect = 1; 
 
}