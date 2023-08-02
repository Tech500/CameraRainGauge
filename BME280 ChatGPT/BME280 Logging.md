Google Sheet script for month to month data logging plus function to send data from ESP32 sketch to Google Sheet.

BME280 Logging

/*
This is a Google Script that retrieves environmental data (datetime, temperature, heat index, humidity, dewpoint, pressure, and a difference value) from a web 
interface and appends it to a Google Spreadsheet. The sheet name is based on the current month and year, and a new sheet will be created with an additional number
if the sheet with the same name already exists. If it's the last day of the current month, a new sheet for the next month will be created. The script has several 
functions, including getting the name of the current month, creating a new sheet with the specified name, adding headers to the new sheet, and appending the data
to the sheet.  

Google Sheets App script created in parts by Google's Bard.  

[Based on IoTDesignPro's documentation](https://iotdesignpro.com/articles/esp32-data-logging-to-google-sheets-with-google-scripts)

Additional code and Google's Bard prompting by William Lucid 07/02/2023

ESP32 Project repository:  https://github.com/Tech500/CameraRainGauge

"googleSheet" is the function in "CameraRainGauge.ino" that send data to Google Sheets "BME280 Logging."
*/

const sheet_id = "Google Sheet id";

function doGet(e) {
  const dtstamp = e.parameter.dtstamp;
  const temp = e.parameter.temp;
  const heatindex = e.parameter.heatindex;
  const humidity = e.parameter.humidity;
  const dewpoint = e.parameter.dewpoint;
  const pressure = e.parameter.pressure;
  const diff = e.parameter.diff;

const headers = ['dtstamp', 'temp', 'heatindex', 'humidity', 'dewpoint', 'pressure', 'diff'];

const data = [
[dtstamp, temp, heatindex, humidity, dewpoint, pressure, diff]
];

var now = new Date();
var month = now.getMonth();
var monthName = getMonthNames();
var year = now.getFullYear();
var endOfMonth = new Date(year, month + 1, 0);

function getMonthNames() {
  var months = ["January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"];
  var monthName = months[new Date().getMonth()];
  return monthName; 
} 

var sheetName = monthName + " " + now.getFullYear();
var ss = SpreadsheetApp.openById(sheet_id);
var sheet = ss.getSheetByName(sheetName);

function createNewSheet() {
var month = now.getMonth();
var monthName = getMonthNames()[month];
console.log(sheetName);
var sheet = SpreadsheetApp.getActive().insertSheet(sheetName);
console.log(sheet);
sheet.appendRow(headers);
for (var i = 0; i < data.length; i++) {
  // Append the data to sheet
  sheet.appendRow([dtstamp,temp,heatindex,humidity,dewpoint,pressure,diff]);
  }
}

function logData(dtstamp, temp, heatindex, humidity, dewpoint, pressure, diff) {
  var sheet = SpreadsheetApp.openById(sheet_id).getSheetByName(sheetName);
  if (!sheet) {
  sheet = SpreadsheetApp.openById(sheet_id).insertSheet(sheetName);
  }
  for (var i = 0; i < data.length; i++) {
  // Append the data to sheet
  sheet.appendRow([dtstamp,temp,heatindex,humidity,dewpoint,pressure,diff]);
  }
}

function testForEndOfMonth() {
  if (now.getDate() === endOfMonth.getDate()) {
    createNewSheet();
  }
  logData(dtstamp, temp, heatindex, humidity, dewpoint, pressure, diff);
}

testForEndOfMonth();

}

--------------------
//Arduino IDE function that sends data to Google sheets


/*
String GOOGLE_SCRIPT_ID = "deployment id"; //Deployment id, BME280 Logging (Google Sheet script.) --before setup.

void googleSheet()
{
  
    //getWeatherData;  called in the 15 Minute routine.
  
    char fahr[7];
    dtostrf(temperature, 6, 1, fahr); 
    
    char heatindex[7];
    dtostrf(heat, 6, 1, heatindex); 
    
    char humid[7]; 
    dtostrf(hum, 6, 1, humid); 
    
    char dewpoint[7]; 
    dtostrf(dew,6, 1, dewpoint); 
    
    char barometric[8]; 
    dtostrf(currentPressure, 7, 3, barometric);
    
    char diff[7]; 
    dtostrf(difference, 6, 3, diff); 

    String data = "&dtstamp="             +  dtStamp
    
                + "&temp="                +  fahr

                + "&heatindex="           +  heatindex

                + "&humidity="            +  humid

                + "&dewpoint="            +  dewpoint

                + "&pressure="            +  barometric

                + "&diff="                +  diff;
  
    String urlFinal = "https://script.google.com/macros/s/"+GOOGLE_SCRIPT_ID+"/exec?"+data;
    Serial.print("POST data to spreadsheet:");
    urlFinal.replace(" ", "%20");  //No spaces in URL; results in error if there are spaces.
    Serial.println(urlFinal);
    HTTPClient http;
    http.begin(urlFinal.c_str());
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");  //Specify content-type header
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    int httpCode = http.GET(); 
    Serial.print("HTTP Status Code: ");
    Serial.println(httpCode);
    
    //getting response from google sheet
    String payload;
    if (httpCode > 0) {
        payload = http.getString();
        Serial.println("Payload: "+payload);    
    }
    
    http.end();
    
}
*/
