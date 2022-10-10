///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//      3.9              Version  3.9 CameraRainGauge.ino 9/30/2022 @ 09:24 EDT  Developed by William  M. Lucid
//
//                       Sketch folder:  I:\Documents\CameraRainGauge
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
//                       https://observeredweather.000webhostapp.com  --Project: served by "free" Domain hosting service
//
//                       BME280 Caliabration:
//                       "Tech Note 142 – Calibrate a BME280/680 Pressure Sensor or Barometer"  --G6EJD - David   (BME280 in this sketch is calibrated for Indianapolis, IND)
//                       https://www.youtube.com/embed/Wq-Kb7D8eQ4?list=LL
//
//
//                       Note:  Uses ESP32 core by ESP32 Community, version 2.0.4; from "Arduino IDE, Board Manager."   Arduino IDE; use Board:  "Node32s" for the "HiLetGo" ESP32 Board.
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


#include <arduino.h>
#include "EEPROM.h"  //Part of version 2.0.4 ESP32 Board Manager install
#include <WiFi.h>   //Part of version 2.0.4 ESP32 Board Manager install
#include <WiFiUdp.h>  //2.0.4 ESP32 Board Manager install
#include <DNSServer.h>
#include <WebServer.h>
#include <WiFiManager.h>  //https://github.com/tzapu/WiFiManager
#include <HTTPClient.h>  //Part of version 2.0.4 ESP32 Board Manager install
#include <AsyncTCP.h>  //https://github.com/me-no-dev/AsyncTCP
#include <ESPAsyncWebServer.h>  //https://github.com/me-no-dev/ESPAsyncWebServer
#include <ESPmDNS.h> //Part of version 2.0.4 ESP32 Board Manager install
#include <FTPServer.h>  //https://github.com/dplasa/FTPClientServer
#include <HTTPClient.h>   //Part of version 2.0.4 ESP32 Board Manager install  ----> Used for Domain Web Interace
#include <sys/time.h>  // struct timeval --> Needed to sync time
#include <time.h>   // time() ctime() --> Needed to sync time
#include <FS.h>
#include <LittleFS.h>
#include <Update.h>  //2.0.4 ESP32 Board Manager install
#include <ThingSpeak.h>   //https://github.com/mathworks/thingspeak-arduino . Get it using the Library Manager
#include <BME280I2C.h>   //Use the Arduino Library Manager, get BME280 by Tyler Glenn
//Addition information on this library:  https://github.com/finitespace/BME280
#include <EnvironmentCalculations.h>  //Use the Arduino Library Manager, get BME280 by Tyler Glenn
#include <Wire.h>    //Part of version 2.0.4 ESP32 Board Manager install  -----> Used for I2C protocol
#include <Ticker.h>  //Part of version 2.0.4 ESP32 Board Manager install  -----> Used for watchdog ISR
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

unsigned long previousMillis = 0;
unsigned long interval = 30000;

int connect = 0;
int disconnect = 0;
int count = 1;
int counter = 0;
int brownout = 0;
int softReset = 0;
int powerOn = 0;

void WiFiEvent(WiFiEvent_t event) {
  //Serial.printf("[WiFi-event] event: %d\n", event);

  switch (event) {
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
      //Serial.println("Connected to access point");
      connect = 1;
      disconnect = 0;
      break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      //Serial.println("Disconnected from WiFi access point");
      disconnect = 1;
      if (event == 7) {
        disconnect = 0;
      }
      break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      Serial.print("\nObtained IP address: ");
      Serial.println(WiFi.localIP());
      break;
    default: break;
  }
}

void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info) {
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(IPAddress(info.got_ip.ip_info.ip.addr));
}

WiFiManager wifiManager;

//default custom static IP
char static_ip[16] = "10.0.0.110";
char static_gw[16] = "10.0.0.1";
char static_sn[16] = "255.255.255.0";
char static_dns[16] = "10.0.0.1";

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
FTPServer ftpSrv(LittleFS);
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

//RTC_DATA_ATTR int reconnect = 0;

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
float referencePressure = 1023.7;  // hPa local QFF (official meteor-station reading) ->  KEYE, Indianapolis, IND
float outdoorTemp = 41;           // °F  measured local outdoor temp.
float barometerAltitude = 250.698;  // meters ... map readings + barometer position  -> 824 Feet  Garmin, GPS measured Altitude.

float baroOffset = 0.08;

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

float temp(NAN), temperature, hum(NAN), pres(NAN), heatIndex, dewPoint, absHum, altitude, seaLevel;

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

  currentPressure = (seaLevel * 0.02953) + baroOffset;   //Convert from hPa to in Hg.

}

int i;
//int counted;
int countFiles;

//unsigned long int a;  <----------------------What is this?

char* fileList[30];

int error = 0;
//int countFile;
int flag = 0;
int wait = 0;

//use I2Cscanner to find LCD display address, in this case 3F   //https://github.com/todbot/arduino-i2c-scanner/
//LiquidCrystal_I2C lcd(0x3F,16,2);  // set the LCD address to 0x3F for a 16 chars and 2 line display

//#define sonalert 9  // pin for Piezo buzzer

#define online 19  //pin for online LED indicator

//int switchRelay;  //switch camera power on or off to conserve battery

//long int id = 1;  //Increments record number

char* filelist[12];

int counted;

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


/* You only need to formatLittleFS the first time you run a
   test or else use theLittleFS plugin to create a partition
   https://github.com/me-no-dev/arduino-esp32fs-plugin */

//Hardware pin definitions
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// digital I/O pins

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

void setup()
{

  Serial.begin(9600);

  while (!Serial) {}

  Serial.println("");
  Serial.println("\nVersion  3.9 CameraRainGauge.ino 9/30/2022 @ 09:24 EDT");
  Serial.println("");

  if (rtc_get_reset_reason(0) == 1)  //VBAT_RESET --brownout restart
  {

    brownout = 1;

    Serial.println("Brownout reset previous boot");

    //powerOn = 1;

  }

  if (rtc_get_reset_reason(0) == 12)  //SOFTWARE_RESET --watchdog restart
  {

    softReset = 1;

    Serial.println("Software reset previous boot");

  }

  Serial.println("\nConnecting to WiFi...");

  // delete old config
  WiFi.disconnect(true);

  delay(1000);

  WiFi.mode(WIFI_STA);

  // Examples of different ways to register wifi events
  WiFi.onEvent(WiFiEvent);
  WiFi.onEvent(WiFiGotIP, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
  WiFiEventId_t eventID = WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info) {
    //Serial.print("WiFi lost connection. Reason: \n");
    //Serial.println(info.wifi_sta_disconnected.reason);
  }, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

  // Remove WiFi event
  //Serial.print("WiFi Event ID: ");
  ////Serial.println(eventID);
  //WiFi.removeEvent(eventID);

  //reset settings - for testing
  //Must be disabled or Brownouts need networking reset from phone app/
  //wifiManager.resetSettings();

  //set static ip
  //block1 should be used for ESP8266 core 2.1.0 or newer, otherwise use block2

  //start-block1
  IPAddress _ip, _gw, _sn, _dns;
  _ip.fromString(static_ip);
  _gw.fromString(static_gw);
  _sn.fromString(static_sn);
  _dns.fromString(static_dns);
  //end-block1

  wifiManager.setSTAStaticIPConfig(_ip, _gw, _sn, _dns);

  //tries to connect to last known settings
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP" with password "password"
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect("AutoConnectAP", "password")) {
    Serial.println("failed to connect, we should reset as see if it connects");
    delay(3000);
    ESP.restart();
    delay(5000);
  }


  //if you get here you have connected to the WiFi
  Serial.println("WiFi Connected");
  Serial.println("local ip");
  Serial.println(WiFi.localIP());

  Wire.begin(21, 22);

  pinMode(25, OUTPUT);

  secondTick.attach(1, ISRwatchdog);  //watchdog ISR increase watchdogCounter by 1 every 1 second

  pinMode(online, OUTPUT);  //Set pinMode to OUTPUT for online LED

  ///////////////////////// FTP /////////////////////////////////
  //FTP Setup, ensureLittleFS is started before ftp;
  ////////////////////////////////////////////////////////////////
  if (LittleFS.begin(true))
  {

    Serial.println("LittleFS opened!");
    Serial.println("");
    ftpSrv.begin(ftpUser, ftpPassword); //username, password for ftp.  set ports in ESP8266FtpServer.h  (default 21, 50009 for PASV)

  }
  /////////////////////// End FTP//////////////////////////////


  serverAsync.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    PATH = "/FAVICON";
    //accessLog();
    if (! flag == 1)
    {
      request->send(LittleFS, "/favicon.png", "image/png");

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
                 request->sendLittleFS, "/ACCESS610.TXT");

            }
            end();
       });
  */

  serverAsync.on("/get-file", HTTP_GET, [](AsyncWebServerRequest * request) {
    PATH = fn;
    accessLog();
    if (! flag == 1)
    {

      request->send(LittleFS, fn, "text/txt");

    }
    end();

  });

  /*
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
  */


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
    request->send(LittleFS, "/index.htm");
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

  serverAsync.on("/logout", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(401);
    logon = 0;
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
        //logon = 0;
        end();
      }
      else
      {
        Update.printError(Serial);
      }
    }


  });


  // attach filesystem root at URL /fs
  //serverAsync.serveStatic("/fs",LittleFS, "/");

  // Catch-All Handlers
  // Any request that can not find a Handler that canHandle it
  // ends in the callbacks below.
  serverAsync.onNotFound(onRequest);
  serverAsync.onFileUpload(onUpload);
  serverAsync.onRequestBody(onBody);

  ///////////////////////// End OTA Support /////////////////////////////

  serverAsync.onNotFound(notFound);

  pinMode(REEDPIN, INPUT_PULLUP);
  //attachInterrupt(digitalPinToInterrupt(REEDPIN), reedSwitch_ISR, FALLING);

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

  /*
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
  */

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

  //LittleFS.format();

  //lcdDisplay();      //   LCD 1602 Display function --used for inital display

  //ThingSpeak.begin(client);

  //WiFi.disconnect();  //Used to test reconnect WiFi routine.

  //delay(75 * 1000);  //Uncomment to test watchdog

  //Serial.println("Delay elapsed");

  serverAsync.begin();

  getWeatherData();

  watchdogCounter = 0;

  powerOn = 1;

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

  digitalWrite(online, LOW);

  delay(1);

  if ((brownout == 1) || (softReset == 1))
  {

    watchdogCounter = 0;  //Resets the watchdogCounter

    getDateTime();

    //Open a "WIFI.TXT" for appended writing.   Client access ip address logged.
    File logFile = LittleFS.open("/WIFI.TXT", "a");

    if (!logFile)
    {
      Serial.println("File: '/WIFI.TXT' failed to open");
    }

    if ((brownout == 1) && (powerOn != 1))
    {

      logFile.println("ESP32 Restart caused by Brownout Detection...");

      brownout = 0;

      connect = 1;

    }

    if ((brownout == 1) && (powerOn == 1))
    {

      powerOn = 0;

      Serial.println("ESP32 Webserver Started...");

      logFile.println("ESP32 Webserver Started...");

      brownout = 0;

      connect = 1;

    }

    if (softReset == 1)
    {

      logFile.println("ESP32 Restart caused by Watchdog Event...");

      softReset = 0;

    }

    logFile.close();

  }

  unsigned long currentMillis = millis();
  // if WiFi is down, try reconnecting
  if ((WiFi.status() != WL_CONNECTED) && (currentMillis - previousMillis >= interval)) {
    Serial.print(millis());
    Serial.println("Reconnecting to WiFi...");
    watchdogCounter = 0;
    WiFi.disconnect();
    WiFi.reconnect();
    previousMillis = currentMillis;
  }



  if (connect == 1)
  {

    //Serial.println("Connect:  " + (String)connect);

    delay(1000);

    configTime();

    //Open a "WIFI.TXT" for appended writing.
    File logFile = LittleFS.open("/WIFI.TXT", "a");

    if (!logFile)
    {
      Serial.println("File: '/WIFI.TXT' failed to open");
    }

    logFile.print("WiFi Connected:       ");

    logFile.print(dtStamp);

    logFile.printf("   Connection result: %d\n", WiFi.waitForConnectResult());

    logFile.println("");

    logFile.close();

    Serial.println("Logged WiFi connection, timestamp\n");

    Serial.println("Ready...\n");

    watchdogCounter = 0;  //Resets the watchdogCounter

    connect = 0;

    counter = 1;

  }

  if ((disconnect == 1) && (counter == 1))
  {

    //Serial.println("Counter:  " + (String)counter);
    //Serial.println("Disconnect:  " + (String)disconnect);


    watchdogCounter = 0;  //Resets the watchdogCounter

    //Open "WIFI.TXT" for appended writing.   Client access ip address logged.
    File logFile = LittleFS.open("/WIFI.TXT", "a");

    if (!logFile)
    {
      Serial.println("File: '/WIFI.TXT' failed to open");
    }

    getDateTime();

    logFile.print("WiFi Disconnected:   ");

    logFile.print(dtStamp);

    logFile.print("   Connection result: ");

    logFile.println(WiFi.waitForConnectResult());

    logFile.close();

    Serial.println("\nLogged WiFi disconnection, timestamp\n");

    watchdogCounter = 0;

    disconnect = 0;
    counter = 0;

  }

  if (watchDog == 1)
  {

    portENTER_CRITICAL(&mux);
    watchdogCounter--;
    portEXIT_CRITICAL(&mux);

    getDateTime();

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

    delay(1000);

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

    watchDog = 0;

    //Executes 15 Minute routine and one Five Minute Rountine.
    if ((MINUTE % 15 == 0) && (SECOND == 0))
    {

      flag = 1;

      Serial.println("");
      Serial.println("Fifthteen minute routine");
      Serial.println(dtStamp);

      getWeatherData();

      lastUpdate = dtStamp;   //store dtstamp for use on dynamic web page
      updateDifference();  //Get Barometric Pressure difference
      logtoSD();   //Output toLittleFS  --Log toLittleFS on 15 minute interval.
      delay(10);  //Be sure there is enoughLittleFS write time
      webInterface();
      speak();

      delayTime = 2000;

      watchdogCounter = 0;

    }

    flag = 0;

  }

  if ((MINUTE == 59) && (SECOND == 59)) // one hour counter
  {
    rainHour = 0;
    rain5min = 0;
    rainFall = 0;
  }

  getDateTime();

  if ((HOUR == 23) && (MINUTE == 57) && (SECOND == 0)) //Start new kog file..
  {

    rain5min = 0;
    rainFall = 0;
    rainHour = 0;
    rainDay = 0;
    daysRain = 0;

    delayTime = 1000;

  }

  getDateTime();

  //Remove "LOG*.TXT" files; do it early (before 00:00:00) so day of week still equals 6.
  if (((HOUR == 23) && (MINUTE == 57) && (SECOND < 2)) && (DOW == 6))
  {

    delay(2000);

    listDel();  //Removes "LOG*.TXT" files from Sunday thru Saturday. do on before Midnight Saturday.

  }


  watchdogCounter = 0;  //reset watchdogCounter

  powerOn = 0;

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

  String url;

  if (!LittleFS.begin())
  {
    Serial.println("LittleFS failed to mount !");
  }

  File root = LittleFS.open("/");

  File file = root.openNextFile();

  while (file)
  {

    String file_name = file.name();

    if (file_name.startsWith("LOG"))
    {

      url += "<a href=\"";
      url += file.name();
      url += "\">";
      url += file.name();
      url += "</a>";
      url += "    ";
      url += file.size();
      url += "<br>\r\n";

    }

    file = root.openNextFile();

  }

  root.close();

  if (var == F("URLLINK"))
    return  url;

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
  File logFile = LittleFS.open(Restricted, FILE_APPEND);

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

void configTime()
{

  configTime(0, 0, udpAddress1, udpAddress2);
  setenv("TZ", "EST+5EDT,M3.2.0/2,M11.1.0/2", 3);   // this sets TZ to Indianapolis, Indiana
  tzset();

  Serial.print("wait for first valid timestamp");

  while (time(nullptr) < 100000ul)
  {
    Serial.print(".");
    delay(5000);
  }

  Serial.println("\nSystem Time set\n");

  getDateTime();

  Serial.println(dtStamp);

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
  File log = LittleFS.open(logname.c_str(), FILE_APPEND);

  if (!log)
  {
    Serial.println("File open failed");
  }

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

void listDel()
{

   File  root = LittleFS.open("/");

   File file = root.openNextFile();

    while(file)
    {

      String file_name = file.name();

      if (file_name.startsWith("LOG"))
      {      
             
        i++;
        counted = countFiles++;
        filelist[i] = strdup(file.name());
        
        Serial.println("");
        Serial.print(filelist[i]);
        Serial.print("  ");
        Serial.print(i);        
        
      }     

      file = root.openNextFile();       

    }
  
  Serial.println("\n");   

  if(counted > 0)
  {
      
      for(i = 1;i < 8; i++)  
      {

        String file_name = filelist[i];
        
        LittleFS.remove("/" + file_name);
        Serial.print("Removed:  ");
        Serial.print(filelist[i]);
        Serial.print("  ");
        //Serial.print(i);
        Serial.println("");
  
      }

      file.close();
      
      i = 0;         

  }

  counted = 0;      
      
}

float logtoSD()   //Output toLittleFS every fifthteen minutes
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
  //File log =LittleFS.open(logname.c_str(), FILE_APPEND);
  File log = LittleFS.open(logname.c_str(), FILE_APPEND);

  if (!log)
  {
    Serial.println("file 'LOG.TXT' open failed");
  }

  if ((HOUR == 0) && (MINUTE == 0) && (SECOND < 3)) //Create header
  {
    header = 1;
  }

  if (header == 1)
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
    File diffFile = LittleFS.open("DIFFER.TXT", FILE_APPEND);

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

  return (pastPressure);
}

void logWatchdog()
{

  //yield();

  Serial.println("");
  Serial.println("Watchdog event triggered.");

  Serial.println("Watchdog Restart  " + dtStamp);

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

//////////////////////////////////////////////////////////////////////////////////
//Hosted Domain, web page -code sends data for Dynamic web page every 15 Minutes
/////////////////////////////////////////////////////////////////////////////////
void webInterface()
{

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

  char alt[10]; // Buffer big enough for 9-character float
  dtostrf(altFeet, 6, 3, alt); // Leave room for too large numbers!

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

/*
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

  if(WiFi.status() != WL_CONNECTED)  // Wait for the Wi-Fi to connect
  {

    WiFi.waitForConnectResult();

  }

  while(WiFi.status() == WL_NO_SSID_AVAIL)
  {

    delay(10 * 1000);

    Serial.println("No SSID availible");

    watchdogCounter = 0;

  }

  Serial.printf("\nConnection result: %d\n", WiFi.waitForConnectResult());
  Serial.print("IP Address:  ");
  Serial.println(WiFi.localIP());
  Serial.print("Signal strength: ");
  Serial.println(WiFi.RSSI());
  Serial.println("WiFi Connected");

  watchdogCounter = 0;

  //reconnect = 1;

  }
*/
