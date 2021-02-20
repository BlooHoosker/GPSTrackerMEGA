#include <GPSTracker.h>

void GPSTracker::test(){

	char buffer[TRACKER_BUFFER_SIZE];
    memset(buffer, 0, TRACKER_BUFFER_SIZE);
	char lattitude[TRACKER_PHONE_NUBER_SIZE];
	char longitude[TRACKER_PHONE_NUBER_SIZE];
	
	if(!powerGPS(true)){
		Serial.println("GPS Failed to enable");
		return;
	}

	Serial.println("GPS on");
	// delay(5000);
	// sendAT("+CGNSSEQ=GGA");
	// if(!waitFor(buffer, TRACKER_BUFFER_SIZE, TRACKER_DEFAULT_TIMEOUT, "+CGNSREQ")){
	// 	Serial.println("Nothing received");
	// }
	// waitFor("OK");
	while (1){
		// sendAT("+CGNSSEQ?");
		// if(!waitFor(buffer, TRACKER_BUFFER_SIZE, TRACKER_DEFAULT_TIMEOUT, "+CGNSREQ")){
		// 	Serial.println("Nothing received");
		// }
		// waitFor("OK");
		// Serial.print(buffer);
		Serial.print("Power status: ");
		Serial.println(getGPSPowerStatus());
		Serial.print("Fix status: ");
		Serial.println(getGPSFixStatus());
		if (getGPSPosition(lattitude, longitude, TRACKER_PHONE_NUBER_SIZE)){
			Serial.println(lattitude);
			Serial.println(longitude);
		} else {
			Serial.println("Fail");
		}
		delay(2000);
	}
}

GPSTracker::GPSTracker(uint8_t SIM_RESET_PIN, uint8_t SIM_PWR_PIN){
    _resetPin = SIM_RESET_PIN;
    _powerPin = SIM_PWR_PIN;
    pinMode(_resetPin, OUTPUT);
    digitalWrite(_resetPin, HIGH);
    pinMode(_powerPin, OUTPUT);
    digitalWrite(_powerPin, LOW);
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

	return true;
}

void GPSTracker::reset(){
    digitalWrite(_resetPin, HIGH);
	delay(10);
	digitalWrite(_resetPin, LOW);
	delay(200);
	digitalWrite(_resetPin, HIGH);
}

