#include <GPSTracker.h>

void GPSTracker::userGPSPower(const char * SMSGPSPower){

    unsigned int status = 0;

    if(sscanf(SMSGPSPower, "GPS POWER: %u", &status) != 1){
        Serial.println("USER POWER: Invalid command format");
        sendSMS("INVALID COMMAND FORMAT\nCOMMAND SHOULD BE: \"GPS 0/1\"", _phoneNumber);
        return;
    }

    if (status != 0 && status != 1){
        Serial.println("USER POWER: Invalid command format2");
        sendSMS("INVALID COMMAND FORMAT\nCOMMAND SHOULD BE: \"GPS [0,1]\"", _phoneNumber);
        return;
    }

    // TODO check if this works
    if( !powerGPS(status) ) {
        Serial.println("USER POWER: GPS failed to power up");
        sendSMS("FAILED SET GPS POWER", _phoneNumber);
        return;
    };

    Serial.println("USER POWER: Sending reply");
    delay(1000);
    if (status){
        // if (!sendSMS("GPS POWERED UP", _phoneNumber)){
        //     Serial.println("USER POWER: Failed to send sms");
        // }
    } else {
        if (!sendSMS("GPS POWERED DOWN", _phoneNumber)){
            Serial.println("USER POWER: Failed to send sms");
        }
    }
}

void GPSTracker::userLocation(){

    char link[TRACKER_BUFFER_LARGE];

    memset(link, 0, TRACKER_BUFFER_LARGE);

    if (!updateGPSStatusInfo()){
        sendSMS("ERROR GETTING GPS INFO", _phoneNumber);
        return;
    }

    if (_fixStatus){
        if (strlen(_latitude) && strlen(_longitude)){
            if (_mapLinkSrc == 0){
                sprintf(link, "LOCATION:\r\nhttp://maps.google.com/maps?q=%s,%s",_latitude, _longitude);
            } else {
                sprintf(link, "LOCATION:\r\nhttps://www.openstreetmap.org/?mlat=%s&mlon=%s",_latitude, _longitude);
            }
        } else {
            strcpy(link, "LOCATION: NO DATA");
        }
    } else {
        if (strlen(_latitude) && strlen(_longitude)){
            if (_mapLinkSrc == 0){
                sprintf(link, "LAST LOCATION:\r\nhttp://maps.google.com/maps?q=%s,%s",_latitude, _longitude);
            } else {
                sprintf(link, "LAST LOCATION:\r\nhttps://www.openstreetmap.org/?mlat=%s&mlon=%s",_latitude, _longitude);
            }
            
        } else {
            strcpy(link, "LAST LOCATION: NO DATA");
        }
    }

    Serial.println("USER LOCATION: Sending location data");
    if(!sendSMS(link, _phoneNumber)){
        Serial.println("USER LOCATION: failed to send");
    }

}

void GPSTracker::userStatus(){

    char text[TRACKER_BUFFER_LARGE];
    char position[TRACKER_BUFFER_MEDIUM];

    memset(text, 0, TRACKER_BUFFER_LARGE);
    memset(position, 0, TRACKER_BUFFER_MEDIUM);

    
    char power[TRACKER_BUFFER_SHORT];
    char fix[TRACKER_BUFFER_SHORT];
    char master[TRACKER_BUFFER_SHORT];
    char link[TRACKER_BUFFER_SHORT];
    char battery[TRACKER_BUFFER_SHORT];
    char timeStamp[TRACKER_BUFFER_SHORT];

    memset(master, 0, TRACKER_BUFFER_SHORT); 
    memset(power, 0, TRACKER_BUFFER_SHORT);
    memset(fix, 0, TRACKER_BUFFER_SHORT);
    memset(link, 0, TRACKER_BUFFER_SHORT);
    memset(battery, 0, TRACKER_BUFFER_SHORT);
    memset(timeStamp, 0, TRACKER_BUFFER_SHORT);

    // Updating internal status variables
    if (!updateGPSStatusInfo()){
        sendSMS("ERROR GETTING GPS INFO", _phoneNumber);
        return;
    }

    // GPS power status
    if (_powerStatus){
        strcpy(power, "GPS POWER: ON");
    } else {
        strcpy(power, "GPS POWER: OFF");
    }

    // GPS Fix status
    if (_fixStatus){
        strcpy(fix, "GPS FIX: FIX AQUIRED");
    } else {
        strcpy(fix, "GPS FIX: NO FIX"); 
    }

    // Position coordinates
    if (strlen(_latitude) && strlen(_longitude)){
        sprintf(position, "POSITION:\n %s, %s",_latitude, _longitude);
    } else {
        strcpy(position, "POSITION: NO DATA");
    }

    // Date and time
    if (strlen(_date) && strlen(_time)){
        sprintf(timeStamp, "TIMESTAMP: %s %s UTC", _date, _time);
    } else {
        sprintf(timeStamp, "TIMESTAMP: NO DATA");
    }

    // Master number status
    if (_masterNumberSet){
        strcpy(master, "MASTER NUM: SET");
    } else {
        strcpy(master, "MASTER NUM: NOT SET");
    }

    // Map link selection
    if (_mapLinkSrc){
        strcpy(link, "MAP LINK: OPENSTREETMAP");
    } else {
        strcpy(link, "MAP LINK: GOOGLE");
    }

    // Battery status
    sprintf(battery, "BATTERY: %d%%", _batteryPercentage);

    sprintf(text, "%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n%s", power, fix, timeStamp, position, master, link, battery);

    Serial.println("USER STATUS: Sending status info");
    if(!sendSMS(text, _phoneNumber)){
        Serial.println("USER STATUS: failed to send");
    }
}

void GPSTracker::userSetMasterNumber(const char * phoneNumber){
    if (setMasterNumber(phoneNumber)){
        Serial.println("USER MASTER SET: Sending reply");
        sendSMS("MASTER NUMBER SET", _phoneNumber);
    } else {
        Serial.println("USER MASTER SET: Failed");
        sendSMS("FAILED TO SET MASTER NUMBER", _phoneNumber);
    }
}

void GPSTracker::userResetMasterNumber(){
    resetMasterNumber();
    Serial.println("USER MASTER RESET: Sending reply");
    sendSMS("MASTER NUMBER RESET", _phoneNumber);
}

void GPSTracker::userSetMapLinkSrc(const uint8_t linkSel){

    if (linkSel != 0 && linkSel != 1) return;

    setMapLinkSrc(linkSel);

    Serial.println("USER LINK SET: Sending reply");
    if (linkSel == 0){
        sendSMS("GOOGLE MAP LINK SELECTED", _phoneNumber);
    } else {
        sendSMS("OPENSTREETMAP LINK SELECTED", _phoneNumber);
    }
}





