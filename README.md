# CameraRainGauge

Project would have been impossible for me without the help of people from community forums; Arduino.cc, Adafruit.com forum, ESP8266.com forum, Github.com. and Random Nerd Tutorials Lab forum.  Project started simple back Fall of 2012 when a friend gave me an Arduino Uno and has evolved to this project.  Thank you to all people that have helped with project. 

http://weather-3.ddns.net/Weather  Project web page severed from ESP32.

https://observations-weather.000webhostapp.com/index.php  Project: served by "free" Domain hosting service.

“CameraRainGauge.ino” was developed to provide a data Logger that is accessible over the Internet, provide capability to manage files via file transfer protocol (FTP), over-the-air (OTA) firmware updates. two web sites, and a RTSP video feed!  Project uses GPS from a NEO, m8n module; everyday, GPS data is recorded and placed on the first line of the log file for that day.  Heart of the project is task management; this is accomplished using network time protocol (NTP) servers to set ESP32 system time.  UDP packets are decoded into Months, DAYS, YEAR HOURS, MINUTES, AND SECONDS.   Conditional statements; like “if ((MINUTES % 15 == 0) && (SECONDS == 0))” are used to execute tasks at specified number of minutes and seconds; this statement evaluates “true” every 15 Minutes and 0 seconds.   NTP is used to automatically configure time zones and if daylight saving time, plus creation of a date and time stamps.  AsyncWebServer handles “GET” requests and processing of web pages.

Most libraries are part of the ESP32 core; version 1.0.4 was used for this project.  

Third party libraries: 
AsyncTCP.h  //https://github.com/me-no-dev/AsyncTCP
ESPAsyncWebServer.h  //https://github.com/me-no-dev/ESPAsyncWebServer
ESP8266FtpServer.h  //https://github.com/nailbuster/esp8266FTPServer  
ThingSpeak.h   //https://github.com/mathworks/thingspeak-arduino 
TinyGPS++.h  //http://arduiniana.org/libraries/tinygpsplus/  Used for GPS parsing
BME280I2C.h   //Use the Arduino Library Manager, get BME280 by Tyler Glenn
EnvironmentCalculations.h  //Use the Arduino Library Manager, get BME280 by Tyler Glenn
LiquidCrystal_I2C.h   https://github.com/esp8266/Basic/tree/master/libraries/LiquidCrystal --optional

Imported files index1-7.h  make up HTML web pages: stored in memory:
Index1.h contains web page for Main Menu options.
Index2.h contains web page for displaying the URL links used by file browser.
Index3.h contains web page for iframes provided by ThingSpeak.com for graphs.
Index4.h contains web page for restarting web server.
Index5.h contains web page for contact us form.
Index6.h contains web page with java script for file reader; adds header and URL options to continue.
Index7.h contains web page for displaying RTSP video feed.

variableInput.h provides configurable options; without having to search 2024 lines of code to edit a variable.

Setup:  Serial connections, WiFi, GPIO pins, interrupts, AsyncWebServer, FTP, OTA, BME280 sensor, Time configuration, and diagnostics (commented out). 

Loop repeats continuously; running tasks based on logical control statements, using timing derived from UTP packets sent from a NTP time server used to set ESP32, system time.

Functions include:
AccessLog function logs activity; such as, ipaddress, requested web page, date and time of request logging to a file.
Beep function enables sonalarm to sound alert.
All Eprom functions and rain gauge code developed by Muhammad Haroon.  Thank you Muhammad.
End function completes HTML code for all web pages.
FileStore function stores current log file at midnight and creates new log file with naming convention “LOG, month, date, year”.
ListDelete function –remove
GetDateTime function handles processing of NTP udp packets into useable units; hours, minutes, seconds, month, date and year.
UpdateDifference function determines difference in barometric pressure from last log entry to current log entry.
LogtoSD function date and time stamps log with data from BME280 sensor along with difference in barometric pressure, dew point and heat index.
LogWatchdog function records date and time watchdog event occurs; writes to “WIFI.TXT.”
NotFound function processes the selected URL filename from the SdBrowse filename list; for the file reader to display.
Speak function sends data to “ThingSpeak.com” for graphing and retrieve of iFrames produced from graphs; used to display on web page.
SmartDelay function gets GPS data from NEO m8n GPS module; after “good” GPS fix.
WebInterface function sends data every 15 minutes to a domain hosted web site.
Wifi_Start function handles process of starting WiFi in Setup and in reconnects.

Files created by project:
“ACCESS.TXT” log includes date, time, ipaddress, and web URL request.
“LOGXXYYZZZZ.TXT” log files; naming convention “LOG”, month, date, and year.
“README.TXT” lists features of project.
“SERVER.TXT” contains times server was started –remove now logged by WIFI.TXT.
WIFI.TXT logs events: WiFi disconnects, WiFi reconnected, watchdog events, and ESP32 Starts with date, time, 
 
