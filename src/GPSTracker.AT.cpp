#include <GPSTracker.h>

void GPSTracker::sendAT(const char * AT /* = "" */){
    char buffer[TRACKER_BUFFER_MEDIUM];
    memset(buffer, 0, TRACKER_BUFFER_MEDIUM);

    sprintf(buffer, "AT%s\r\n", AT);
    _serialPort->write(buffer);
}

uint16_t GPSTracker::receiveAT(char * buffer, size_t size, uint16_t timeout){
    
    uint16_t length = 0;

    // Waits until more than 2 characters are received. 
    // That is to filter out blank commands
    while (length <= 2 && timeout){
        length = readAT(buffer, size, &timeout);
    }
    
    if (length <= 2) return 0;

    return length;
}

int8_t GPSTracker::decodeAT(const char * ATCommand){

    char incomingSMS[] = "+CMTI";
    if (compareAT(ATCommand, incomingSMS)){
        return 0;
    }

    return -1;
}

bool  GPSTracker::compareAT(const char * receivedAT, const char * expectedAT){

    // if received AT command is longer than expected we can safely compare
    if (strlen(receivedAT) >= strlen(expectedAT)){
        if (!strncmp(receivedAT, expectedAT, strlen(expectedAT))){
            return true;
        }
    }
    
    return false;
}

void GPSTracker::processAT(const char * ATCommand){

    switch (decodeAT(ATCommand)){
        case 0:
            Serial.println("PROCESS AT: New SMS");
            ATSMS(ATCommand);
            break;
        
        default:
            Serial.println("PROCESS AT: No command");
            break;
    }

}