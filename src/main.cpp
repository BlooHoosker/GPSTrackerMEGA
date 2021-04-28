#include <Arduino.h>
#include <SoftwareSerial.h>
#include <BoardConfig.h>
#include <GPSTracker.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <LowPower.h>

//SoftwareSerial trackerSerial = SoftwareSerial(SIM_TX, SIM_RX);
GPSTracker tracker = GPSTracker(SIM_RST, SIM_PWR, SIM_DTR, RST_BTN, BATTERY_VPIN);

// Empty interrupt routine for button
void interruptRoutine(){}

// Disabling SPI, TWI, USART2, USART3 because they are not used
void lowPowerConfig(){
  power_spi_disable();
  power_twi_disable();
  power_usart2_disable();
  power_usart3_disable();
}

void setup() {
  Serial.begin(9600);

  // Serial.println("Testing...");
  // tracker.test();

  // Pinmode for built in LED

  Serial.println("SETUP: Arduino config");

  lowPowerConfig();  // Disabling unused CPU parts for lower power

  // Attaching interupt to reset button pin, used to wake up CPU from sleep mode
  attachInterrupt(digitalPinToInterrupt(RST_BTN), interruptRoutine, FALLING);

  // Serial needs to be initialized to be used
  Serial1.begin(SIM808_BAUDRATE);

  Serial.println("SETUP: Starting GSM..");

  // Starting GSM module
  // If GSM module fails to start, Arduino restarts after 4 seconds by watchdog
  if (!tracker.start(Serial1)) {
    Serial.println("SETUP: Start failed");

    wdt_enable(WDTO_4S); // Arduino restarts after 4 seconds
    tracker.builtInLedFastBlink(); // Fast blinking of built in LED
  }

  // Disabling built in LED
  tracker.builtInLedOff();

  Serial.println("SETUP: Start successful");
  Serial.println("=====================================================");
  Serial.println();
}

void loop() {

  Serial.println("Receiving...");

  // Entering idle sleep mode for 8 seconds, wakes up on UART communication
  LowPower.idle(SLEEP_8S, ADC_OFF, TIMER5_OFF, TIMER4_OFF, TIMER3_OFF, TIMER2_OFF, TIMER1_OFF, TIMER0_OFF, SPI_ON, USART3_ON, USART2_ON, USART1_ON, USART0_OFF, TWI_ON);
  
  // Receiving any data available on serial link
  tracker.receive();
  
  // Checking reset button status
  tracker.checkButton();

  // Checking battery percentage
  tracker.checkBatteryPercentage();

  // Checking GSM status
  tracker.checkGSM();

  tracker.printStatus();
  Serial.println("=====================================================");
  Serial.println();

  delay(100);
}

