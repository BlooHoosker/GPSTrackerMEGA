#include <Arduino.h>
#include <SoftwareSerial.h>
#include <BoardConfig.h>
#include <GPSTracker.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <LowPower.h>

//SoftwareSerial trackerSerial = SoftwareSerial(SIM_TX, SIM_RX);
GPSTracker tracker = GPSTracker(SIM_RST, SIM_PWR, SIM_DTR, RST_BTN, BATTERY_VPIN);

void interruptRoutine(){}

// Disabling SPI, TWI, USART2, USART3 because they are not used
void lowPowerConfig(){
  power_spi_disable();
  power_twi_disable();
  power_usart2_disable();
  power_usart3_disable();
}

void builtInLedOn(){
    digitalWrite(LED_BUILTIN, HIGH);
}

void builtInLedOff(){
    digitalWrite(LED_BUILTIN, LOW);
}

void fastBlink(){
  uint8_t state = 0;
  pinMode(LED_BUILTIN, OUTPUT);
  while (1){
    digitalWrite(LED_BUILTIN, state);
    delay(200);
    state = !state;
  }
}

void setup() {
  Serial.begin(9600);
  // Pinmode for built in LED
  pinMode(LED_BUILTIN, OUTPUT);
  builtInLedOn();

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
    fastBlink(); // Fast blinking of built in LED
  }

  // Disabling built in LED
  builtInLedOff();

  Serial.println("SETUP: Start successful");
  Serial.println("=====================================================");
  Serial.println();

  // Serial.println("Testing...");
  // tracker.test();
}

char trackerReceiveBuffer[TRACKER_BUFFER_SHORT];
void loop() {

  Serial.println("Receiving...");

  // Entering idle sleep mode for 8 seconds, wakes up on UART communication
  LowPower.idle(SLEEP_8S, ADC_OFF, TIMER5_OFF, TIMER4_OFF, TIMER3_OFF, TIMER2_OFF, TIMER1_OFF, TIMER0_OFF, SPI_ON, USART3_ON, USART2_ON, USART1_ON, USART0_OFF, TWI_ON);

  // Checking if UART has received any data
  if (tracker.receiveAT(trackerReceiveBuffer, TRACKER_BUFFER_SHORT, 100)){
    Serial.print(trackerReceiveBuffer);
    builtInLedOn();
    tracker.processAT(trackerReceiveBuffer);  
    builtInLedOff();
  }

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

