#include <GPSTracker.h>
#include <EEPROM.h>
#include <avr/wdt.h>

void GPSTracker::printStatus(){
	updateGPSStatusInfo();
	Serial.print("Latitude: ");
	Serial.println(_latitude);
	Serial.print("Longitude: ");
	Serial.println(_longitude);
	Serial.print("Power status: ");
	Serial.println(_powerStatus);
	Serial.print("Fix status: ");
	Serial.println(_fixStatus);
	Serial.print("Master set: ");
	Serial.println(_masterNumberSet);
	Serial.print("Link source: ");
	Serial.println(_mapLinkSrc);
	Serial.print("Battery: ");
	Serial.println(_batteryPercentage);
}

void GPSTracker::resetEEPROM(){
	for (uint8_t i = 0; i < 6+TRACKER_BUFFER_SHORT; i++){
		EEPROM.update(i, 0);
	}
}

void GPSTracker::test(){
	char trackerReceiveBuffer[TRACKER_BUFFER_SHORT];
	memset(trackerReceiveBuffer, 0, TRACKER_BUFFER_SHORT);
	delay(1000);
	Serial.println("Sleep");
	gsmSleep();
	delay(5000);
	//gsmWake();
	//Serial.println("Wake");
	Serial.println("recieving");
	while(1){
		if (receiveAT(trackerReceiveBuffer, TRACKER_BUFFER_SHORT, 5*TRACKER_SECOND)){
    		Serial.print(trackerReceiveBuffer);
		}
	}
}

GPSTracker::GPSTracker(uint8_t SIM_RESET_PIN, uint8_t SIM_PWR_PIN, uint8_t SIM_DTR_PIN, uint8_t RST_BTN_PIN, uint8_t BATTERY_PIN){

    _resetPin = SIM_RESET_PIN;
	pinMode(_resetPin, OUTPUT);
    digitalWrite(_resetPin, HIGH);

    _powerPin = SIM_PWR_PIN;
    pinMode(_powerPin, OUTPUT);
    digitalWrite(_powerPin, LOW);

	_dtrPin = SIM_DTR_PIN;
	pinMode(_dtrPin, OUTPUT);
    digitalWrite(_dtrPin, LOW);

	_buttonPin = RST_BTN_PIN;
	pinMode(_dtrPin, INPUT_PULLUP);

	_batteryPin = BATTERY_PIN;
	//pinmode?

	memset(_phoneNumber, 0, TRACKER_BUFFER_SHORT);
	memset(_latitude, 0, TRACKER_BUFFER_SHORT);
	memset(_longitude, 0, TRACKER_BUFFER_SHORT);

	_powerStatus = 0;
	_fixStatus = 0;
	_masterNumberSet = 0;
	_mapLinkSrc = 0;
	_batteryPercentage = 0;
}

GPSTracker::~GPSTracker(){}

void GPSTracker::restart(){
	// Check if master number is set
	if (!_masterNumberSet){
		// If its not set, save phone number to memory to send reply after restart
		setMasterNumber(_phoneNumber);
		resetMasterNumber();
	}
	EEPROM[RESTART_ADDR] = 0xFF;
	wdt_enable(WDTO_120MS);
}

bool GPSTracker::start(Stream &serial){

    _serialPort = &serial;

    Serial.println("START: Powering up..");
    
    // Power on module
    bool powerStatus = powerOn();

    if (!powerStatus){
        Serial.println("START: Failed to power up");
        return false;
    }

    Serial.println("START: Initializing..");
    
    // Initialize module
    if (!init()) {
        Serial.println("START: Initialization Failed");
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
		Serial.println("RESTART: Sending reply");
		sendSMS("RESTART COMPLETE", _phoneNumber);
		Serial.println("START: Restart complete");
		Serial.println(_phoneNumber);
	}
	
	// Resetting restart flag
	EEPROM.update(RESTART_ADDR, 0);
    return true;
}

// Reset
// Delay
// Send AT
// Wait for AT
// Wait for SMS ready
// Disable ECHO
// Set SMS text mode
// Set SMS storage mode to module itself 
// Delete all stored SMS in module
bool GPSTracker::init(){

	bool received = false;
	// Reset the module to start clean initialization
	reset();
	delay(1000);

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
		Serial.println("INIT: Echo not disabled");
        return false;
	} 

	// Set SMS message format to "Text"
	if (!setSMSMessageMode(true)) {
		Serial.println("INIT: Failed to set SMS text mode");
		return false;
	}

	// Set storage for SMS to module itself
	if (!setSmsStorage()){
		Serial.println("INIT: Failed to set storage");
		return false;
	}

	// Deleting all messages from module to create space
	if (!deleteAllSMS()){
		Serial.println("INIT: Failed to receive OK delsms");
		return false;
	}

	// if (!setGsmSleepMode()){
	// 	Serial.println("INIT: Failed to receive OK sleep mode");
	// 	return false;
	// }

	return true;
}

void GPSTracker::checkButton(){

	uint8_t sample_1 = digitalRead(_buttonPin);
	delay(10);
	uint8_t sample_2 = digitalRead(_buttonPin);
	delay(10);
	uint8_t sample_3 = digitalRead(_buttonPin);
	delay(10);

	if (sample_1 != 0 || sample_1 != sample_2 || sample_2 != sample_3){
		return;
	}

	Serial.println("BUTTON: Pressed");
	delay(3000);

	sample_1 = digitalRead(_buttonPin);
	delay(10);
	sample_2 = digitalRead(_buttonPin);
	delay(10);
	sample_3 = digitalRead(_buttonPin);
	delay(10);


	if (sample_1 == 0 && sample_1 == sample_2 && sample_2 == sample_3){
		resetEEPROM();
		wdt_enable(WDTO_120MS);
		while (1);
	}
}

void GPSTracker::checkGSM(){

	sendAT();

	if(waitFor("OK")){
		return;
	}

	init();

	if (_powerStatus){
		powerGPS(true);
	}
}