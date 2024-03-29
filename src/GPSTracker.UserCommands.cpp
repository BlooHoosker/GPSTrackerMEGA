#include <GPSTracker.h>

void GPSTracker::userGPSPower(const char * SMSGPSPower){
    wdt_reset();

    unsigned int state = 0;

    // Parses requested GPS power state from text command
    if(sscanf(SMSGPSPower, "GPS POWER: %u", &state) != 1){
        DEBUG_PRINTLN("USER POWER: Invalid command format");
        sendSMS("INVALID COMMAND FORMAT\nCOMMAND SHOULD BE: \"GPS 0/1\"", _phoneNumber);
        return;
    }

    // Check if state is valid
    if (state != 0 && state != 1){
        DEBUG_PRINTLN("USER POWER: Invalid command format2");
        sendSMS("INVALID COMMAND FORMAT\nCOMMAND SHOULD BE: \"GPS [0,1]\"", _phoneNumber);
        return;
    }

    // Sets GPS power state
    if( !powerGPS(state) ) {
        DEBUG_PRINTLN("USER POWER: GPS failed to power up");
        sendSMS("FAILED SET GPS POWER", _phoneNumber);
        return;
    };

    DEBUG_PRINTLN("USER POWER: Sending reply");
    // Sends reply
    if (_gpsPowerStatus){
        if (!sendSMS("GPS POWERED UP", _phoneNumber)){
            DEBUG_PRINTLN("USER POWER: Failed to send sms");
        }
    } else {
        if (!sendSMS("GPS POWERED DOWN", _phoneNumber)){
            DEBUG_PRINTLN("USER POWER: Failed to send sms");
        }
    }
}

void GPSTracker::userLocation(){
    wdt_reset();

    char text[TRACKER_BUFFER_LARGE];
    char link[TRACKER_BUFFER_LARGE];
    char timeStamp[TRACKER_BUFFER_SHORT];
    char position[TRACKER_BUFFER_MEDIUM];
    
    memset(text, 0, TRACKER_BUFFER_LARGE);
    memset(link, 0, TRACKER_BUFFER_LARGE);
    memset(timeStamp, 0, TRACKER_BUFFER_SHORT);
    memset(position, 0, TRACKER_BUFFER_MEDIUM);

    // Update internal GPS info variables
    if (!updateGPSStatusInfo()){
        sendSMS("ERROR UPDATING GPS INFO", _phoneNumber);
        return;
    }

    // Date and time
    if (strlen(_date) && strlen(_time)){
        sprintf(timeStamp, "TIMESTAMP:\r\n%s %s UTC", _date, _time);
    } else {
        sprintf(timeStamp, "TIMESTAMP:\r\nNO DATA");
    }

    // Link and position coordinates
    if (strlen(_latitude) && strlen(_longitude)){

        sprintf(position, "POSITION:\r\n%s, %s",_latitude, _longitude);

        if (_mapLinkSrc == 0){
            sprintf(link, "LOCATION:\r\nhttp://maps.google.com/maps?q=%s,%s",_latitude, _longitude);
        } else {
            sprintf(link, "LOCATION:\r\nhttps://www.openstreetmap.org/?mlat=%s&mlon=%s",_latitude, _longitude);
        }
    } else {
        strcpy(position, "POSITION: NO DATA");
        strcpy(link, "LOCATION: NO DATA");
    }

    // Creates final text message 
    sprintf(text, "%s\r\n%s\r\n%s", timeStamp, position, link);

    DEBUG_PRINTLN("USER LOCATION: Sending location data");
    // Sends reply
    if(!sendSMS(text, _phoneNumber)){
        DEBUG_PRINTLN("USER LOCATION: failed to send");
    }

}

void GPSTracker::userStatus(){
    wdt_reset();

    char text[TRACKER_BUFFER_LARGE];
    memset(text, 0, TRACKER_BUFFER_LARGE);
    
    char power[TRACKER_BUFFER_SHORT];
    char fix[TRACKER_BUFFER_SHORT];
    char master[TRACKER_BUFFER_SHORT];
    char link[TRACKER_BUFFER_SHORT];
    char battery[TRACKER_BUFFER_SHORT];

    memset(master, 0, TRACKER_BUFFER_SHORT); 
    memset(power, 0, TRACKER_BUFFER_SHORT);
    memset(fix, 0, TRACKER_BUFFER_SHORT);
    memset(link, 0, TRACKER_BUFFER_SHORT);
    memset(battery, 0, TRACKER_BUFFER_SHORT);

    // Updating internal status variables
    if (!updateStatus()){
        sendSMS("ERROR UPDATING STATUS INFO", _phoneNumber);
        return;
    }

    // GPS power status
    if (_gpsPowerStatus){
        strcpy(power, "GPS POWER: ON");
    } else {
        strcpy(power, "GPS POWER: OFF");
    }

    // GPS Fix status
    if (_gpsFixStatus){
        strcpy(fix, "GPS FIX: FIX AQUIRED");
    } else {
        strcpy(fix, "GPS FIX: NO FIX"); 
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

    sprintf(text, "%s\r\n%s\r\n%s\r\n%s\r\n%s", power, fix, master, link, battery);

    DEBUG_PRINTLN("USER STATUS: Sending status info");
    if(!sendSMS(text, _phoneNumber)){
        DEBUG_PRINTLN("USER STATUS: failed to send");
    }
}

void GPSTracker::userSetMasterNumber(const char * phoneNumber){
    wdt_reset();

    if (setMasterNumber(phoneNumber)){
        DEBUG_PRINTLN("USER MASTER SET: Sending reply");
        sendSMS("MASTER NUMBER SET", _phoneNumber);
    } else {
        DEBUG_PRINTLN("USER MASTER SET: Failed");
        sendSMS("FAILED TO SET MASTER NUMBER", _phoneNumber);
    }
}

void GPSTracker::userResetMasterNumber(){
    wdt_reset();

    resetMasterNumber();
    DEBUG_PRINTLN("USER MASTER RESET: Sending reply");
    sendSMS("MASTER NUMBER RESET", _phoneNumber);
}

void GPSTracker::userSetMapLinkSrc(const uint8_t linkSel){
    wdt_reset();
    
    if (linkSel != 0 && linkSel != 1) return;

    setMapLinkSrc(linkSel);

    DEBUG_PRINTLN("USER LINK SET: Sending reply");
    if (linkSel == 0){
        sendSMS("GOOGLE MAP LINK SELECTED", _phoneNumber);
    } else {
        sendSMS("OPENSTREETMAP LINK SELECTED", _phoneNumber);
    }
}





