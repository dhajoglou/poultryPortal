/*
   Class for DisplayModule
*/
#include "DisplayModule.h"
#define SERIAL_DEBUG 0

DisplayModule::DisplayModule(int8_t csPin, int8_t dcPin, int8_t rstPin)
  : tft(csPin, dcPin, rstPin)
{
  peepBufferIndex = 0;
};

DisplayModule::~DisplayModule() {};

void DisplayModule::startDisp() {
  tft.begin();
  tft.fillScreen(ILI9341_BLACK);
  tft.setRotation(3);
}
void DisplayModule::PrintMainScreen() {}

void DisplayModule::clearDisp() {
  tft.fillScreen(ILI9341_BLACK);
}
void DisplayModule::printStartMsg() {
  printTft(startMsg, 0, 0, ILI9341_GREEN, ILI9341_BLACK, 3, true);
}
void DisplayModule::printWifiStart() {
  clearDisp();
  printTft(connWifi, 0, 0, ILI9341_YELLOW, ILI9341_BLACK, 2, true);
}
void DisplayModule::printWifiFail() {
  printTft( wifiConnError, 0, 0, ILI9341_RED, ILI9341_BLACK, 2, true);
}
void DisplayModule::printLdrMode() {
  clearDisp();
  printTft(ldrMode, 0, 0, ILI9341_RED, ILI9341_BLACK, 3, true);
}
void DisplayModule::printTft(String msg, int x, int y, int fgColor, int bgColor, int size, bool newline) {
  tft.setTextColor(fgColor, bgColor);
  tft.setTextSize(size);
  if (x > -1) {
    tft.setCursor(x, y);
  }
  if (newline) {
    tft.println(msg);
#if SERIAL_DEBUG
    Serial.println(msg);
#endif
  } else {
    tft.print(msg);
#if SERIAL_DEBUG
    Serial.print(msg);
#endif
  }
}
void DisplayModule::printTime(String fTime) {
  printTft(fTime, -1, 0, ILI9341_BLUE, ILI9341_BLACK, 2, true);
}
void DisplayModule::printTftSimple(String  msg, bool newline) {
  printTft(msg, -1, -1, ILI9341_YELLOW, ILI9341_BLACK, 2, newline);
}
int DisplayModule::defaultScreen() {
  //clearDisp();
  tft.fillRoundRect(0, 0, 320, 52, 5, ILI9341_YELLOW);
  tft.drawRoundRect(0, 0, 320, 52, 5, ILI9341_DARKCYAN);
  String header = "  Hentry Status";
  String timeRow = "Current Time:";
  String positionRow = "Door position:";
  String stateRow = "Door state:";
  String nextRow = "Next:";
  printTft(header,  0,  2, ILI9341_DARKCYAN, ILI9341_YELLOW ,3, true);
  printTft(timeRow, 0, 60, ILI9341_GREEN, ILI9341_BLACK, 2, true);
  printTft(positionRow, 0, 83, ILI9341_GREEN, ILI9341_BLACK, 2, true);
  printTft(stateRow, 0, 106, ILI9341_GREEN, ILI9341_BLACK, 2, true);
  printTft(nextRow, 0, 129, ILI9341_GREEN, ILI9341_BLACK, 2, true);
  printTft(tempRow, 0, 152, ILI9341_GREEN, ILI9341_BLACK, 2, true);
  return DEFAULT_SCREEN;
}
void DisplayModule::defaultOverlay(String charge, String fTime, String pos, String state, String sch, String tmp) {
  printTft(charge,110, 26, ILI9341_DARKCYAN, ILI9341_YELLOW ,3, true);
  printTft(fTime, 155, 55, ILI9341_GREEN, ILI9341_BLACK, 3, false);
  printTft(tmp, 154, 152, ILI9341_GREEN, ILI9341_BLACK, 2,  true);
}
void DisplayModule::updatDefTime(String fTime) {}

void DisplayModule::printPeep(int p) {
  tft.fillRoundRect(0, 175, 320, 65, 5, ILI9341_BLACK);
  tft.drawRoundRect(0, 175, 320, 65, 5, ILI9341_YELLOW);
  tft.setTextColor(ILI9341_ORANGE);
  tft.setTextSize(1);
  peepBufferIndex = p;
  tft.setCursor(0, 180);
  tft.print(peepBuffer[peepBufferIndex]); tft.println("     __/\/ ");
  tft.print(peepBuffer[peepBufferIndex]); tft.println("    /.__.\\ ");
  tft.print(peepBuffer[peepBufferIndex]); tft.println("    \\ \\/ / ");
  tft.print(peepBuffer[peepBufferIndex]); tft.println(" `__/    \\ ");
  tft.print(peepBuffer[peepBufferIndex]); tft.println("  \\-      ) ");
  tft.print(peepBuffer[peepBufferIndex]); tft.println("   \\_____/ ");
  tft.print(peepBuffer[peepBufferIndex]); tft.println("_____|_|____ ");
  tft.print(peepBuffer[peepBufferIndex]); tft.println("     \" \" ");

  /*
   * Based on the screen, disp the function
   */
   tft.setTextSize(4);
   switch(p){
    case DOOR_OPEN_SCREEN:
      printTft(sOpen, 100, 194, ILI9341_RED, ILI9341_BLACK, 4, false);
      break;
     case DOOR_CLOSE_SCREEN:
      printTft(sClose, 124, 194, ILI9341_RED, ILI9341_BLACK, 4, false);
      break;
   }
}

void DisplayModule::setPeepBuffer(int i) {
  peepBufferIndex = abs(i % 4);
}
