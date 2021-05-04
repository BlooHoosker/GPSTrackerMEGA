#include <GPSTracker.h>

bool GPSTracker::waitFor(const char *command, uint16_t timeout /*= TRACKER_DEFAULT_TIMEOUT*/){

	char buffer[TRACKER_BUFFER_LARGE];
	uint8_t length = 0;

	do {
		memset(buffer, 0, TRACKER_BUFFER_LARGE);
		length = readAT(buffer, TRACKER_BUFFER_LARGE, &timeout);
		if (length > 0){

			// Compare received AT command with what is requested
			if (compareAT(buffer, command)){
				return true;
			}

			// Check if received AT is CMTI indicating new message
			if (decodeAT(buffer) == 0){
				queueInsert(buffer);
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
			
			// Compare received AT command with what is requested
			if (compareAT(buffer, command)){
				return true;
			}

			// Check if received AT is CMTI indicating new message
			if (decodeAT(buffer) == 0){
				queueInsert(buffer);
			}
		}
	} while (timeout > 0);
	
	return false;
}

uint16_t GPSTracker::readAT(char * buffer, size_t size, uint16_t * timeout)
{
	uint16_t i = 0;
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
