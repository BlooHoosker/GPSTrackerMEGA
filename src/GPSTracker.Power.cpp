#include <GPSTracker.h>
#include <BoardConfig.h>

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
		if ( powered() ) return true;
		delay(300);
	}

	return false;
}

bool GPSTracker::powered(){
	char buffer[TRACKER_BUFFER_SHORT];
	
	// Send AT and waiting for response
	sendAT();

	// Wait for any AT response
	if (!receiveAT(buffer, TRACKER_BUFFER_SHORT, TRACKER_DEFAULT_TIMEOUT)) return false;

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

bool GPSTracker::setGsmSleepMode(){
	sendAT("+CSCLK=1");
	if(!waitFor("OK")){
		return false;
	}
    return true;
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

void GPSTracker::checkBatteryPercentage(){

	unsigned long sum = 0;
	float voltage = 0;

	for (int i = 0; i < 100; i++){
		sum += analogRead(_batteryPin);
	}

	voltage = (sum / 100) * (5.0 / 1023.0);

	// todo map to 4.2-3.4
	_batteryPercentage = (voltage - BATTERY_VMIN) * 100 / (BATTERY_VMAX - BATTERY_VMIN);

	if (_batteryPercentage > 100) {
		_batteryPercentage = 100;
	} else if (_batteryPercentage < 0){
		_batteryPercentage = 0;
	}

	if (_batteryPercentage < 20){
		if (!_batteryWarningSent){
			//sendSMS("LOCATOR BATTERY LESS THAN 20%", _phoneNumber);
			_batteryWarningSent = true;
		}
	} else if (_batteryPercentage > 25){
		_batteryWarningSent = false;
	}
}
