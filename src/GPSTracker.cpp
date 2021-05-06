#include <GPSTracker.h>
#include <EEPROM.h>
#include <avr/wdt.h>

GPSTracker::GPSTracker(uint8_t SIM_RESET_PIN, uint8_t SIM_PWR_PIN,/* uint8_t SIM_DTR_PIN,*/ uint8_t RST_BTN_PIN, uint8_t BATTERY_PIN){

	// Enabling built in LED
	pinMode(LED_BUILTIN, OUTPUT);
	builtInLedOn();

    _resetPin = SIM_RESET_PIN;
	pinMode(_resetPin, OUTPUT);
    digitalWrite(_resetPin, HIGH);

    _powerPin = SIM_PWR_PIN;
    pinMode(_powerPin, OUTPUT);
    digitalWrite(_powerPin, LOW);

	// _dtrPin = SIM_DTR_PIN;
	// pinMode(_dtrPin, OUTPUT);
    // digitalWrite(_dtrPin, LOW);

	_buttonPin = RST_BTN_PIN;
	pinMode(_buttonPin, INPUT_PULLUP);

	_batteryPin = BATTERY_PIN;

	memset(_phoneNumber, 0, TRACKER_BUFFER_SHORT);
	memset(_latitude, 0, TRACKER_BUFFER_SHORT);
	memset(_longitude, 0, TRACKER_BUFFER_SHORT);
	memset(_date, 0, TRACKER_BUFFER_DATE);
	memset(_time, 0, TRACKER_BUFFER_TIME);

	_gpsPowerStatus = 0;
	_gpsFixStatus = 0;
	_masterNumberSet = 0;
	_mapLinkSrc = 0;
	_batteryPercentage = 0;
}

GPSTracker::~GPSTracker(){}

bool GPSTracker::updateStatus(){
	wdt_reset();
	_batteryPercentage = getBatteryPercentage();
	return updateGPSStatusInfo();
}

void GPSTracker::printStatus(){
	wdt_reset();

	updateGPSStatusInfo();
	DEBUG_PRINT("Latitude: ");
	DEBUG_PRINTLN(_latitude);
	DEBUG_PRINT("Longitude: ");
	DEBUG_PRINTLN(_longitude);
	DEBUG_PRINT("Timestamp: ");
	DEBUG_PRINT(_date);
	DEBUG_PRINT(" ");
	DEBUG_PRINTLN(_time);
	DEBUG_PRINT("Power status: ");
	DEBUG_PRINTLN(_gpsPowerStatus);
	DEBUG_PRINT("Fix status: ");
	DEBUG_PRINTLN(_gpsFixStatus);
	DEBUG_PRINT("Master set: ");
	DEBUG_PRINTLN(_masterNumberSet);
	DEBUG_PRINT("Link source: ");
	DEBUG_PRINTLN(_mapLinkSrc);
	DEBUG_PRINT("Battery: ");
	DEBUG_PRINTLN(_batteryPercentage);
}

void GPSTracker::resetEEPROM(){
	wdt_reset();

	for (uint8_t i = 0; i < 6+TRACKER_BUFFER_SHORT; i++){
		EEPROM.update(i, 0);
	}
}

void GPSTracker::restart(){
	wdt_reset();

	// Check if master number is set
	if (!_masterNumberSet){
		// If its not set, save phone number to memory to send reply after restart
		setMasterNumber(_phoneNumber);
		resetMasterNumber();
	}
	EEPROM[RESTART_ADDR] = 0xFF;
	delay(10);
	wdt_enable(WDTO_120MS);
	while(1);
}

bool GPSTracker::start(Stream &serial){
	wdt_reset();

    _serialPort = &serial;

    DEBUG_PRINTLN("START: Powering up..");
    
    // Power on module
    bool powerStatus = powerOn();

    if (!powerStatus){
        DEBUG_PRINTLN("START: Failed to power up");
        return false;
    }

    DEBUG_PRINTLN("START: Initializing..");
    
    // Initialize module
    if (!init()) {
        DEBUG_PRINTLN("START: Initialization Failed");
        return false;
    }

	// Check restart flag in EEPROM indicating device powered up after user requested restart
	if (EEPROM[RESTART_ADDR] == 0xFF){
		// If master wasn't set before restart
		if (!_masterNumberSet){
			// Temporarily setting master number flag so getMasterNumber can be used
			EEPROM[MASTERSET_ADDR] = 0xFF;
			getMasterNumber();
			resetMasterNumber();
		}
		DEBUG_PRINTLN("RESTART: Sending reply");
		sendSMS("RESTART COMPLETE", _phoneNumber);
		DEBUG_PRINTLN("START: Restart complete");
		DEBUG_PRINTLN(_phoneNumber);
	}
	
	// Resetting restart flag
	EEPROM.update(RESTART_ADDR, 0);
    return true;
}

bool GPSTracker::init(){
	wdt_reset();

	// Reset
	// Delay
	// Send AT
	// Wait for AT
	// Wait for SMS ready
	// Disable ECHO
	// Set SMS text mode
	// Set SMS storage mode to module itself 
	// Delete all stored SMS in module

	bool received = false;

	// Reset the module to start clean initialization
	reset();

	// Load master number from EEPROM
	getMasterNumber();
	getMapLinkSrc();

	// Send "AT" and wait for "AT" response
	// If "AT" response is received, reset is complete
	// 4 attempts in total
	for (uint8_t i = 4; i > 0; i--){
		sendAT();
		received = waitFor("AT");
		if (received) break;

		delay(300);
	}

	// If module fails to respond with "AT", initialization failed
	if (!received){
		return false;
	}

	// Wait until we get SMS Ready which means the module is ready to send/receive SMS
	// Timeout 5 seconds
	if (!waitFor("SMS Ready", 5*TRACKER_SECOND)){
		return false;
	}

	// Disable echoing of sent commands
	if (!setEchoMode(false)){
		DEBUG_PRINTLN("INIT: Echo not disabled");
        return false;
	} 

	// Set SMS message format to "Text"
	if (!setSMSMessageMode(true)) {
		DEBUG_PRINTLN("INIT: Failed to set SMS text mode");
		return false;
	}

	// Set storage for SMS to module itself
	if (!setSmsStorage()){
		DEBUG_PRINTLN("INIT: Failed to set storage");
		return false;
	}

	// Deleting all messages from module to create space
	if (!deleteAllSMS()){
		DEBUG_PRINTLN("INIT: Failed to receive OK delsms");
		return false;
	}

	// if (!enableGsmSleepMode()){
	// 	DEBUG_PRINTLN("INIT: Failed to receive OK sleep mode");
	// 	return false;
	// }

	return true;
}

void GPSTracker::receive(){
	wdt_reset();

	char receiveBuffer[TRACKER_BUFFER_SHORT];

	if (waitFor(receiveBuffer, TRACKER_BUFFER_SHORT, 100, "+CMTI")){
		//tracker.gsmWake();
		DEBUG_PRINT(receiveBuffer);
		builtInLedOn();
		processAT(receiveBuffer);  
		builtInLedOff();
		//tracker.gsmSleep();
  	}

	while(queueExtract(receiveBuffer, TRACKER_BUFFER_SHORT)){
		//tracker.gsmWake();
		DEBUG_PRINT(receiveBuffer);
		builtInLedOn();
		processAT(receiveBuffer);  
		builtInLedOff();
		//tracker.gsmSleep();
	}

	// if (receiveAT(receiveBuffer, TRACKER_BUFFER_SHORT, 100)){
	// 	//tracker.gsmWake();
	// 	DEBUG_PRINT(receiveBuffer);
	// 	builtInLedOn();
	// 	processAT(receiveBuffer);  
	// 	builtInLedOff();
	// 	//tracker.gsmSleep();
  	// }
}

void GPSTracker::checkButton(){
	//wdt_reset();

	DEBUG_PRINTLN("BUTTON: Check");

	// 3 samples of button pin
	uint8_t sample_1 = digitalRead(_buttonPin);
	delay(10);
	uint8_t sample_2 = digitalRead(_buttonPin);
	delay(10);
	uint8_t sample_3 = digitalRead(_buttonPin);
	delay(10);

	if (sample_1 != 0 || sample_1 != sample_2 || sample_2 != sample_3){
		return;
	}

	DEBUG_PRINTLN("BUTTON: Pressed");
	delay(3000);

	sample_1 = digitalRead(_buttonPin);
	delay(10);
	sample_2 = digitalRead(_buttonPin);
	delay(10);
	sample_3 = digitalRead(_buttonPin);
	delay(10);

	// If button is held for 3s resets device to factory settings
	if (sample_1 == 0 && sample_1 == sample_2 && sample_2 == sample_3){
		resetEEPROM();
		wdt_enable(WDTO_120MS);
		while (1);
	}
}

void GPSTracker::checkGSM(){
	wdt_reset();

	DEBUG_PRINTLN("GSM: Check");

	// Sends AT and waits for answer
	sendAT();
	if(waitFor("OK")){
		return;
	}

	DEBUG_PRINTLN("GSM: Restart");
	// If GSM doesn't respond, Arduino restarts it
	reset();
	delay(500);
	if (powerOn()){
		if (init()){
			// Enables GPS if it was enabled before
			if (_gpsPowerStatus == 1){
				powerGPS(true);
			}
		}
	}

}

void GPSTracker::updateGPSLocation(){
	wdt_reset();

	if (_gpsPowerStatus == 1){
		updateGPSStatusInfo();
	}
}


