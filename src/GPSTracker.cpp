#include <GPSTracker.h>

void GPSTracker::printStatus(){
	char buffer[TRACKER_BUFFER_SIZE];
	char latitude[TRACKER_PHONE_NUBER_SIZE];
	char longitude[TRACKER_PHONE_NUBER_SIZE];

	Serial.print("Power status: ");
	Serial.println(getGPSPowerStatus());
	Serial.print("Fix status: ");
	Serial.println(getGPSFixStatus());
	if (getGPSPosition(latitude, longitude, TRACKER_PHONE_NUBER_SIZE)){
		Serial.println(latitude);
		Serial.println(longitude);
	} else {
		Serial.println("Fail");
	}
}

void GPSTracker::test(){


}

GPSTracker::GPSTracker(uint8_t SIM_RESET_PIN, uint8_t SIM_PWR_PIN){
    _resetPin = SIM_RESET_PIN;
    _powerPin = SIM_PWR_PIN;
    pinMode(_resetPin, OUTPUT);
    digitalWrite(_resetPin, HIGH);
    pinMode(_powerPin, OUTPUT);
    digitalWrite(_powerPin, LOW);
	strcpy(_phoneNum, "+420732885552");
}

GPSTracker::~GPSTracker(){}

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

	Serial.println("Disabling echo..");
	// Disable echoing of sent commands
	if (!setEchoMode(false)){
		Serial.println("Echo not disabled");
        return false;
	} 

	Serial.println("Echo disabled\nSetting SMS mode..");
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

	// sendAT("+CNMI=0");
	// if(!waitFor("OK")){
	// 	Serial.println("Failed to set new SMS mode");
	// 	return false;
	// }

	Serial.println("Deleting SMS");
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

