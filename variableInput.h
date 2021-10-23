//
//   "CameraRainGauge.ino" and  
//   variableInput.h file
//   William M. Lucid   10/23/2021 @ 4:45 EDT  
//
//   RTSP video feed requires "Unreal Media Server" running on Windows PC and a video camera capable of an RTSP video feed.
//   Ports 5119 and 5135 must be allowed to pass Firewall.  
// 
//   REEDPIN 34   //Rain Gauge input pin.  Opposite side of reed switch goes to ground.

// Replace with your network details  
//const char * host  = "esp32";

// Replace with your network details
const char * ssid = "Replace_ssid";
const char * password = "Replace_password";

//Settings pertain to NTP
const int udpPort = 1337;
//NTP Time Servers
const char * udpAddress1 = "us.pool.ntp.org";
const char * udpAddress2 = "time.nist.gov";

//publicIP accessiable over Internet with Port Forwarding; know the risks!!!
//WAN IP Address.  Or use LAN IP Address --same as server ip; no Internet access. 
#define publicIP  "Replace_public ipAddress"  //Part of href link for "GET" requests
String LISTEN_PORT = "Replace_AsyncWebServer Port"; //Part of href link for "GET" requests

String linkAddress = "Replace_public ipAddress:Port";  //publicIP and PORT --AsyncWebServer Port for URL link

String ip1String = "Replace_Public ipAddress";  //Host ip address  

int PORT = 8030;  //AsyncWebServer port

//Graphing requires "FREE" "ThingSpeak.com" account..  
//Enter "ThingSpeak.com" data here....
//Example data; enter yout account data..
unsigned long int myChannelNumber = Replace_123456; 
const char * myWriteAPIKey = "Replace_E12345";

//Server settings --Fixed ipAddress
#define ip {10,0,0,110}
#define subnet {255,255,255,0}
#define gateway {10,0,0,1}
#define dns {10,0,0,1}

//webInterface --send Data to Domain, hosted web site
const char * sendData = "https://Replace_your domain hosted web site/collectdata2.php";

//FTP Credentials
const char * ftpUser = "Replace_ftpUser";
const char * ftpPassword = "Replace_ftpPassword";
 
//Restricted Access
const char* Restricted = "/Replace_any create any filename.TXT";  //Can be any filename.  
//Will be used for "GET" request path to pull up client ip list.

///////////////////////////////////////////////////////////
//   "pulicIP/LISTEN_PORT/reset" wiill restart the server
///////////////////////////////////////////////////////////

///////////////// OTA Support //////////////////////////

const char* http_username = "Replace_userName";
const char* http_password = "Replace_password";

// xx.xx.xx.xx:yyyy/login will log in; this will allow updating firmware using:
// xx.xx.xx.xx:yyyy/update
//
// xx.xx.xx.xx being publicIP and yyyy being PORT.
//
///////////////////////////////////////////////////////
