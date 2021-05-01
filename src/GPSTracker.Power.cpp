#include <GPSTracker.h>
#include <BoardConfig.h>

bool GPSTracker::powerOn(){

	// Check if module is already powered on
	bool currentlyPowered = powered();
	if (currentlyPowered == true) return true;

    // Hold PWR PIN high for 1 second to power on module
	// Timings based on documentation
	digitalWrite(_powerPin, LOW);
	delay(600); // setup delay
	digitalWrite(_powerPin, HIGH);
	delay(1100);
	digitalWrite(_powerPin, LOW);
	delay(1900);

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

void GPSTracker::reset(){
	// Based on documentation, modul can be reset by holding reset pin low
    digitalWrite(_resetPin, HIGH);
	delay(10);
	digitalWrite(_resetPin, LOW);
	// Reset has to be held low for at least 105ms
	delay(150);
	digitalWrite(_resetPin, HIGH);
	// After reset it takes about 2.6s to be active
	delay(2600);
}

void GPSTracker::checkBatteryPercentage(){

	_batteryPercentage = getBatteryPercentage();

	if (_batteryPercentage < 20){
		if (!_batteryWarningSent){
			//sendSMS("LOCATOR BATTERY LESS THAN 20%", _phoneNumber);
			_batteryWarningSent = true;
		}
	} else if (_batteryPercentage > 25){
		_batteryWarningSent = false;
	}
}

uint8_t GPSTracker::getBatteryPercentage(){
	unsigned long sum = 0;
	float voltage = 0;
	uint8_t batteryPercentage;

	for (int i = 0; i < 100; i++){
		sum += analogRead(_batteryPin);
	}

	voltage = (sum / 100) * (5.0 / 1023.0);

	// Percentage between 4.2 - 3.0
	batteryPercentage = (voltage - BATTERY_VMIN) * 100 / (BATTERY_VMAX - BATTERY_VMIN);

	if (batteryPercentage > 100) {
		batteryPercentage = 100;
	} else if (batteryPercentage < 0){
		batteryPercentage = 0;
	}

	return batteryPercentage;
}

// bool GPSTracker::enableGsmSleepMode(){
// 	sendAT("+CSCLK=1");
// 	if(!waitFor("OK")){
// 		return false;
// 	}
// 	gsmSleep();
//     return true;
// }

// bool GPSTracker::disableGsmSleepMode(){
// 	gsmWake();
// 	sendAT("+CSCLK=0");
// 	if(!waitFor("OK")){
// 		return false;
// 	}
//     return true;
// }

// void GPSTracker::gsmSleep(){
// 	// If GPS is off, GSM can go into sleep mode
// 	if (!_gpsPowerStatus){
// 		digitalWrite(_dtrPin, HIGH);
// 	}
// }

// void GPSTracker::gsmWake(){
// 	digitalWrite(_dtrPin, LOW);
// 	delay(100);
// }

