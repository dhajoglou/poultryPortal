/**
   Collect all of the door controls/status here into one model
*/

#ifndef DoorControl_h
#define DoorControl_h

#include "Arduino.h"

class DoorControl
{
  public:
    DoorControl(uint8_t relay1_2, uint8_t relay3_4, uint8_t b_lim, uint8_t t_lim, uint8_t cPin);
    ~DoorControl();
    void open();
    void close();
    void stopMotor();
    void doorStateUpdate();
    int getDoorState();
    const static int DSTATE_CLOSED = 0;
    const static int DSTATE_OPENED = 1;
    const static int DSTATE_CRACKED = 3;
    const static int MSTATE_CLOSING = 4;
    const static int MSTATE_OPENING = 5;
    const static int MSTATE_STOPPED = 6;
    const static int MSTATE_UNKNOWN = 7;

  private:
    int doorState;
    int motorState;
    uint8_t relay1_2;
    uint8_t relay3_4;
    uint8_t b_lim_sw;
    uint8_t t_lim_sw;
    uint8_t curr_pin;
    const uint8_t S_SELECTED = HIGH;
    const uint8_t S_UNSELECTED = LOW;
};
#endif
