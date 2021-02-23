#include <GPSTracker.h>

bool GPSTracker::waitFor(const char *command, uint16_t timeout = TRACKER_DEFAULT_TIMEOUT){

	char buffer[TRACKER_BUFFER_SIZE];
	uint8_t length = 0;

	do {
		memset(buffer, 0, TRACKER_BUFFER_SIZE);
		length = readNext(buffer, TRACKER_BUFFER_SIZE, &timeout, '\n');
		if (length > 0){

			/*
			Serial.println("Reading in Wait");
			
			Serial.print("Length of read received bytes ");
			Serial.println(length);
			Serial.print(trackerBuffer);	

			
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

bool GPSTracker::waitFor(char * buffer, size_t bufferSize, uint16_t timeout, const char * command){

	uint8_t length = 0;

	do {
		memset(buffer, 0, bufferSize);
		length = readNext(buffer, bufferSize, &timeout, '\n');
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

size_t GPSTracker::readNext(char * buffer, size_t size, uint16_t * timeout, char stop)
{
	size_t i = 0;
	bool exit = false;

	memset(buffer, 0, size);

	do {
		while(!exit && i < size - 1 && _serialPort->available()) {
			char c = _serialPort->read();
			buffer[i] = c;
			i++;
			exit |= stop && c == stop;
		}

		if(timeout) {			
			if(*timeout) {
				delay(1);
				(*timeout)--;
			}

			if(!(*timeout)) break;
		}
	} while(!exit && i < size - 1);

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
