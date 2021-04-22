#include <GPSTracker.h>

bool GPSTracker::setSMSMessageMode(bool text){
    if (text){
        sendAT("+CMGF=1");
    } else {
        sendAT("+CMGF=0");
    }

    return waitFor("OK");
}

bool GPSTracker::setSmsStorage(){
    sendAT("+CPMS=\"ME\",\"ME\",\"ME\"");
	if(!waitFor("OK")){
		return false;
	}
    return true;
}

bool GPSTracker::deleteAllSMS(){

    sendAT("+CMGDA=\"DEL ALL\"");
    if (!waitFor("OK", TRACKER_SECOND*25)){
        return false;
    }

    return true;
}

int8_t GPSTracker::parseSMSIndex(const char * ATCMTI){
    int index = 0;
    // Parse index from CMTI command
    if( sscanf(ATCMTI, "+CMTI: \"ME\",%d\r\n", &index) != 1){
        return -1;
    }
    return index;
}

int GPSTracker::decodeSMSText(const char *SMSTEXT){

    if (!strcmp(SMSTEXT, "LOCATION")) return 0;

    if (!strcmp(SMSTEXT, "STATUS")) return 1;
    
    if (compareAT(SMSTEXT, "GPS POWER:")) return 2;

    if (!strcmp(SMSTEXT, "SET MASTER")) return 3;

    if (!strcmp(SMSTEXT, "RESET MASTER")) return 4;

    if (!strcmp(SMSTEXT, "RESTART")) return 5;

    if (!strcmp(SMSTEXT, "GOOGLE")) return 6;

    if (!strcmp(SMSTEXT, "OPENSTREETMAP")) return 7;

    return -1;
}

void GPSTracker::ATSMS(const char * ATCMTI){

    char text[TRACKER_BUFFER_LARGE];
    char phoneNumber[TRACKER_BUFFER_SHORT];

    // Get SMS index in storage
    int8_t index = parseSMSIndex(ATCMTI);
    if (index < 0){
        Serial.println("ATSMS: Failed to parse index");
        return;
    }

    // Read SMS from module storage at index
    if (!readSMS(index, text, phoneNumber, TRACKER_BUFFER_LARGE, TRACKER_BUFFER_SHORT)){
        Serial.println("ATSMS: Failed to read SMS");
        return;
    }

    // If master number is set
    if (_masterNumberSet){
        Serial.println("ATSMS: Master set");
        // Checks if master number and number of received SMS match
        if (strcmp(phoneNumber, _phoneNumber)) {
            Serial.println("ATSMS: Not a master number");
            return;
        }
    } else {
        Serial.println("ATSMS: Master not set");
        strcpy(_phoneNumber, phoneNumber);
    }

    // Get which command SMS text contains
    switch(decodeSMSText(text)){
        case 0: //LOCATION
            Serial.println("ATSMS: LOCATION");
            userLocation();
            break;
        case 1: // STATUS
            Serial.println("ATSMS: STATUS");
            userStatus();
            break;
        case 2: // GPS ON/OFF
            Serial.println("ATSMS: GPS POWER");
            userGPSPower(text);
            break;
        case 3: // SET MASTER NUMBER
            Serial.println("ATSMS: SET MASTER");
            userSetMasterNumber(phoneNumber);
            break;
        case 4: // SET MASTER NUMBER
            Serial.println("ATSMS: RESET MASTER");
            userResetMasterNumber();
            break;
        case 5: // RESTART
            Serial.println("ATSMS: RESTART");
            restart();
            break;
        case 6:
            Serial.println("ATSMS: GOOGLE");
            userSetMapLinkSrc(0);
            break;
        case 7:
            Serial.println("ATSMS: OPENSTREETMAP");
            userSetMapLinkSrc(1);
            break;
        default: // Unknown command
            sendSMS("INVALID COMMAND", _phoneNumber);
            Serial.println("ATSMS: UNKNOWN");
            break;
    }

    // Deletes all SMS messages to keep free space
    deleteAllSMS();
}

void GPSTracker::extractSMSText(char * SMSText){
    uint16_t length = strlen(SMSText);
    bool endOfText = false;
    for (uint16_t i = 0; i < length; i++){
        if (SMSText[i] == '\r' || SMSText[i] == '\n') endOfText = true;
        if (endOfText) SMSText[i] = '\0';
    }
}

bool GPSTracker::sendSMS(const char * text, const char * phoneNumber){

    char buffer[TRACKER_BUFFER_LARGE];

    // Checking length, max SMS length is 160
    if (strlen(text) > 160){
        Serial.println("SEND SMS: SMS text is too long");
        return false;
    }

    // Creating AT command sequence
    memset(buffer, 0, TRACKER_BUFFER_LARGE);
    if (sprintf(buffer, "+CMGS=\"%s\"", phoneNumber) <= 0){
        Serial.println("SEND SMS: Failed to create CMGS sequence");
        return false;
    }

    // Send command and wait for "> " promt
    // Timeout 60s as per documentation
    sendAT(buffer);
    if(!waitForPromt(60L*TRACKER_SECOND)){
        Serial.println("SEND SMS: Failed to receive SMS text promt");
        return false;
    }

    // Write SMS text ending with ctrl+z character
    _serialPort->write(text);
    _serialPort->write((char)0x1A);
    
    // Waiting for confirmation sequence
    if(!waitFor("+CMGS", 60L*TRACKER_SECOND)){
        Serial.println("SEND SMS: Failed to receive SMS text confirmation");
        return false;
    }

    // while(receiveAT(buffer, TRACKER_BUFFER_SIZE, 60L*TRACKER_SECOND)){ 
    //     Serial.print(buffer);

    //     if (compareAT(buffer, "+CMGS")){
    //         break;
    //     }
    // }

    // if (!compareAT(buffer, "+CMGS")){
    //         Serial.println("SEND SMS: Failed to receive SMS text confirmation");
    //         return false;
    // }
    
    if (!waitFor("OK")){
        Serial.println("SEND SMS: Failed to receive OK");
        return false;
    }

    return true;
}

bool GPSTracker::waitForPromt(uint16_t timeout){

    bool dartChar = false;
    char c = 0;

    while(timeout){
        while(_serialPort->available()){
            c = _serialPort->read();
            if (dartChar){
                if( c != ' '){
                    return false;
                } else {
                    return true;
                }
            }
            if (c == '>'){
                dartChar = true;
            }
        }
        delay(1);
        timeout--;
    }

    return false;
}

bool GPSTracker::readSMS(uint8_t smsIndex, char * text, char * phoneNumber, size_t textSize, size_t phoneNumSize){

    memset(text, 0, TRACKER_BUFFER_LARGE);
    memset(phoneNumber, 0, TRACKER_BUFFER_SHORT);

    if (sprintf(text, "+CMGR=%d", smsIndex) <= 0){
        return false;
    }

    // Send AT to request SMS at index AT+CMGR=index
    sendAT(text);

    // Wait for CMGR reply
    if (!waitFor(text, TRACKER_BUFFER_LARGE, 5*TRACKER_SECOND, "+CMGR")){
        Serial.println("READ SMS: Failed to receive SMS AT sequence");
        return false;
    }

    // Parsing phone number from CMGR
    if (!parseSMSPhoneNumber(text, phoneNumber, TRACKER_BUFFER_SHORT)){
        Serial.println("READ SMS: Failed to parse phone number");
        return false;
    }
      
    // Waiting for actual SMS text
    if (!receiveAT(text, TRACKER_BUFFER_LARGE, TRACKER_DEFAULT_TIMEOUT)){
        Serial.println("READ SMS: Failed to receive sms text");
        return false;
    }

    if (!waitFor("OK")){
        Serial.println("READ SMS: Failed to receive OK");
        return false;
    }

    // Removes \r\n from the end of the SMS text
    extractSMSText(text);

    return true;
}

bool GPSTracker::parseSMSPhoneNumber(const char * ATCMGR, char * numberBuffer, size_t numberBufferSize){

    uint16_t commandLength = strlen(ATCMGR);
    char delimiters[] = ",\"\"";

    uint8_t index = 0;
    uint8_t numberIndex = 0;

    if (!numberBufferSize) return false;

    // +CMGR: "REC READ","+420123456789","","20/12/20,01:59:44+04"
    // Phone number starts after first ',' in the command string surrounded by " "
    // Extract number starting " and ending "
    for (uint8_t i = 0; i < 3; i++){
        // Iterate through command string characters 
        for (; index < commandLength; index++){

            // Additional check, if second ',' is reached before parsing is complete
            // means the format is incorrect
            if (i > 0 && (ATCMGR[index] == delimiters[0])) return false;

            // Exit when delimiter character is reached
            if (ATCMGR[index] == delimiters[i]) break;

            // If last delimiter is expected, copy number into buffer
            if (i == 2){
                // If length of number exceeds length of its buffer, returns false
                if(numberIndex >= numberBufferSize)  return false;
                numberBuffer[numberIndex] = ATCMGR[index];
                numberIndex++;
            }
        }
        index++;
    }
    
    // If end of command string is reached, format of command string must be incorrect
    // In every circumstance command contains additional information/characters after phone number
    if (index >= commandLength) return false;

    // Adding \0 at the end
    // If length of number exceeds length of its buffer, returns false
    numberIndex++;
    if(numberIndex >= numberBufferSize)  return false;
    numberBuffer[numberIndex] = '\0';
    
    return true; 
}

