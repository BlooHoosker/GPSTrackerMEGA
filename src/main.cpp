#include <Arduino.h>
#include <SoftwareSerial.h>
#include <BoardConfig.h>
#include <GPSTracker.h>

SoftwareSerial trackerSerial = SoftwareSerial(SIM_TX, SIM_RX);
GPSTracker tracker = GPSTracker(SIM_RST, SIM_PWR);

void setup() {
  Serial.begin(9600);
  Serial.println("*Starts and cums* uwu");

  // Serial.println("Testing..");
  // tracker.test();
  // while(1);

  // Start SoftwareSerial before starting tracker
  trackerSerial.begin(SIM808_BAUDRATE);

  Serial.println("Starting..");
  if (!tracker.start(trackerSerial)) {
    Serial.println("Start failed");
    while(1);
  }
  Serial.println("Start successful");

}

char trackerReceiveBuffer[TRACKER_BUFFER_SIZE];
void loop() {

  Serial.println("Receiving...");

  if (tracker.receiveAT(trackerReceiveBuffer, TRACKER_BUFFER_SIZE, TRACKER_DEFAULT_TIMEOUT)){
    Serial.print(trackerReceiveBuffer);
    tracker.processAT(trackerReceiveBuffer);  
  }

  tracker.printStatus(); 
  
  Serial.println();
  delay(100);
}

