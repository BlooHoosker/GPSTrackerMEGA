#include <Arduino.h>
#include <SoftwareSerial.h>
#include <BoardConfig.h>
#include <GPSTracker.h>
#include <avr/wdt.h>

//SoftwareSerial trackerSerial = SoftwareSerial(SIM_TX, SIM_RX);
GPSTracker tracker = GPSTracker(SIM_RST, SIM_PWR);

void setup() {
  Serial.begin(9600);
  Serial.println("Starts and cums uwu");

  // Serial.println("Testing..");
  // tracker.test();

  // Start SoftwareSerial before starting tracker
  //trackerSerial.begin(SIM808_BAUDRATE);

  Serial1.begin(SIM808_BAUDRATE);

  Serial.println("SETUP: Starting..");
  if (!tracker.start(Serial1)) {
    Serial.println("SETUP: Start failed");
    wdt_enable(WDTO_4S); // Arduino restarts after 4 seconds
    while(1);
  }
  Serial.println("SETUP: Start successful");

}

char trackerReceiveBuffer[TRACKER_BUFFER_SIZE];
void loop() {

  Serial.println("Receiving...");

  if (tracker.receiveAT(trackerReceiveBuffer, TRACKER_BUFFER_SIZE, TRACKER_SECOND*10)){
    Serial.print(trackerReceiveBuffer);
    tracker.processAT(trackerReceiveBuffer);  
  }

  tracker.printStatus(); 
  
  Serial.println();
  delay(100);
}

