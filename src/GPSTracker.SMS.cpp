#include <GPSTracker.h>

bool GPSTracker::setSMSMessageMode(bool text){
    if (text){
        sendAT("+CMGF=1");
    } else {
        sendAT("+CMGF=0");
    }

    return waitFor("OK");
}

int8_t GPSTracker::parseSMSIndex(const char * ATCMTI){
    int index = 0;
    // Parse index from CMTI command
    if( sscanf(ATCMTI, "+CMTI: \"ME\",%d\r\n", &index) != 1){
        return -1;
    }
    return index;
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

void GPSTracker::ATSMS(const char * ATCMTI){

    char buffer[TRACKER_BUFFER_SIZE];
    char phoneNumber[TRACKER_PHONE_NUBER_SIZE];
    memset(buffer, 0, TRACKER_BUFFER_SIZE);
    memset(phoneNumber, 0, TRACKER_PHONE_NUBER_SIZE);

    // Get SMS index in storage
    int8_t index = parseSMSIndex(ATCMTI);
    if (index < 0){
        Serial.println("Failed to parse index");
        return;
    }

    if (sprintf(buffer, "+CMGR=%d", index) <= 0){
        // TODO what to do in this case
    }

    // Send AT to request SMS at index AT+CMGR=index
    sendAT(buffer);

    // Wait for CMGR reply
    memset(buffer, 0, TRACKER_BUFFER_SIZE);
    Serial.println("Waiting for CMGR reply");
    if (!waitFor(buffer, TRACKER_BUFFER_SIZE, 5000, "+CMGR")){
        Serial.println("SMS didnt arrive");
        // TODO what to do in this case
    }

    // Parsing phone number from CMGR
    if (!parseSMSPhoneNumber(buffer, phoneNumber, TRACKER_PHONE_NUBER_SIZE)){
        Serial.println("Failed to parse phone number");
        // TODO what to do in this case
    }
      
    // Waiting for actual SMS text
    memset(buffer, 0, TRACKER_BUFFER_SIZE);
    Serial.println("Waiting for SMS text");
    if (!receiveAT(buffer, TRACKER_BUFFER_SIZE, TRACKER_DEFAULT_TIMEOUT)){
        Serial.println("Failed to receive sms text");
        // TODO what to do in this case
    }

    waitFor("OK");

    // TODO check master number

    // Removes \r\n from the end of the SMS text
    extractSMSText(buffer);

    // Get which command SMS text contains
    switch(decodeSMSText(buffer)){
        case 0: //LOCATION
            sendSMS("LOCATION", "+420732885552");
            break;
        case 1: // STATUS
            break;
        case 2: // GPS ON/OFF
            userGPSPower(buffer);
            break;
        default:
            // TODO Send error to user
            break;
    }

}

int GPSTracker::decodeSMSText(const char *SMSTEXT){

    if (!strcmp(SMSTEXT, "LOCATION")){
        Serial.println("Received LOCATION command");
        return 0;
    }

    if (!strcmp(SMSTEXT, "STATUS")){
        Serial.println("Received STATUS command");
        return 1;
    }

    Serial.println("Recieved unknown command");
    return -1;
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

    char buffer[TRACKER_BUFFER_SIZE];
    memset(buffer, 0, TRACKER_BUFFER_SIZE);

    Serial.println("Checking length");
    if (strlen(text) > 160){
        Serial.println("SMS text too long");
        return false;
    }

    sprintf(buffer, "+CMGS=\"%s\"", phoneNumber);

    //Serial.println(buffer);
    Serial.println("Sending SMS number");
    sendAT(buffer);
    Serial.println("Waiting promt..");
    if(!waitForPromt(60L*TRACKER_SECOND)){
        Serial.println("No promt");
        return false;
    }

    Serial.println("Promt recieved. Writing SMS..");
    _serialPort->write(text);
    _serialPort->write(0x1A);

    if(!waitFor("+CMGS", 60L*TRACKER_SECOND)){
        Serial.println("No confirmation response");
        return false;
    }

    Serial.println("Confirmation recieved");
    waitFor("OK");

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