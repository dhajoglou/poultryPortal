#include "displayFunctions.h"
#define SERIAL_DEBUG 1
void clearDisp(Adafruit_ILI9341& tft) {
  tft.fillScreen(ILI9341_BLACK);
}
void printStartMsg(Adafruit_ILI9341& tft) {
  printTft(tft, startMsg, 0, 0, ILI9341_GREEN, 3, true);
}

void printWifiStart(Adafruit_ILI9341& tft) {
  clearDisp(tft);
  printTft(tft, connWifi, 0, 0, ILI9341_YELLOW, 2, true);
}
void printWifiFail(Adafruit_ILI9341& tft) {
  printTft(tft, wifiConnError, 0, 0, ILI9341_RED, 2, true);
}
void printLdrMode(Adafruit_ILI9341& tft) {
  clearDisp(tft);
  printTft(tft, ldrMode, 0, 0, ILI9341_RED, 3, true);
}

void printTft(Adafruit_ILI9341& tft, String msg, int x, int y, int color, int size, bool newline) {
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

void printTime(Adafruit_ILI9341& tft, String fTime) {
  printTft(tft, fTime, -1, 0, ILI9341_BLUE, 2, true);
}

void printTftSimple(Adafruit_ILI9341& tft, String  msg, bool newline) {
  printTft(tft, msg, -1, -1, ILI9341_YELLOW, 2, newline);
}


int defaultScreen(Adafruit_ILI9341& tft, String bChg, String dPos, String dState, String fTime, String nextAction) {
  //clearDisp(tft);
  tft.fillRoundRect(0, 0, 320, 50, 5, ILI9341_YELLOW);
  tft.drawRoundRect(0, 0, 320, 50, 5, ILI9341_DARKCYAN);
  String header = "  Hentry Status\n    " + bChg;
  String timeRow = "Current Time: " + fTime + "\n";
  String positionRow = "Door position: " + dPos + "\n";
  String stateRow = "Door state: " + dState + "\n";
  String nextRow = "Next: " + nextAction + "\n";
  printTft(tft, header,  0,  2, ILI9341_DARKCYAN, 3, true);
  printTft(tft, timeRow, 0, 55, ILI9341_GREEN, 2, true);
  printTft(tft, positionRow, -1, -0, ILI9341_GREEN, 2, true);
  printTft(tft, stateRow, -1, -0, ILI9341_GREEN, 2, true);
  printTft(tft, nextRow, -1, -0, ILI9341_GREEN, 2, true);
  printPeep(tft);
  return DEFAULT_SCREEN;
}

void updatDefTime(Adafruit_ILI9341& tft, String fTime) {
  String timeRow = fTime + "\n";
  tft.fillRoundRect(160, 52, 160, 22, 0, ILI9341_BLACK);
  printTft(tft, timeRow, 160, 52, ILI9341_DARKCYAN, 3, true);
}

void printPeep(Adafruit_ILI9341& tft) {
  tft.fillRoundRect(0, 175, 320, 65, 5, ILI9341_BLACK);
  tft.drawRoundRect(0, 175, 320, 65, 5, ILI9341_YELLOW);
  tft.setTextColor(ILI9341_ORANGE);
  tft.setTextSize(1);
  tft.setCursor(0, 180);
  tft.println("               __/\/ ");
  tft.println("              /.__.\\ ");
  tft.println("              \\ \\/ / ");
  tft.println("           `__/    \\ ");
  tft.println("            \\-      ) ");
  tft.println("             \\_____/ ");
  tft.println("          _____|_|____ ");
  tft.println("               \" \" ");



}
