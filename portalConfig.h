/**
 * General configuration stuff.
 *
*/
#include "DisplayModule.h"
#include "DoorControl.h"
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Preferences.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <InputDebounce.h>
#include <Encoder.h>


#define TIMER_INTERRUPT_DEBUG       1
#include "ESP32TimerInterrupt.h"
#define WIFI_TIMER_TIMEOUT_MS       5000 /* 5 seconds */
#define NTP_UPDATE_INTERVAL         86400000L
#define WIFI_RETRY                   3
#define DEBUG_NTPClient             1
#define ENCODER_STEPS_PER_NOTCH     1
#define BUTTON_DEBOUNCE_DELAY       20 /* delay for debouncing encoder push */
#define SECONDS_PER_HOUR            3600
//#define TEMP_CYCLE                  300000 /*check ever 5 min*/
#define TEMP_CYCLE                  10000 
/*
 * Preferences keys and constants
 */
const char* pWifiSsid = "wifi-ssid";
const char* pWifiPass = "wifi-password";
const char* pUtcOffset = "utcOffset";
const char* pLastBoot = "lastBoot";
const char* ntpPool = "pool.ntp.org";

/**
   door/motor states
*/
#define D_CLOSED      0
#define D_OPENED      2
#define D_CRACKED     6
#define M_UNKNOWN     7

/*
 * These pins are the default for the Thing Plus PCB layout.
 * These are basically in order on the module
 * Starting with A0 (on the 16 pin row) and running down the module towards the ufl connector
 * 
 * The next series starts on the 12 pin row at GPIO 13
*/
/* The first three are analog inputs, 4 and 5 are digital inputs */
#define B_LIM_SW    A0
#define T_LIM_SW    A1
#define TEMP        A2
#define BATT12V     A3
#define MOTOR_CURR  A4
#define LDR         A5 //see note just below
/* analogRead on A5 does not function if WiFi is configured.  This makes it a good candidate
   for LDR mode as a backup when wifi isn't reachable. 
   If you want to run LDR and wifi then you have to pick a different pin for
   the LDR sensor
*/

/* The next series of pins are the SPI bus for the display. */
#define TFT_CLK   5
#define TFT_MOSI  18
#define TFT_MISO  19
#define TFT_DC    17
#define TFT_CS    16
/* last pin on the long side */
#define ENC_BTN   21

/* Start of 12 pin row */
#define ENC_A     13 
#define ENC_B     12
#define RELAY_1_2 27
#define RELAY_3_4 33
#define RELAY_5   15
#define RELAY_6   32
#define RELAY_7   14
#define UNUSED_SCL  22 //i2c bus
#define UNUSED_SDA  23 //i2c bus

/*
 * The open/close times is held in this struct
 * It's easy to simply define and hour/minute combo for each time
 */
typedef struct
{
  char openHour;
  char openMin;
  char closeHour;
  char closeMin;
} timeTrigger;
#define OSCH 1
#define CSCH 2


 
