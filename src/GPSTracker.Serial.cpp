#include <GPSTracker.h>

bool GPSTracker::waitFor(const char *command, uint16_t timeout = TRACKER_DEFAULT_TIMEOUT){

	char buffer[TRACKER_BUFFER_SIZE];
	uint8_t length = 0;

	do {
		memset(buffer, 0, TRACKER_BUFFER_SIZE);
		length = readAT(buffer, TRACKER_BUFFER_SIZE, &timeout);
		if (length > 0){

			/*
			Serial.println("Reading in Wait");
			
			Serial.print("Length of read received bytes ");
			Serial.println(length);
			Serial.print(buffer);	

			
			Serial.print("Length of command ");
			Serial.println(strlen(command));
			Serial.println(command);

			
			for (int i = 0; i < strlen(buffer); i++ ){
				Serial.print((uint8_t)buffer[i]);
			}
			Serial.println();

			for (int i = 0; i < strlen(command); i++ ){
				Serial.print((uint8_t)command[i]);
			}
			Serial.println();
			*/

			if (compareAT(buffer, command)){
				return true;
			}
		}
	} while (timeout > 0);
	
	return false;
}

bool GPSTracker::waitFor(char * buffer, size_t bufferSize, uint16_t timeout, const char * command){

	uint8_t length = 0;

	do {
		memset(buffer, 0, bufferSize);
		length = readAT(buffer, bufferSize, &timeout);
		if (length > 0){
			
			/*
			Serial.println("Reading in Wait");
			
			Serial.print("Length of read received bytes ");
			Serial.println(length);
			Serial.print(buffer);	

			
			Serial.print("Length of command ");
			Serial.println(strlen(command));
			Serial.print(command);

			
			for (int i = 0; i < strlen(command); i++ ){
				Serial.print((uint8_t)trackerBuffer[i]);
			}
			Serial.println();

			for (int i = 0; i < strlen(command); i++ ){
				Serial.print((uint8_t)command[i]);
			}
			Serial.println();
			*/

			if (compareAT(buffer, command)){
				return true;
			}
		}
	} while (timeout > 0);
	
	return false;
}

size_t GPSTracker::readAT(char * buffer, size_t size, uint16_t * timeout)
{
	size_t i = 0;
	bool cr = false;
	bool nl = false;
	bool end = false;
	char c = 0;

	// If timeout is NULL returns 0
	if (!timeout) return 0;

	memset(buffer, 0, size);

	do {
		// Read from serial as long as data is available,
		// Size is less than buffer size
		// Terminating sequence \r\n is not reached
		while(!end && (i < size-2) && _serialPort->available()){
			c = _serialPort->read();
			buffer[i] = c;
			i++;
			// Checking terminating sequence \r\n
			if (cr){
				nl = (c == '\n');
				if (nl) {
					end = true;
					break;
				} else {
					nl = false;
					cr = false;
				}
			}
			cr = (c == '\r');
		}

		// Check if both terminating characters were reached
		if (end) {
			break;
		}

		if (*timeout){
			(*timeout)--;
			delay(1);
		} else {
			return 0;
		}

	} while(!end && i < size - 1);

	if (!end) {
		return 0;
	}
	buffer[i] = '\0';

	return i;
}

bool GPSTracker::setEchoMode(bool echo){

	if(echo){
		sendAT("E1");
	} else {
		sendAT("E0");
	}

	return waitFor("OK");
}
