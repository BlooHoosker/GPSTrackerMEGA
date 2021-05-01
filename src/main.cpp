#include <Arduino.h>
#include <SoftwareSerial.h>
#include <BoardConfig.h>
#include <GPSTracker.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <LowPower.h>

//SoftwareSerial trackerSerial = SoftwareSerial(SIM_TX, SIM_RX);
GPSTracker tracker = GPSTracker(SIM_RST, SIM_PWR, /*SIM_DTR,*/ RST_BTN, BATTERY_VPIN);

// Empty interrupt routine for button
void interruptRoutine(){}

// Disabling SPI, TWI, USART2, USART3 because they are not used
void lowPowerConfig(){
  power_spi_disable();
  power_twi_disable();
  power_usart2_disable();
  power_usart3_disable();
  USART0_DISABLE
}

void setup() {
  Serial.begin(9600);

  // DEBUG_PRINTLN("Testing...");
  // tracker.test();

  // Pinmode for built in LED

  DEBUG_PRINTLN("SETUP: Arduino config");

  lowPowerConfig();  // Disabling unused CPU parts for lower power

  // Attaching interupt to reset button pin, used to wake up CPU from sleep mode
  attachInterrupt(digitalPinToInterrupt(RST_BTN), interruptRoutine, FALLING);

  // Serial needs to be initialized to be used
  Serial1.begin(SIM808_BAUDRATE);

  DEBUG_PRINTLN("SETUP: Starting GSM..");

  // Starting GSM module
  // If GSM module fails to start, Arduino restarts after 4 seconds by watchdog
  if (!tracker.start(Serial1)) {
    DEBUG_PRINTLN("SETUP: Start failed");

    wdt_enable(WDTO_4S); // Arduino restarts after 4 seconds
    tracker.builtInLedFastBlink(); // Fast blinking of built in LED
  }

  // Disabling built in LED
  tracker.builtInLedOff();

  DEBUG_PRINTLN("SETUP: Start successful");
  DEBUG_PRINTLN("=====================================================");
  DEBUG_PRINTLN();
}

void loop() {

  DEBUG_PRINTLN("Receiving...");

  // Entering idle sleep mode for 8 seconds, wakes up on UART communication
  for (uint8_t i = 0; i < 4; i++){
    LowPower.idle(SLEEP_8S, 
                  ADC_OFF, 
                  TIMER5_OFF, 
                  TIMER4_OFF, 
                  TIMER3_OFF, 
                  TIMER2_OFF, 
                  TIMER1_OFF, 
                  TIMER0_OFF, 
                  SPI_ON, 
                  USART3_ON, 
                  USART2_ON, 
                  USART1_ON, 
                  USART0_OFF, // change for final release, so it stays off
                  TWI_ON);
      
    // Checking reset button status
    tracker.checkButton();
  }
  
  // Receiving any data available on serial link
  tracker.receive();

  // Update GPS location if GPS is active
  tracker.updateGPSLocation();

  // Checking battery percentage
  tracker.checkBatteryPercentage();

  // Checking GSM status
  tracker.checkGSM();

  PRINTSTATUS
  DEBUG_PRINTLN("=====================================================");
  DEBUG_PRINTLN();
}

