#include <GPSTracker.h>

void GPSTracker::builtInLedOn(){
    digitalWrite(LED_BUILTIN, HIGH);
}

void GPSTracker::builtInLedOff(){
    digitalWrite(LED_BUILTIN, LOW);
}

void GPSTracker::builtInLedFastBlink(){
  uint8_t state = 0;
  pinMode(LED_BUILTIN, OUTPUT);
  while (1){
    digitalWrite(LED_BUILTIN, state);
    delay(200);
    state = !state;
  }
}
