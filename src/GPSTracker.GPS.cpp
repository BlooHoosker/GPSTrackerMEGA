#include <GPSTracker.h>

bool GPSTracker::powerGPS(bool on){

    if (on){
        // Powering up GPS
        sendAT("+CGNSPWR=1");
        if(!waitFor("OK")) return false;

        // Checking if GPS is powered up
        delay(2*TRACKER_SECOND);
        if (getGPSPowerStatus() != 1) return false;
    } else {
        // Powering down GPS
        sendAT("+CGNSPWR=0");
        if(!waitFor("OK")) return false;

        // Checking if GPS is powered down
        delay(2*TRACKER_SECOND);
        if (getGPSPowerStatus() != 0) return false; 
    }

    return true;
}

int8_t GPSTracker::getGPSPowerStatus(){

    char buffer[TRACKER_BUFFER_SIZE];

    if (!getGPSInfo(buffer, TRACKER_BUFFER_SIZE)) return -1;

    return parseGPSPowerStatus(buffer);
}

bool GPSTracker::getGPSPosition(char * lattitude, char * longitude, size_t size){

    char buffer[TRACKER_BUFFER_SIZE];

    if (!getGPSInfo(buffer, TRACKER_BUFFER_SIZE)) return false;

    // Checking GPS power status
    if (parseGPSPowerStatus(buffer) != 1){
        Serial.println("Wrong format1");
        return false;
    }

    // Checking GPS fix status
    if (parseGPSFixStatus(buffer) != 1) {
        Serial.println("Wrong format2");
        return false;
    }

    // Parsing GPS position
    if(parseGPSPosition(buffer, lattitude, longitude, size)){
        return true;
    } else {
        Serial.println("Wrong format3");
        return false;
    }
}

int8_t GPSTracker::getGPSFixStatus(){

    char buffer[TRACKER_BUFFER_SIZE];

    if (!getGPSInfo(buffer, TRACKER_BUFFER_SIZE)) return -1;

    return parseGPSFixStatus(buffer);
}

//+CGNSINF: 1,1,20210218230108.000,50.468168,13.421028,385.700,3.39,209.6,1,,5.0,5.1,1.0,,9,3,,,27,,
bool  GPSTracker::getGPSInfo(char * buffer, size_t bufferSize){

    memset(buffer, 0, bufferSize);

    // Send AT requesting GPS information sequence
    sendAT("+CGNSINF");
    if(!waitFor(buffer, bufferSize, TRACKER_DEFAULT_TIMEOUT, "+CGNSINF")){
        Serial.println("Failed to receive reply");
        return false;
    }

    if (!waitFor("OK")){
        Serial.println("Failed to receive OK");
        return false;
    }

    return true;
}

bool GPSTracker::parseGPSPosition(const char * CGNSINF, char * lattitude, char * longitude, size_t bufferSize){

    uint8_t length = strlen(CGNSINF);
    uint8_t index = 0;
    uint8_t positionIndex = 0;
    uint8_t delimiter;

    memset(lattitude, 0, bufferSize);
    memset(longitude, 0, bufferSize);

    // Latitude after 3rd ','
    // Reaches third ','
    delimiter = 3;
    for (index = 0; index < length; index++){
        if (CGNSINF[index] == ',') delimiter--;
        if (!delimiter) break;
    }

    // Check if third',' is reached
    if (delimiter){
        return false;
    }

    index++;
    // Copy latitude to *latitude until ',' is reached
    for (; index < length; index++){
        // If ending ',' is reached exit cycle
        if (CGNSINF[index] == ',') break;

        // If latitude is longer than buffer size return false
        if (positionIndex >= bufferSize) return false;
        lattitude[positionIndex] = CGNSINF[index];
        positionIndex++;
    }
    
    // Add '\0' at the end
    if (positionIndex >= bufferSize) return false;
    lattitude[positionIndex] = '\0';

    // If char at current position isn't ',' 
    // Means format is incorrect
    if (CGNSINF[index] != ',') return false;

    index++;
    positionIndex = 0;
    // Copy longitude to *longitude until ',' is reached
    for (; index < length; index++){
        // If ending ',' is reached exit cycle
        if (CGNSINF[index] == ',') break;

        // If longitude is longer than buffer size return false
        if (positionIndex >= bufferSize) return false;
        longitude[positionIndex] = CGNSINF[index];
        positionIndex++;
    }

    // Add '\0' at the end
    if (positionIndex >= bufferSize) return false;
    longitude[positionIndex] = '\0';

    // If char at current position isn't ',' 
    // Means format is incorrect
    if (CGNSINF[index] != ',') return false;

    return true;
}

int8_t  GPSTracker::parseGPSAccuracy(const char * CGNSINF){

}

int8_t  GPSTracker::parseGPSFixStatus(const char * CGNSINF){

    uint8_t length = strlen(CGNSINF);
    uint8_t index = 0;
    uint8_t delimiter;

    // Latitude after 3rd ','
    // Reaches third ','
    delimiter = 1;
    for (index = 0; index < length; index++){
        if (CGNSINF[index] == ',') delimiter--;
        if (!delimiter) break;
    }

    // Check if first ',' is reached
    if (delimiter){
        return -1;
    }

    index++;
    if (index >= length) return false;

    // Return status of GPS fix
    // -1 in case of error
    if (CGNSINF[index] == '0') {
        return 0;
    } else if (CGNSINF[index] == '1'){
        return 1;
    } else {
        return -1;
    } 
}

int8_t GPSTracker::parseGPSPowerStatus(const char * CGNSINF){

    uint8_t length = strlen(CGNSINF);
    uint8_t index = 0;
    uint8_t delimiter;

    // Latitude after 3rd ','
    // Reaches third ','
    delimiter = 1;
    for (index = 0; index < length; index++){
        if (CGNSINF[index] == ',') delimiter--;
        if (!delimiter) break;
    }

    // Check if first ',' is reached
    if (delimiter) return -1;

    index--;
    if (index < 0) return -1;

    // Return GPS power status
    // -1 in case of error
    if (CGNSINF[index] == '0') {
        return 0;
    } else if (CGNSINF[index] == '1'){
        return 1;
    } else {
        return -1;
    } 
}