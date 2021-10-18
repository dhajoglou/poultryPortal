#include <Adafruit_ILI9341.h>
#include <Adafruit_GFX.h>
#define DEFAULTSCREEN 1

//Strings
const String startMsg = "Starting Poultry Portal...";
const String connWifi = "Connecting to wifi...";
const String wifiConnError = "Could not connect to wifi!";
const String wifiConnSuccess = "Connected to wifi";
const String ldrMode = "Running on Light Sense";

//functions
void clearDisp(Adafruit_ILI9341& tft);
void printWifiStart(Adafruit_ILI9341& tft);
void printWifiFail(Adafruit_ILI9341& tft);
void printLdrMode(Adafruit_ILI9341& tft);
void printStartMsg(Adafruit_ILI9341& tft);
void printTft(Adafruit_ILI9341& tft, String msg, int x, int y, int color, int size,bool newline);
void printTftSimple(Adafruit_ILI9341& tft, String msg,bool newline);
void printTime(Adafruit_ILI9341& tft,String fTime);
int defaultScreen(Adafruit_ILI9341& tft, String bChg, String dPos, String dState, String fTime, String nextAction);
void updatDefTime(Adafruit_ILI9341& tft, String fTime);
void printPeep(Adafruit_ILI9341& tft);
