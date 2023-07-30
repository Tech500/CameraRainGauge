//index7.h
const char HTML7[] PROGMEM = R"====(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
<style>
  html, body {
  margin: 0; /* Set margin to 0 to remove any unwanted spacing */
  padding: 0;
  height: 100%;
}

.container {
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: space-between;
  height: 100vh; /* Use viewport height for container */
}

  @media (max-width: 768px) {
  .container {
    flex: 1 1 auto;
    flex-direction: row;
    flex-wrap: wrap;
    align-items: center;
    justify-content: center;
  }
}
    
    header {
      margin: 5vh auto 0;
      width: 100%;
      text-align: center;
    }
    
    main {
      display: flex;
      align-items: center;
      justify-content: center;
    }
    
    iframe {
      aspect-ratio: 16 / 9;
      width: 1300px; /* Set the width to 100% to occupy the entire container */
      flex: 1; /* Use flex: 1 to make the iframe expand and fill the available space */
    }

    footer {
      width: 100%;
      text-align: center;
      padding: 10px; /* Add some padding to the footer for spacing */
    }
</style>
</head>
<body>
  <div class="container">
    <header>
      <br><br>This is original H264 video encoded by IP camera; server doesn't do any transcoding.
      <br>Wyse Cam v3; providing webRTC video feed.  Camera maybe offline; depending on battery discharge state.
      <br><br>
    </header>
    <main>
      <iframe class="iframe" width="1300" height="731"src="http://68.45.250.167:8889/backyard" frameborder="0"></iframe> 
    </main>
    <footer>
      <h2><a href="http://%LINK%/Weather" >ESP32 Server</a></h2>  
    </footer>
  </div>
</body>
</html>
)====";
