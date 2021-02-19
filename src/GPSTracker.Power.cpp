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
	uint16_t timeout = TRACKER_DEFAULT_TIMEOUT;
	memset(buffer, 0, TRACKER_BUFFER_SIZE);

	// Send AT and waiting for response
	sendAT();

	readNext(buffer, TRACKER_BUFFER_SIZE, &timeout, '\n');

	if (compareAT(buffer, "\r\n" || compareAT(buffer, "AT") || compareAT(buffer, "OK") || compareAT(buffer, "ERROR")){
		return true;
	}

	return false;
}


