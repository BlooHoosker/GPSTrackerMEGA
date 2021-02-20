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

bool GPSTracker::getGPSPosition(char * lattitude, char * longitude, size_t bufferSize){

    char buffer[TRACKER_BUFFER_SIZE];

    if (!getGPSInfo(buffer, TRACKER_BUFFER_SIZE)) return false;

    Serial.print(buffer);

    // Checking GPS power status
    if (parseGPSPowerStatus(buffer) != 1){
        return false;
    }

    // Checking GPS fix status
    if (parseGPSFixStatus(buffer) != 1) {
        return false;
    }

    // Parsing GPS position
    if(parseGPSPosition(buffer, lattitude, longitude, bufferSize)){
        return true;
    } else {
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

bool GPSTracker::parseGPSValue(const char * CGNSINF, uint8_t valuePosition, char * value, uint8_t valueSize){

    uint8_t length = strlen(CGNSINF);
    const char prefix[] = "+CGNSINF: ";
    uint8_t prefixLength = strlen(prefix);
    uint8_t position = 0;

    // Checking if length of CGNSINF sequence is longer than prefix
    if (length <= prefixLength){
        Serial.println("Wrong format");
        return false;
    }

    // Checking if prefix is correct
    if (strncmp(CGNSINF, prefix, prefixLength) != 0){
        Serial.println("Wrong AT sequence");
        return false;
    }

    // Reaches requested position in sequence
    uint8_t index;
    for (index = prefixLength; index < length; index++){
        if (CGNSINF[index] == ',') position++;
        if (position == valuePosition) break;
    }

    // Checks if position was reached
    if (position != valuePosition) {
        Serial.println("Failed to reach requested position");
        return false;
    }

    memset(value, 0, valueSize);

    // If position is not 0 then move by one character to avoid ','
    if (valuePosition) index++;

    uint8_t valueIndex = 0;
    for (; index < length; index++){
        if (CGNSINF[index] == ',' || CGNSINF[index] == '\r') break;
        
        if (valueIndex >= valueSize){
            Serial.println("Value larger than buffer size1");
            return false;
        } 
        value[valueIndex] = CGNSINF[index];
        valueIndex++;
    }

    // Checks if correct end was reached
    if (CGNSINF[index] != ',' && CGNSINF[index] != '\r') return false;

    // Adds \0 at the end
    if (valueIndex >= valueSize){
        Serial.println("Value larger than buffer size2");
        return false;
    } 
    value[valueIndex] = '\0';

    return true;
}

bool GPSTracker::parseGPSPosition(const char * CGNSINF, char * lattitude, char * longitude, size_t bufferSize){

    if (!parseGPSValue(CGNSINF, 3, lattitude, bufferSize)){
        Serial.println("Failed to parse latitude");
        return false;
    }

    if (!parseGPSValue(CGNSINF, 4, longitude, bufferSize)){
        Serial.println("Failed to parse latitude");
        return false;
    }

    return true;
}

int8_t  GPSTracker::parseGPSAccuracy(const char * CGNSINF){

}

int8_t  GPSTracker::parseGPSFixStatus(const char * CGNSINF){

    char status[2] = "\0";

    if (!parseGPSValue(CGNSINF, 1, status, 2)){
        Serial.println("Failed to parse power status");
        return false;
    }

    // Return GPS power status
    // -1 in case of error
    if (status[0] == '0') {
        return 0;
    } else if (status[0] == '1'){
        return 1;
    } else {
        return -1;
    } 
}

int8_t GPSTracker::parseGPSPowerStatus(const char * CGNSINF){

    char status[2] = "\0";

    if (!parseGPSValue(CGNSINF, 0, status, 2)){
        Serial.println("Failed to parse power status");
        return false;
    }

    // Return GPS power status
    // -1 in case of error
    if (status[0] == '0') {
        return 0;
    } else if (status[0] == '1'){
        return 1;
    } else {
        return -1;
    } 
}