#include "DoorControl.h"


DoorControl::DoorControl(uint8_t r1_2, uint8_t r3_4, uint8_t b_lim, uint8_t t_lim, uint8_t cPin){
  doorState = DSTATE_CRACKED; //default presumtpin that the door is not actually closed
  motorState = MSTATE_UNKNOWN;
  relay1_2 = r1_2;
  relay3_4 = r3_4;
  b_lim_sw = b_lim;
  t_lim_sw = t_lim;
  Serial.println("Created door");
};
DoorControl::~DoorControl() {};
void DoorControl::open(){
  doorStateUpdate();
  if (doorState == DSTATE_OPENED) {
    return;
  }
  digitalWrite(relay1_2, HIGH);
  digitalWrite(relay3_4, LOW);
}
void DoorControl::close(){
  doorStateUpdate();
  if (doorState == DSTATE_CLOSED) {
    return;
  }
  digitalWrite(relay1_2, LOW);
  digitalWrite(relay3_4, HIGH);
}
void DoorControl::stopMotor(){
  digitalWrite(relay1_2, HIGH);
  digitalWrite(relay3_4, HIGH);
}
void DoorControl::doorStateUpdate()
{
  //first check the fault state if both switches are closed/disconnected
  if ( !(digitalRead(b_lim_sw) && digitalRead(t_lim_sw)) ) {
    stopMotor();
    Serial.println("FAULT. BOTH LIMIT SWITCHES ARE \"PRESSED\"");
    //return;
  }
  //next, check the motor state
  if ( digitalRead(relay1_2) == LOW && digitalRead(relay3_4) == HIGH) {
    motorState = MSTATE_OPENING;
  } else if (digitalRead(relay1_2) == HIGH && digitalRead(relay3_4) == LOW) {
    motorState = MSTATE_CLOSING;
  } else if (digitalRead(relay1_2) == LOW && digitalRead(relay3_4) == LOW) {
    Serial.println("FAULT. DEAD SHORT ON BATTERY. CHECK LOG AND FUSE");
    stopMotor();
    motorState = MSTATE_UNKNOWN;
    //return;
  } else {
    motorState = MSTATE_STOPPED;
  }

  //if the motor is running, we will check the limits
  if (motorState == MSTATE_OPENING && digitalRead(t_lim_sw) == S_SELECTED) {
    //stop the motor
    stopMotor();
  } else if (motorState == MSTATE_CLOSING && digitalRead(b_lim_sw) == S_SELECTED) {
    stopMotor();
  }

  if (digitalRead(t_lim_sw) == S_SELECTED && digitalRead(b_lim_sw) == S_UNSELECTED) {
    doorState = DSTATE_OPENED;
  } else if (digitalRead(t_lim_sw) == S_UNSELECTED && digitalRead(b_lim_sw) == S_SELECTED) {
    doorState = DSTATE_CLOSED;
  } else {
    //anyting that's not fully open is "cracked"
    doorState = DSTATE_CRACKED;
  }

  doorState = DSTATE_OPENED; //debug
}

int DoorControl::getDoorState(){
  return doorState;
}
