//index1.h
const char HTML1[] PROGMEM = R"====(
<!DOCTYPE HTML>
<html>

<head>
    <title>Weather Observations</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
</head>

<body>
    <h2>Rain Gauge<br>Indianapolis, Indiana</h2>
    <br> LastUpdate: %LASTUPDATE%
    <br> Latitude: %GPSLAT%
    <br> Longitude : %GPSLNG%
    <br> Elevation:  843 Feet.
    <br> Temperature: %TEMP% F.
    <br> Dewpoint:  %DEWPOINT% F.
    <br> Heat Idex:  %HEATINDEX% F.
    <br> Humidity: %HUM% %%
    <br> Barometric Pressure: %SEALEVEL%  inHg.
    <br> Difference:  %DIF% inHg --Past 15 Minutes
    <br> Rain Day : %RAINDAY% Day/mm
    <br> Rain Hour: %RAINHOUR% Hour/mm
    <br> Rain 5 Min : %RAINFALL% Five Min/mm
    <br>
    <h2>Weather Observations</h2>
    <h3>%DTSTAMP%</h3>
    <br>
    <a href=http://%LINK%/SdBrowse >File Browser</a>
    <br>
    <a href=http://%LINK%/Graphs>Graphed Weather Observations</a>
    <br>    
    <a href=http://%LINK%/WIFI.TXT >WiFi</a>
    <br>
    <a href=http://%LINK%/RTSP >Camera View</a>
    <br>
    <a href=http://%LINK%/README.TXT >Readme</a>
    <br>
    <br>
    <a href=https://forum.arduino.cc/index.php?topic=606947.0>Project Discussion</a>
    <br>
    <br> Client IP: %CLIENTIP%
</body>

</html>
)====";
