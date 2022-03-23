/*
   Library fpr the TFT screen
*/
#ifndef DisplayModule_h
#define DisplayModule_h

#include "Arduino.h"
#include <Adafruit_ILI9341.h>
#include <Adafruit_GFX.h>

/*
 * TFT Screens 
 */
#define DEFAULT_SCREEN 0
#define CONTROL_DOOR_SCREEN 1
#define WIFI_UPDATE_SCREEN 2

class DisplayModule
{
  public:
    DisplayModule(int8_t csPin, int8_t dcPin, int8_t rstPin);
    ~DisplayModule();
    void PrintMainScreen();
    void clearDisp();
    void printStartMsg();
    void printWifiStart();
    void printWifiFail();
    void printLdrMode();
    void printTft(String msg, int x, int y, int color, int size, bool newline);
    void printTime(String fTime);
    void printTftSimple(String  msg, bool newline);
    int defaultScreen( String bChg, String dPos, String dState, String fTime, String nextAction);
    void updatDefTime(String fTime);
    void printPeep();
    void setPeepBuffer(int i);
    void startDisp();
  private:
    Adafruit_ILI9341 tft;
    int peepBufferIndex;
    const String peepBuffer[4] = {"","    ","        ","            "};
    const String startMsg = "Starting Poultry Portal...";
    const String connWifi = "Connecting to wifi...";
    const String wifiConnError = "Could not connect to wifi!";
    const String wifiConnSuccess = "Connected to wifi";
    const String ldrMode = "Running on Light Sense";
};

#endif
