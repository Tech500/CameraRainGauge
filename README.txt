# CameraRainGauge

Project would have been impossible for me without the help of people from community; arduino.cc
Forum, Adafruit.com Forum, ESP8266 Forum, Github.com, Random Nerd Tutorials Forum.  Project started
back in the Fall of 2012; with gifted of a "Arduino Uno."  Thank you to all the people that have helped
with the project!

http://weather-3.ddns.net/Weather  Project web site servered from ESP32.

https://observations-weather.000webhostapp.com/index.php  Project servered from hosted, domain web site.

"CameraRainGauge.ino" was developed to provide a data logger accessible over the Internet, provide
capability to manage files via file transfer protocol (FTP), over-the-air (OTA) firmware updates, two
web sites, and a RTSP video feed!

Heart of the project is task management; this is accomplished using network time protocol (NTP) servers
to set ESP32 system time.  UDP packets from the NTP server are decoded into MONTH,DATE, YEAR, HOUR, MINUTE,
and SECOND.  Conditional "if" statement "if((MINUTE % 15 == 0) && (SECOND -- 0))" are used to excute tasks
at specified number of minutes and seconds; this statement evaluates "true" every fifthteen minutes and zero
seconds.  NTP is used to automatically configure time zone and daylight saving time., plus date-time stamps.

AsyncWebServer handles "GET" requests from the Internet and processing of web pages.

Most libraries are part of the ESP32 core; version 1.0.4 was used for this project.

Third party libraries:   

AsyncTCP.h //https://github.com/me-no-dev/AsyncTCP
ESPAsyncWebServer.h  //https://github.com/me-no-dev/ESPAsyncWebServer
ESP8266FTPServer.h  //https://github.com/nailbuster/esp8266FTPServer
ThingSpeak.h  //https://github.com/mathworks/thingspeak-arduino
TinyGPS++.h  //http://arduiniana.org/libraries/tinygpsplus  used with NEO m8n GPS module
BME280I2C.h  //Use Arduino IDE library manager; get BME280 by Tyler Glenn
EnvironmentalCalculations.h  //Part of BME280 library by Tyler Glenn
LiquidCrystal_I2C.h  //https://github.com/esp8266/Basic/tree/master/libraries/LiquidCrystal --optional

Imported files:  index1-7.h make up HTML web pages loaded into memory.

index1.h contains web page for displaying Main Menu options.
index2.h contains web page for displaying the URL filename links used by File Browser.
index3.h contains web page for iframes provided by "ThingSpeak.com" for graphs.
index4.h contains web page for Restarting the ESP32 Web server.
index5.h contains web page for Contact Us form.
index6.h contains web page with java script for displaying selected filename in File Browser.
index7.h contains web page for displaying RTSP "live" video feed.

variable.h provides configutable options; without having to search 2024 lines of code.

Setup:  Setups Serial connections, WiFi, GPIO pins, Interrupts, AsyncWebServer, FTP, OTA, BME280, Time
configuration, and diagnostic statements (commented out.)

LOOP:  repeats continously; running tasks based on logic statements using time dervied from UDP packets
of NTP servers; which are used to set system time.

Functions include:

AccessLog function logs activity; such as, ipAddress, requested web page, date and time of request.
Beep function enables Sonalarm to sound alert.
All Eprom functions and rain gauge coding by Muhammad Haroon.  Thank you.
End function completes HTML document of all web pages
FileStore function creates new log file at Midnight with naming convention "LOG" Month. Date. Year.
GetDateTime function process NTP< udp packets into useable units: MONTH, DATE, YEAR, HOUR, MINUTE, and  
UpdateDifference function determines difference in barometric pressure from last log entry to current barometric pressure.
LogtoSD function logs date and time, with data from BME280 sensor, dew point, heat index, and Barometric pressure differences.
LogWatchdog function records date and time watchdog event occured; writes to "WIFI.TXT."
NotFound function processes the selected filename from the File Browser, Main Menu option; dddisplay text of file.
Speak function sends data every fifthteen minutes to "ThingSpeak.com" for graphing; retrieves iFrames for graphs.
SmartDelay function gets GPS data from NEO, m8n module; after getting "good" fix data.
WebInterface function sends data every fifthteen minutes to a domain, hosted web site.
Wifi_Start function processes starting of WiFi for SETUP and reconnects.

Files created in project:

"ACCESS.TXT" log includes date and time, ipAddress, and web URL request.
"LOGxxyyzzzz" log files; naming convention "LOG" MONTH, DATE, YEAR.
"README.TXT" lists features of project.
"WIFI.TXT" logs events; WiFi coonnects, Wifi disconnects, Watchdog events, and ESP32 starts.

Video feed:

Video is captured by "Wyse Cam 3" feeding the free "Unreal Media Server" running on a Windows PC. HTML5 document embeds 
WebRTC Player; which, pulls "live" video feed from the UMS. Main Menu option "Camera View" of project website displays 
video feed.
