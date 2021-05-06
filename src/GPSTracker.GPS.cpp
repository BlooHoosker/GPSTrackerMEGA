#include <GPSTracker.h>

bool GPSTracker::powerGPS(bool on){
    wdt_reset();

    if (on){
        // Powering up GPS
        sendAT("+CGNSPWR=1");
        if(!waitFor("OK")) return false;

        // Checking if GPS is powered up
        delay(2*TRACKER_SECOND);
        if (getGPSPowerStatus() != 1) return false;
        
        _gpsPowerStatus = 1;
    } else {
        // Powering down GPS
        sendAT("+CGNSPWR=0");
        if(!waitFor("OK")) return false;

        // Checking if GPS is powered down
        delay(2*TRACKER_SECOND);
        if (getGPSPowerStatus() != 0) return false; 

        _gpsPowerStatus = 0;
    }

    return true;
}

bool GPSTracker::updateGPSStatusInfo(){
    wdt_reset();

    char buffer[TRACKER_BUFFER_LARGE];
    char latitude[TRACKER_BUFFER_SHORT];
    char longitude[TRACKER_BUFFER_SHORT];
    char time[TRACKER_BUFFER_TIME];
    char date[TRACKER_BUFFER_DATE];

    int8_t powerStatus = 0;
    int8_t fixStatus = 0;

    _gpsPowerStatus = 0;   
    _gpsFixStatus = 0;

    // Getting GPS info sequence
    if (!getGPSInfo(buffer, TRACKER_BUFFER_LARGE)){
        DEBUG_PRINTLN("UPDATE: Failed to get info");
        return false;
    }

    // Parsing GPS power status
    powerStatus = parseGPSPowerStatus(buffer);
    if (powerStatus == -1) {
        DEBUG_PRINTLN("UPDATE: Failed to parse power status");
        return false;        
    } 

    // If GPS is disabled, returns
    _gpsPowerStatus = powerStatus;
    if (!powerStatus){
        return true;
    }

    // Parsing GPS fix status
    fixStatus = parseGPSFixStatus(buffer);
    if (fixStatus == -1){
        DEBUG_PRINTLN("UPDATE: Failed to parse fix status");
        return false;           
    }

    // If GPS doesn't have fix, returns
    _gpsFixStatus = fixStatus;
    if (!fixStatus) return true;

    // Parsing GPS position coordinates
    if (!parseGPSPosition(buffer, latitude, longitude, TRACKER_BUFFER_SHORT)){
        DEBUG_PRINTLN("UPDATE: Failed to parse position");
        return false;
    }

    // Parsing time and date
    if(!parseTimeAndDate(buffer, date, TRACKER_BUFFER_DATE, time, TRACKER_BUFFER_TIME)){
        DEBUG_PRINTLN("UPDATE: Failed to parse time and date");
        return false;
    }
    
    // Copy string to class arrays.
    strncpy(_date, date, TRACKER_BUFFER_DATE);
    strncpy(_time, time, TRACKER_BUFFER_TIME);
    strcpy(_latitude, latitude);
    strcpy(_longitude, longitude);
    return true;
}

//+CGNSINF: 1,1,20210218230108.000,50.468168,13.421028,385.700,3.39,209.6,1,,5.0,5.1,1.0,,9,3,,,27,,
bool  GPSTracker::getGPSInfo(char * buffer, size_t bufferSize){
    wdt_reset();

    // Send AT requesting GPS information sequence
    sendAT("+CGNSINF");
    if(!waitFor(buffer, bufferSize, TRACKER_DEFAULT_TIMEOUT, "+CGNSINF")){
        DEBUG_PRINTLN("GPS INFO: Failed to receive reply");
        return false;
    }

    if (!waitFor("OK")){
        DEBUG_PRINTLN("GPS INFO: Failed to receive OK");
        return false;
    }

    return true;
}

int8_t GPSTracker::getGPSPowerStatus(){
    wdt_reset();

    char buffer[TRACKER_BUFFER_LARGE];

    if (!getGPSInfo(buffer, TRACKER_BUFFER_LARGE)) return -1;

    return parseGPSPowerStatus(buffer);
}

bool GPSTracker::getGPSPosition(char * latitude, char * longitude, size_t bufferSize){
    wdt_reset();

    char buffer[TRACKER_BUFFER_LARGE];

    if (!getGPSInfo(buffer, TRACKER_BUFFER_LARGE)) return false;

    DEBUG_PRINT(buffer);

    // Checking GPS power status
    if (parseGPSPowerStatus(buffer) != 1){
        return false;
    }

    // Checking GPS fix status
    if (parseGPSFixStatus(buffer) != 1) {
        return false;
    }

    // Parsing GPS position
    if(parseGPSPosition(buffer, latitude, longitude, bufferSize)){
        return true;
    } else {
        return false;
    }
}

int8_t GPSTracker::getGPSFixStatus(){
    wdt_reset();

    char buffer[TRACKER_BUFFER_LARGE];

    if (!getGPSInfo(buffer, TRACKER_BUFFER_LARGE)) return -1;

    // Checking GPS power status
    // TODO check if this is good idea to return 0
    if (parseGPSPowerStatus(buffer) != 1){
        return 0;
    }

    return parseGPSFixStatus(buffer);
}

bool GPSTracker::parseGPSValue(const char * CGNSINF, uint8_t valuePosition, char * value, uint8_t valueSize){
    wdt_reset();

    uint8_t length = strlen(CGNSINF);
    const char prefix[] = "+CGNSINF: ";
    uint8_t prefixLength = strlen(prefix);
    uint8_t position = 0;

    // Checking if length of CGNSINF sequence is longer than prefix
    if (length <= prefixLength){
        DEBUG_PRINTLN("GPS VALUE: Wrong format");
        return false;
    }

    // Checking if prefix is correct
    if (strncmp(CGNSINF, prefix, prefixLength) != 0){
        DEBUG_PRINTLN("GPS VALUE: Wrong AT sequence");
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
        DEBUG_PRINTLN("GPS VALUE: Failed to reach requested position");
        return false;
    }

    memset(value, 0, valueSize);

    // If position is not 0 then move by one character to avoid ','
    if (valuePosition) index++;

    // Copy value to buffer
    uint8_t valueIndex = 0;
    for (; index < length; index++){
        if (CGNSINF[index] == ',' || CGNSINF[index] == '\r') break;
        
        if (valueIndex >= valueSize){
            DEBUG_PRINTLN("GPS VALUE: Value larger than buffer size1");
            return false;
        } 
        value[valueIndex] = CGNSINF[index];
        valueIndex++;
    }

    // Checks if correct end was reached
    if (CGNSINF[index] != ',' && CGNSINF[index] != '\r') return false;

    // Adds \0 at the end
    if (valueIndex >= valueSize){
        DEBUG_PRINTLN("GPS VALUE: Value larger than buffer size2");
        return false;
    } 
    value[valueIndex] = '\0';

    return true;
}

bool GPSTracker::parseGPSPosition(const char * CGNSINF, char * latitude, char * longitude, size_t bufferSize){
    wdt_reset();

    if (!parseGPSValue(CGNSINF, 3, latitude, bufferSize)){
        DEBUG_PRINTLN("GPS POSITION: Failed to parse latitude");
        return false;
    }

    if (!parseGPSValue(CGNSINF, 4, longitude, bufferSize)){
        DEBUG_PRINTLN("GPS POSITION: Failed to parse longitude");
        return false;
    }

    return true;
}

int8_t  GPSTracker::parseGPSFixStatus(const char * CGNSINF){
    wdt_reset();

    char status[2] = "\0";

    if (!parseGPSValue(CGNSINF, 1, status, 2)){
        DEBUG_PRINTLN("GPS FIX: Failed to parse power status");
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
    wdt_reset();

    char status[2] = "\0";

    if (!parseGPSValue(CGNSINF, 0, status, 2)){
        DEBUG_PRINTLN("GPS POWER: Failed to parse power status");
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

// 2021-02-18 (10 chars) 23:01:08 (8 chars)
bool GPSTracker::parseTimeAndDate(const char * CGNSINF, char * date, uint8_t dateSize, char * time, uint8_t timeSize){
    wdt_reset();
    
    //+CGNSINF: 1,1,20210218230108.000,50.468168,13.421028,385.700,3.39,209.6,1,,5.0,5.1,1.0,,9,3,,,27,,
    char timeStampText[TRACKER_BUFFER_SHORT];
    memset(timeStampText, 0, TRACKER_BUFFER_SHORT);

    // Parse timestamp text from CGNSINF sequence
    if (!parseGPSValue(CGNSINF, 2, timeStampText, TRACKER_BUFFER_SHORT)){
        DEBUG_PRINTLN("GPS TimeAndDate: Failed to timestamp");
        return false;
    }

    // Check timetext length nad buffer sizes
    if (strlen(timeStampText) <= 13 || timeSize <= 8 || dateSize <= 10){
        DEBUG_PRINTLN("GPS TimeAndDate: Buffer too small");
        return false;
    }

    // Check if timestamp text is only numbers
    for (uint8_t i = 0; i < 14; i++){
        if (timeStampText[i] < 48 || timeStampText[i] > 57){
            DEBUG_PRINTLN("GPS TimeAndDate: Timestamp text corrupted");
            return false;
        }
    }

    // Creating time and date string
    // 20210218 230 1 0 8
    // 01234567 8910111213
    date[0] = timeStampText[0];
    date[1] = timeStampText[1];
    date[2] = timeStampText[2];
    date[3] = timeStampText[3];
    date[4] = '-';
    date[5] = timeStampText[4];
    date[6] = timeStampText[5];
    date[7] = '-';
    date[8] = timeStampText[6];
    date[9] = timeStampText[7];
    date[10] = '\0';

    time[0] = timeStampText[8];
    time[1] = timeStampText[9];
    time[2] = ':';
    time[3] = timeStampText[10];
    time[4] = timeStampText[11];
    time[5] = ':';
    time[6] = timeStampText[12];
    time[7] = timeStampText[13]; 
    time[8] = '\0';
    return true;
}

