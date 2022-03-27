/* all of the main library references along with config stuff in this .h file */
#include "portalConfig.h"

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
DisplayModule disp = DisplayModule(TFT_CS, TFT_DC, -1);
/* Display variables */
String temperature = "";
String dPosition = "Closed";
String dState = "Idle";
char nextAction[100] = "Open at dawn";
String fTime = "Wifi Off";
String battCharge = "---";
long lastTempBattCheck = 0;

/* enc */
InputDebounce encoder_btn;
bool encBtnPressed = false;
Encoder enc(ENC_A, ENC_B);
volatile int16_t lastEncoderValue = 0;
volatile int16_t newEncoderValue = 0;
volatile int16_t realEncVal = 0;


unsigned long lastEncTurn = 0;
unsigned long thisEncTurn = 0;
volatile uint8_t CURRENTSCREEN = -1;
volatile uint8_t SCREEN = 0;

Preferences prefs;
timeTrigger schedule;

/*
   Door Control object
*/
DoorControl door = DoorControl(RELAY_1_2, RELAY_3_4, B_LIM_SW, T_LIM_SW, MOTOR_CURR);


void setup() {
  Serial.begin(115200);

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
  encoder_btn.registerCallbacks(encPressCallback, encReleaseCallback, NULL, NULL);
  encoder_btn.setup(ENC_BTN, BUTTON_DEBOUNCE_DELAY, InputDebounce::PIM_INT_PULL_UP_RES, 100);

  disp.clearDisp();
  disp.defaultScreen();
}

void checkEncoder()
{
  newEncoderValue = enc.read();
  if (newEncoderValue != lastEncoderValue) {
    Serial.print("last enc: "); Serial.print(lastEncoderValue); Serial.print(", new enc: "); Serial.println(newEncoderValue);
    lastEncoderValue = newEncoderValue;
    realEncVal = lastEncoderValue / 4;
  }
  SCREEN = abs(realEncVal % DisplayModule::SCREEN_CNT);
}
void encPressCallback(uint8_t pinIn) {

}

void encReleaseCallback(uint8_t pinIn) {

}

void loop() {
  /* check the state of the door */
  webUpdateApi();
  
  /*
     Not only do we only need to check the battery and temp infrequently,
     an AnalogRead right after we start kills the device. Might have somnething to do with
     the v1.0 power issues of the Thing Plus https://learn.sparkfun.com/tutorials/esp32-thing-plus-hookup-guide
  */
  if ((millis() - lastTempBattCheck) > TEMP_CYCLE) {
    battCharge = getBattV();
    temperature = getTemp();
    lastTempBattCheck = millis();
  }


  if (!LDRMODE) {
    fTime = timeClient.getFormattedTime();
    //if door is open, lookup close time. else, lookup open time:
    sprintf(nextAction, "Open at %02d:%02dh", schedule.openHour, schedule.openMin);
  }

  //check to see if we switched screens
  checkEncoder();
  encoder_btn.process(millis());

  // print overlay info
  if (!fTime.equals(prevTime)) {
    //Serial.println("t print");
    prevTime = fTime;
    disp.defaultOverlay(battCharge, fTime, "", "" , "" , temperature);
  }

  switch (SCREEN) {
    case DisplayModule::DEFAULT_SCREEN:
      if (SCREEN != CURRENTSCREEN) {
        disp.printPeep(DisplayModule::DEFAULT_SCREEN);
      }

      CURRENTSCREEN = SCREEN;
      break;
    case DisplayModule::DOOR_OPEN_SCREEN:
      if (SCREEN != CURRENTSCREEN) {
        disp.printPeep(DisplayModule::DOOR_OPEN_SCREEN);
        CURRENTSCREEN = DisplayModule::DOOR_OPEN_SCREEN;
      }
      break;
    case DisplayModule::DOOR_CLOSE_SCREEN:
      if (SCREEN != CURRENTSCREEN) {
        disp.printPeep(DisplayModule::DOOR_CLOSE_SCREEN);
        CURRENTSCREEN = DisplayModule::DOOR_CLOSE_SCREEN;
      }
      break;
    default:
      break;

  }
  delay(300);
}

volatile boolean stopWifiConn = false;
void IRAM_ATTR wifiSetupTimerHandler0(void)
{
  stopWifiConn = true;
  //return true;
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
      disp.printTft(wifiMsgs, -1, 0, ILI9341_GREEN, ILI9341_BLACK, 2, true);
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
  //pinMode(BATT12V, INPUT);
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
  pinMode(ENC_BTN, INPUT);
}

String getTemp() {
  float aVal = analogRead(TEMP) / 1023.0;
  float tmpC = (aVal - 0.5) * 100;
  float tmpF = (tmpC * 1.8) + 32;
  return String(tmpF) + "F";
}
/**
   With voltage divider
   Bat -> 55k -> pin -> 15K -> ground
   15v should drop down to 3.2v though the divier.
   12v batteries should not go above 14.4 volts
   measurements:
   5.4v ~ 1255
   10v ~ 2450 (/245 for scale)
   12v ~ 2960
   15v ~ 4095
   ESP32 ADC graph for reference https://randomnerdtutorials.com/esp32-adc-analog-read-arduino-ide/
*/
String getBattV() {
  char t[5];
  float aVal = analogRead(A3) / 245.0;
  dtostrf(aVal, 3, 1, t);
  return String(t) + "v";
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
}
void closeDoorApi() {
}
void openDoorApi() {
}

void webUpdateApi() {
  door.doorStateUpdate();
  switch (door.getDoorState()) {
    case DoorControl::DSTATE_OPENED:
      break;
    case DoorControl::DSTATE_CLOSED:
      break;
  }
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
    webUpdateApi();
    switch (door.getDoorState()) {
      case DoorControl::DSTATE_OPENED:
        return "Opened";
      case DoorControl::DSTATE_CLOSED:
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
