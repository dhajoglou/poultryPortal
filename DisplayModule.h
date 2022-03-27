/*
   Library fpr the TFT screen
*/
#ifndef DisplayModule_h
#define DisplayModule_h

//#include "Arduino.h"
#include <Adafruit_ILI9341.h>
#include <Adafruit_GFX.h>

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
    void printTft(String msg, int x, int y, int fgColor, int bgColor, int size, bool newline);
    void printTime(String fTime);
    void printTftSimple(String  msg, bool newline);
    int defaultScreen();
    void updatDefTime(String fTime);
    void printPeep(int p);
    void setPeepBuffer(int i);
    void startDisp();
    void defaultOverlay(String charge, String fTime, String pos, String state, String sch, String tmp);
    void showOpen();
    const static int DEFAULT_SCREEN = 0;
    const static int DOOR_OPEN_SCREEN = 1;
    const static int DOOR_CLOSE_SCREEN = 2;
    const static int SCREEN_CNT = 3;
  private:
    Adafruit_ILI9341 tft;
    int peepBufferIndex;
    const String peepBuffer[4] = {"","    ","        ","            "};
    const String startMsg = "Starting Poultry Portal...";
    const String connWifi = "Connecting to wifi...";
    const String wifiConnError = "Could not connect to wifi!";
    const String wifiConnSuccess = "Connected to wifi";
    const String ldrMode = "Running on Light Sense";
    const String sClose = "CLOSE?";
    const String sOpen = "OPEN?";
    const String tempRow = "Temperature:";
};

#endif
