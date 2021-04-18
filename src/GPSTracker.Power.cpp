#include <GPSTracker.h>

bool GPSTracker::powerOn(){

	// Check if module is already powered on
	bool currentlyPowered = powered();
	if (currentlyPowered == true) return true;

    // Hold PWR PIN low for 1 second to power on module
	digitalWrite(_powerPin, LOW);
	delay(1000);
	digitalWrite(_powerPin, HIGH);
	delay(1000);
	digitalWrite(_powerPin, LOW);

	// Send "AT" and wait for "AT" response
	// If "AT" response is received, the module is powered on
	// 4 attempts in total
	for (uint8_t i = 4; i > 0; i--){
		sendAT();
		if ( waitFor("AT") ) return true;
		delay(300);
	}

	return false;
}

bool GPSTracker::powered(){
	char buffer[TRACKER_BUFFER_SIZE];
	
	// Send AT and waiting for response
	sendAT();

	// Wait for any AT response
	if (!receiveAT(buffer, TRACKER_BUFFER_SIZE, TRACKER_DEFAULT_TIMEOUT)) return false;

	return true;
}

// TODO make own version or explain based on doc
void GPSTracker::reset(){
    digitalWrite(_resetPin, HIGH);
	delay(10);
	digitalWrite(_resetPin, LOW);
	delay(200);
	digitalWrite(_resetPin, HIGH);
}

void GPSTracker::checkBatteryPercentage(){

}

bool GPSTracker::setGsmSleepMode(){
	sendAT("+CSCLK=1");
	if(!waitFor("OK")){
		return false;
	}
    return true;
	gsmSleep();
}

void GPSTracker::gsmSleep(){
	// If GPS is off, GSM can go into sleep mode
	if (!_powerStatus){
		digitalWrite(_dtrPin, HIGH);
	}
}

void GPSTracker::gsmWake(){
	digitalWrite(_dtrPin, LOW);
	delay(60);
}
