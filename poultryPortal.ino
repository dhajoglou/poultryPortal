/* all of the main library references along with config stuff in this .h file */
#include "doorConfig.h"



/* wifi and ntp */
ESP32Timer ITimer0(0);
boolean LDRMODE = false;
//WiFiServer server(80);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpPool);
String prevTime; //clock variable for showing each second
int32_t utcOffset;


/* webserver and spifs stuff */
AsyncWebServer server(80);


/* display and encoder */
//Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC); //Use hardware SPI and specify the chip selet and data command pins
DisplayModule disp = DisplayModule(TFT_CS, TFT_DC, -1);
ESP32Encoder encoder;
InputDebounce encoder_btn;
int16_t lastEncoderValue = 0;
int16_t currentEncoderValue;
unsigned long encoderWaitBuffer; //after we turn the encoder, pause .25 seconds
uint8_t CURRENTSCREEN = 0;
uint8_t PREVIOUS_SCREE = 0;
unsigned long buttonMillis;


Preferences prefs;
timeTrigger schedule;

/*
   Door Control variables
*/
volatile int doorState = D_CRACKED; //default presumtpin that the door is not actually closed
volatile int motorState = M_UNKNOWN;



void setup() {
  prefs.begin("cdoor", false);

  

  // Check to see if we are brand new based on if the offset is actually set. Otherwise, set the offset and continue:
  if ((utcOffset = prefs.getInt(pUtcOffset, 50)) == 50) {
    //seed the offset with 0 and tell everyone else
    utcOffset = 0;
    prefs.putInt(pUtcOffset, 0);
    prefs.putULong(pLastBoot, 0);
  }

  time_t t = prefs.getULong(pLastBoot);
  char lastSeen[20];
  strftime(lastSeen, 20, "%Y-%m-%d %H:%M:%S", localtime(&t));


  Serial.begin(115200);
  //delay(1000);
  setupPins();
  disp.startDisp();
  disp.printStartMsg();
  disp.printTftSimple("Last Boot: ", true);
  disp.printTftSimple(lastSeen, true);
  if (!setupWifi()) {
    //error that time isn't set
    disp.clearDisp();
    disp.printWifiFail();
    delay(2000);
    disp.printLdrMode();
    LDRMODE = true;
    ntpUDP.stop();
    WiFi.mode(WIFI_OFF);
  } else {
    time_t t = timeClient.getEpochTime();
    prefs.putULong("lastBoot", t);
    loadSchedule();
    setupWebServer();

  }
  disp.clearDisp();

  ESP32Encoder::useInternalWeakPullResistors = UP;
  encoder.attachHalfQuad(ENC_A, ENC_B);
  encoder_btn.registerCallbacks(encPressCallback, encReleaseCallback, NULL, NULL);
  encoder_btn.setup(ENC_BTN, BUTTON_DEBOUNCE_DELAY, InputDebounce::PIM_INT_PULL_UP_RES, 100);
  
  //encoder.setCount(0);
}

void encPressCallback(uint8_t pinIn){
  Serial.println("enc pressed");
}
void encReleaseCallback(uint8_t pinIn){
  
}





void loop() {

  /*
     General Loop opearation
     First, check the current sensor
     Second, check the door state
     Third, check for any API/UI calls
     Fourth, check the schedule or, LDR values if we're not in wifi
     Fifth, check the rotary encoder for menu actions
     Sixth, update the display
     Seventh, check for time updates
     Eight, update temp/battery data
  */
  // put your main code here, to run repeatedly:
  //delay(1000);
  //getTemp();
  //getLDR();
  //Serial.print(digitalRead(B_LIM_SW));
  //Serial.println(digitalRead(T_LIM_SW));

  String dPosition = "Closed";
  String dState = "Idle";
  char nextAction[100] = "Open at dawn";
  String fTime = "Wifi Off";
  String battCharge = "12.5v (ok)";
  if (!LDRMODE) {
    fTime = timeClient.getFormattedTime();
    //if door is open, lookup close time. else, lookup open time:
    sprintf(nextAction, "Open at %02d:%02dh", schedule.openHour, schedule.openMin);
  }

  //check to see if we switched screens
  currentEncoderValue = encoder.getCount();
  if (currentEncoderValue != lastEncoderValue) {
    Serial.print("NEW ENC VAL: ");
    Serial.println(currentEncoderValue);

    if (currentEncoderValue > lastEncoderValue){
      CURRENTSCREEN++;
    } else{
      CURRENTSCREEN--;
    }
    lastEncoderValue = currentEncoderValue;
    //set new display screen
    
    disp.setPeepBuffer(CURRENTSCREEN);
  }
  buttonMillis = millis();
  encoder_btn.process(buttonMillis);

  switch (CURRENTSCREEN) {
    case DEFAULT_SCREEN:
      if (!fTime.equals(prevTime)) {
        prevTime = fTime;
        disp.defaultScreen(battCharge, dPosition, dState, fTime, nextAction);
      }
      break;
    case CONTROL_DOOR_SCREEN:
      disp.printPeep();
      break;
  }
  /*
  if (CURRENTSCREEN == DEFAULT_SCREEN) {
    //only update once per second
    if (!fTime.equals(prevTime)) {
      updatDefTime(tft, fTime);
      prevTime = fTime;
    }
  } else if (CURRENTSCREEN < 1)  {
    CURRENTSCREEN = defaultScreen(tft, battCharge, dPosition, dState, fTime, nextAction);
  }
  */

  doorStateUpdate();

}

volatile boolean stopWifiConn = false;
void IRAM_ATTR wifiSetupTimerHandler0(void)
{
  stopWifiConn = true;
}

boolean setupWifi() {
  boolean connected = false;
  String ssid = prefs.getString(pWifiSsid, "");
  String pass = prefs.getString(pWifiPass, "");

  uint8_t wifiAttempts = 0;
  while (wifiAttempts < WIFI_RETRY && !connected) {
    wifiAttempts++;
    delay(5000); //wait 5 seconds before any connect attempt

    if (ITimer0.attachInterruptInterval(WIFI_TIMER_TIMEOUT_MS * 1000, wifiSetupTimerHandler0)) {
      Serial.println("Starting WIFI WATCH / ITimer0 OK, millis() = " + String(millis()));
    } else {
      Serial.println(F("Can't set WIFI WATCH/ITimer0. Select another freq. or timer"));
      return false;
    }
    disp.printWifiStart();
    char wifiMsgs[100];
    sprintf(wifiMsgs, "Connecting to %s (%d)", ssid, wifiAttempts);
    disp.printTftSimple(wifiMsgs, true);
    WiFi.begin(ssid.c_str(), pass.c_str());
    while (WiFi.status() != WL_CONNECTED && !stopWifiConn) {
      delay(500);
      disp.printTftSimple(".", false);
    }
    //stop the watchdog
    ITimer0.stopTimer();
    if (stopWifiConn) {
      //wifi connection didn't succeed, return false
      stopWifiConn = false;
      connected = false;
    } else {
      sprintf(wifiMsgs, "\nWifi Connected\nIP Address:%s", WiFi.localIP().toString().c_str());
      disp.printTft(wifiMsgs, -1, 0, ILI9341_GREEN, 2, true);
      connected =  true;
      Serial.print("IP Address: ");
      Serial.println(WiFi.localIP());
      delay(1000);
    }
  }
  if (connected) {
    //utcOffset = prefs.getInt(pUtcOffset, -6);
    timeClient.setTimeOffset(SECONDS_PER_HOUR * utcOffset);
    timeClient.setUpdateInterval(NTP_UPDATE_INTERVAL);
    timeClient.begin();
    timeClient.update();
    disp.printTime(timeClient.getFormattedTime());
  }
  return connected;
}


/*
   Set the pin mode on the ones we have to manually configure
   SPI bus and the encoder pins are set by their respective libraries
*/
void setupPins() {
  pinMode(B_LIM_SW, INPUT_PULLUP);
  pinMode(T_LIM_SW, INPUT_PULLUP);
  pinMode(TEMP, INPUT);
  pinMode(BATT12V, INPUT);
  pinMode(MOTOR_CURR, INPUT);
  pinMode(LDR, INPUT);
  pinMode(RELAY_1_2, OUTPUT);
  digitalWrite(RELAY_1_2, HIGH);
  pinMode(RELAY_3_4, OUTPUT);
  digitalWrite(RELAY_3_4, HIGH);
  pinMode(RELAY_5, OUTPUT);
  digitalWrite(RELAY_5, HIGH);
  pinMode(RELAY_6, OUTPUT);
  digitalWrite(RELAY_6, HIGH);
  pinMode(RELAY_7, OUTPUT);
  digitalWrite(RELAY_7, HIGH);
  pinMode(ENC_BTN,INPUT);
}

float getTemp() {
  float aVal = analogRead(TEMP) / 1023.0;
  float tmpC = (aVal - 0.5) * 100;
  float tmpF = (tmpC * 1.8) + 32;

  Serial.print("temp c:");
  Serial.print(tmpC);
  Serial.print(", tmp f:");
  Serial.println(tmpF);
  return tmpC;
}

float getLDR() {
  int ldr = analogRead(LDR);
  Serial.println(ldr);
  return 0.0;
}

void loadSchedule() {
  int openHour = prefs.getChar("oHour", -1);
  int openMinute = prefs.getChar("oMin", -1);
  int closeHour = prefs.getChar("cHour", -1);
  int closeMinute = prefs.getChar("cMin", -1);

  if (openHour < 0) {
    schedule.openHour = 8;
    schedule.openMin = 0;
    prefs.putChar("oHour", schedule.openHour);
    prefs.putChar("oMin", schedule.openMin);

  }  else {
    schedule.openHour = openHour;
    schedule.openMin = openMinute;
  }

  if (closeHour < 0) {
    schedule.closeHour = 20;
    schedule.closeMin = 0;
    prefs.putChar("cHour", schedule.closeHour);
    prefs.putChar("cMin", schedule.closeMin);
  } else {
    schedule.closeHour = closeHour;
    schedule.closeMin = closeMinute;
  }
}

/*
   Called from the rest API.  Update the hour and minute of the schedule
*/
void updateSchedule(int h, int m, int SCH) {
  switch (SCH) {
    case OSCH:
      prefs.putChar("oHour", h);
      prefs.putChar("oMin", m);
      break;
    case CSCH:
      prefs.putChar("cHour", h);
      prefs.putChar("cMin", m);
      break;
  }
  loadSchedule();
}

/*
   The following is the open/close/state api and logic for the door controler
*/

/**
   These API calls can be issued via the webapp, scheduler, or the
   controls on the door itself.
*/
void stopMotorApi() {
  digitalWrite(RELAY_1_2, HIGH);
  digitalWrite(RELAY_3_4, HIGH);
}
void closeDoorApi() {
  doorStateUpdate();
  if (doorState == D_CLOSED) {
    return;
  }
  digitalWrite(RELAY_1_2, LOW);
  digitalWrite(RELAY_3_4, HIGH);
}
void openDoorApi() {
  doorStateUpdate();
  if (doorState == D_OPENED) {
    return;
  }
  digitalWrite(RELAY_1_2, HIGH);
  digitalWrite(RELAY_3_4, LOW);
}

void doorStateUpdate() {

  //first check the fault state if both switches are closed/disconnected
  if ( !(digitalRead(B_LIM_SW) && digitalRead(T_LIM_SW)) ) {
    stopMotorApi();
    Serial.println("FAULT. BOTH LIMIT SWITCHES ARE \"PRESSED\"");
    return;
  }
  //next, check the motor state
  if ( digitalRead(RELAY_1_2) == LOW && digitalRead(RELAY_3_4) == HIGH) {
    motorState = M_OPENING;
  } else if (digitalRead(RELAY_1_2) == HIGH && digitalRead(RELAY_3_4) == LOW) {
    motorState = M_CLOSING;
  } else if (digitalRead(RELAY_1_2) == LOW && digitalRead(RELAY_3_4) == LOW) {
    Serial.println("FAULT. DEAD SHORT ON BATTERY. CHECK LOG AND FUSE");
    stopMotorApi();
    motorState = M_UNKNOWN;
    return;
  } else {
    motorState = M_STOPPED;
  }

  //if the motor is running, we will check the limits
  if (motorState == M_OPENING && digitalRead(T_LIM_SW) == S_SELECTED) {
    //stop the motor
    stopMotorApi();
  } else if (motorState == M_CLOSING && digitalRead(B_LIM_SW) == S_SELECTED) {
    stopMotorApi();
  }

  if (digitalRead(T_LIM_SW) == S_SELECTED && digitalRead(B_LIM_SW) == S_UNSELECTED) {
    doorState = D_OPENED;
  } else if (digitalRead(T_LIM_SW) == S_UNSELECTED && digitalRead(B_LIM_SW) == S_SELECTED) {
    doorState = D_CLOSED;
  } else {
    //anyting that's not fully open is "cracked"
    doorState = D_CRACKED;
  }

  doorState = D_OPENED; //debug
}


String processor(const String& var) {
  Serial.println(var);

  if (var == "WIFI_PASS") {
    return "12345";
  }
  if (var == "WIFI_SSID") {
    return "hotspot";
  }
  if (var == "TOD") {
    return timeClient.getFormattedTime();
  }
  if (var == "OTIME") {
    char openTime[10];
    sprintf(openTime, "%02d:%02dh", schedule.openHour, schedule.openMin);
    return String(openTime);
  }
  if (var == "CTIME") {
    char closeTime[10];
    sprintf(closeTime, "%02d:%02dh", schedule.closeHour, schedule.closeMin);
    return String(closeTime);
  }
  if (var == "STATE") {
    doorStateUpdate();
    switch (doorState) {
      case D_OPENED:
        return "Opened";
      case D_CLOSED:
        return "Closed";
      default:
        return "UNKNOWN";
    }
  }
  if (var == "TOFFSET") {
    return String(utcOffset);
  }

  return String();
}

/**
   add 1 or -1 to the offset.  Save it in prefs and update the timeClient.
*/
void updateTime(int i) {
  utcOffset += i;
  timeClient.setTimeOffset(SECONDS_PER_HOUR * utcOffset);
  prefs.putInt(pUtcOffset, utcOffset);
}
