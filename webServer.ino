

boolean setupWebServer() {

  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return false;
  }

  //hello world
  server.on("/hw", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain", "Hello, Hens");
  });
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/style.css", "text/css");
  });
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  server.on("/open", HTTP_GET, [](AsyncWebServerRequest * request) { //call the door open routine;
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  server.on("/close", HTTP_GET, [](AsyncWebServerRequest * request) { //call the door close routine;
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  server.on("/update", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/update.html", String(), false, processor);
  });
  server.on("/settings", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/settings.html", String(), false, processor);
  });

  //post?
  server.on("/saveClose", HTTP_GET, [](AsyncWebServerRequest * request) { //call close schedule save;
    request->send(SPIFFS, "/update.html", String(), false, processor);
  });

  server.on("/saveOpen", HTTP_GET, [](AsyncWebServerRequest * request) { //call open schedule save;
    request->send(SPIFFS, "/update.html", String(), false, processor);
  });

  server.on("/spring", HTTP_GET, [](AsyncWebServerRequest * request) { //update offset +1
    request->send(SPIFFS, "/settings.html", String(), false, processor);
  });

  server.on("/fall", HTTP_GET, [](AsyncWebServerRequest * request) { //update offset -1
    request->send(SPIFFS, "/settings.html", String(), false, processor);
  });
  server.on("/newWifi", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/newWifi.html", String(), false, processor);
  });


  server.on("/saveWifi", HTTP_GET, [](AsyncWebServerRequest * request) {
    const char* WSID = "wifissid";
    const char* WPWD = "wifipass";
    Serial.println("Updating wifi and restarting");
    String ssid = "";
    String passwd = "";
    if (request->hasParam(WSID)) {
      ssid = request->getParam(WSID)->value();
    }
    if (request->hasParam(WPWD)) {
      passwd = request->getParam(WPWD)->value();
    }

    if (ssid.length() > 0 && passwd.length() > 0){
      //update params, send restart OK
      Serial.print("password is:"); Serial.print(passwd); Serial.println(":");
      request->send(200, "text/plain", "creds accepted");
    } else {
      request->send(200, "text/plain", "retry");
    }
    
  });


  server.on("/notifyEmail", HTTP_GET, [](AsyncWebServerRequest * request) { //update email notification
    request->send(SPIFFS, "/settings.html", String(), false, processor);
  });

  server.on("/notifyMode", HTTP_GET, [](AsyncWebServerRequest * request) { //update email mode
    request->send(SPIFFS, "/settings.html", String(), false, processor);
  });



  server.begin();

  return true;
}
