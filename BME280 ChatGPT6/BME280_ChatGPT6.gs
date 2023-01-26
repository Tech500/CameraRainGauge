  // BME280 ChatGPT6 Google App Script

  // This Google App Script collects data from a web service and appends it to a Google Sheet.
  // It uses the doGet() function to parse the data and append it to the sheet. 
  // The getSheetForMonth() function is used to get the current sheet for the month or create 
  // a new one if it doesn't exist.
  // The getMonthName() function is used to get the name of the month for the given month number.
  // The addNewSheetAtEndOfMonth() function is used to create a new sheet at the end of each month, 
  // allowing the script to collect data for a full year.

  // Google App Script produced using ChatGPT and ChatGPT-3.

  var sheet_id = "replace with sheet_id"; 
  const now = new Date();   
  // Get the current sheet for the month
  var sheet = getSheetForMonth(now.getMonth());
  function doGet(e){
  var ss = SpreadsheetApp.openById(sheet_id);  
  var dtstamp = String(e.parameter.dtstamp);
  var temp = Number(e.parameter.temp);
  var heatindex = Number(e.parameter.heatindex);
  var humidity = Number(e.parameter.humidity);
  var dewpoint = Number(e.parameter.dewpoint);
  var pressure = Number(e.parameter.pressure);
  var diff = Number(e.parameter.diff);
  
  // Call the addNewSheetAtEndOfMonth() function
  addNewSheetAtEndOfMonth();
	  
  try {
    // Append the data to the sheet
    sheet.appendRow([dtstamp,temp,heatindex,humidity,dewpoint,pressure,diff]);
  } catch (error) {
    // If there is an error, create a new sheet with an additional number to the month name
    var sheetName = getMonthName(now.getMonth()) + " " + now.getFullYear();
    var newSheetName = sheetName;
    var i = 2;
    while(SpreadsheetApp.getActive().getSheetByName(newSheetName) != null) {
        newSheetName = sheetName + " " + i;
        i++;
    }
    sheet = SpreadsheetApp.getActive().insertSheet(newSheetName);
    // Add headers to the new sheet
    sheet.appendRow([dtstamp,temp,heatindex,humidity,dewpoint,pressure,diff]);
	sheet.appendRow([dtstamp,temp,heatindex,humidity,dewpoint,pressure,diff]);
  }
}

function getSheetForMonth(month) {
  // Get the name of the sheet for the given month
  var sheetName = getMonthName(month) + " " + now.getFullYear();
  
  // Get the sheet with the specified name, or create a new one if it doesn't exist
  var sheet = SpreadsheetApp.getActive().getSheetByName(sheetName);
  if (sheet == null) {
    sheet = SpreadsheetApp.getActive().insertSheet(sheetName);
    sheet.appendRow(["dtstamp","temp","heatindex","humidity","dewpoint","pressure","diff"]);
  }
  
  return sheet;
}

function getMonthName(month) {
  // Get the name of the month for the given month number
  var monthNames = ["January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"];
  return monthNames[month];
}

function addNewSheetAtEndOfMonth() {
  var now = new Date();
  if (now.getDate() === new Date(now.getFullYear(), now.getMonth() + 1, 0).getDate()) {
    var sheetName = getMonthName(now.getMonth() + 1) + " " + now.getFullYear();
    var sheet = SpreadsheetApp.getActive().insertSheet(sheetName);
    sheet.appendRow(["dtstamp","temp","heatindex","humidity","dewpoint","pressure","diff"]);
  }
}
