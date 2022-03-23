/*
   Class for DisplayModule
*/
#include "DisplayModule.h"
#define SERIAL_DEBUG 1

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
  printTft(startMsg, 0, 0, ILI9341_GREEN, 3, true);
}
void DisplayModule::printWifiStart() {
  clearDisp();
  printTft(connWifi, 0, 0, ILI9341_YELLOW, 2, true);
}
void DisplayModule::printWifiFail() {
  printTft( wifiConnError, 0, 0, ILI9341_RED, 2, true);
}
void DisplayModule::printLdrMode() {
  clearDisp();
  printTft(ldrMode, 0, 0, ILI9341_RED, 3, true);
}
void DisplayModule::printTft(String msg, int x, int y, int color, int size, bool newline) {
  tft.setTextColor(color);
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
  printTft(fTime, -1, 0, ILI9341_BLUE, 2, true);
}
void DisplayModule::printTftSimple(String  msg, bool newline) {
  printTft(msg, -1, -1, ILI9341_YELLOW, 2, newline);
}
int DisplayModule::defaultScreen( String bChg, String dPos, String dState, String fTime, String nextAction) {
  //clearDisp();
  tft.fillRoundRect(0, 0, 320, 50, 5, ILI9341_YELLOW);
  tft.drawRoundRect(0, 0, 320, 50, 5, ILI9341_DARKCYAN);
  String header = "  Hentry Status\n    " + bChg;
  String timeRow = "Current Time: " + fTime + "\n";
  String positionRow = "Door position: " + dPos + "\n";
  String stateRow = "Door state: " + dState + "\n";
  String nextRow = "Next: " + nextAction + "\n";
  printTft(header,  0,  2, ILI9341_DARKCYAN, 3, true);
  printTft(timeRow, 0, 55, ILI9341_GREEN, 2, true);
  printTft(positionRow, -1, -0, ILI9341_GREEN, 2, true);
  printTft(stateRow, -1, -0, ILI9341_GREEN, 2, true);
  printTft(nextRow, -1, -0, ILI9341_GREEN, 2, true);
  printPeep();
  return DEFAULT_SCREEN;
}
void DisplayModule::updatDefTime(String fTime) {}

void DisplayModule::printPeep() {
  tft.fillRoundRect(0, 175, 320, 65, 5, ILI9341_BLACK);
  tft.drawRoundRect(0, 175, 320, 65, 5, ILI9341_YELLOW);
  tft.setTextColor(ILI9341_ORANGE);
  tft.setTextSize(1);

  tft.setCursor(0, 180);
  tft.print(peepBuffer[peepBufferIndex]); tft.println("     __/\/ ");
  tft.print(peepBuffer[peepBufferIndex]); tft.println("    /.__.\\ ");
  tft.print(peepBuffer[peepBufferIndex]); tft.println("    \\ \\/ / ");
  tft.print(peepBuffer[peepBufferIndex]); tft.println(" `__/    \\ ");
  tft.print(peepBuffer[peepBufferIndex]); tft.println("  \\-      ) ");
  tft.print(peepBuffer[peepBufferIndex]); tft.println("   \\_____/ ");
  tft.print(peepBuffer[peepBufferIndex]); tft.println("_____|_|____ ");
  tft.print(peepBuffer[peepBufferIndex]); tft.println("     \" \" ");
}

void DisplayModule::setPeepBuffer(int i) {
  peepBufferIndex = abs(i%4);
}
