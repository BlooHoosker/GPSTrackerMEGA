#include <GPSTracker.h>
#include <EEPROM.h>
#include <avr/wdt.h>

void GPSTracker::printStatus(){
	updateGPSStatusInfo();
	Serial.print("Power status: ");
	Serial.println(_powerStatus);
	Serial.print("Fix status: ");
	Serial.println(_fixStatus);
	Serial.print("Master set: ");
	Serial.println(_masterNumberSet);
	Serial.print("Latitude: ");
	Serial.println(_latitude);
	Serial.print("Longitude: ");
	Serial.println(_longitude);
}

void GPSTracker::test(){
	EEPROM[RESTART_ADDR] = 0;
}

GPSTracker::GPSTracker(uint8_t SIM_RESET_PIN, uint8_t SIM_PWR_PIN){
    _resetPin = SIM_RESET_PIN;
    _powerPin = SIM_PWR_PIN;
    pinMode(_resetPin, OUTPUT);
    digitalWrite(_resetPin, HIGH);
    pinMode(_powerPin, OUTPUT);
    digitalWrite(_powerPin, LOW);

	strcpy(_phoneNumber, "+420732885552");

	memset(_latitude, 0, TRACKER_PHONE_NUBER_SIZE);
	memset(_longitude, 0, TRACKER_PHONE_NUBER_SIZE);

	_powerStatus = 0;
	_fixStatus = 0;
	_masterNumberSet = 0;
}

GPSTracker::~GPSTracker(){}

void GPSTracker::restart(){
	Serial.println("Restarting");
	if (!_masterNumberSet){
		setMasterNumber(_phoneNumber);
		resetMasterNumber();
	}
	EEPROM[RESTART_ADDR] = 0xFF;
	wdt_enable(WDTO_120MS);
}

bool GPSTracker::start(Stream &serial){

    _serialPort = &serial;

    Serial.println("Powering up..");
    
    // Power on module
    bool powerStatus = powerOn();

    if (!powerStatus){
        Serial.println("Failed to power up");
        return false;
    }

    Serial.println("Initializing..");
    
    // Initialize module
    if (!init()) {
        Serial.println("Initialization Failed");
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
		sendSMS("RESTART COMPLETE", _phoneNumber);
		Serial.println("Restart complete");
		Serial.println(_phoneNumber);
	}
	
	// Resetting restart flag
	EEPROM.update(RESTART_ADDR, 0);

    Serial.println("Initialization Success");
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
bool GPSTracker::init(){

	bool received = false;
	// Reset the module to start clean initialization
	reset();
	delay(1000);

	// Load master number from EEPROM
	getMasterNumber();

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
		Serial.println("Echo not disabled");
        return false;
	} 

	// Set SMS message format to "Text"
	if (!setSMSMessageMode(true)) {
		Serial.println("Failed to set SMS text mode");
		return false;
	}

	// Set storage for SMS to module itself
	sendAT("+CPMS=\"ME\",\"ME\",\"ME\"");
	if(!waitFor("OK")){
		Serial.println("Failed to set storage");
		return false;
	}

	if (!deleteAllSMS()){
		Serial.println("Failed to receive OK delsms");
		return false;
	}

	return true;
}

void GPSTracker::reset(){
    digitalWrite(_resetPin, HIGH);
	delay(10);
	digitalWrite(_resetPin, LOW);
	delay(200);
	digitalWrite(_resetPin, HIGH);
}

