#include <Arduino.h>
#include <SoftwareSerial.h>
#include <BoardConfig.h>
#include <GPSTracker.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <LowPower.h>

//SoftwareSerial trackerSerial = SoftwareSerial(SIM_TX, SIM_RX);
GPSTracker tracker = GPSTracker(SIM_RST, SIM_PWR);

// Disabling SPI, TWI, USART2, USART3 because they are not used
void lowPowerConfig(){
  power_spi_disable();
  power_twi_disable();
  power_usart2_disable();
  power_usart3_disable();
}

void setup() {
  Serial.begin(9600);

  Serial.println("SETUP: Low power config");
  lowPowerConfig();

  //tracker.test();
  
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

  // Entering idle sleep mode for 8 seconds 
  LowPower.idle(SLEEP_8S, ADC_OFF, TIMER5_OFF, TIMER4_OFF, TIMER3_OFF, TIMER2_OFF, TIMER1_OFF, TIMER0_OFF, SPI_ON, USART3_ON, USART2_ON, USART1_ON, USART0_OFF, TWI_ON);

  if (tracker.receiveAT(trackerReceiveBuffer, TRACKER_BUFFER_SIZE, 100)){
    Serial.print(trackerReceiveBuffer);
    tracker.processAT(trackerReceiveBuffer);  
  }

  tracker.printStatus(); 
  Serial.println();
  delay(100);
}

