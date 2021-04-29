#include <GPSTracker.h>

void GPSTracker::userGPSPower(const char * SMSGPSPower){

    unsigned int status = 0;

    if(sscanf(SMSGPSPower, "GPS POWER: %u", &status) != 1){
        DEBUG_PRINTLN("USER POWER: Invalid command format");
        sendSMS("INVALID COMMAND FORMAT\nCOMMAND SHOULD BE: \"GPS 0/1\"", _phoneNumber);
        return;
    }

    if (status != 0 && status != 1){
        DEBUG_PRINTLN("USER POWER: Invalid command format2");
        sendSMS("INVALID COMMAND FORMAT\nCOMMAND SHOULD BE: \"GPS [0,1]\"", _phoneNumber);
        return;
    }

    // TODO check if this works
    if( !powerGPS(status) ) {
        DEBUG_PRINTLN("USER POWER: GPS failed to power up");
        sendSMS("FAILED SET GPS POWER", _phoneNumber);
        return;
    };

    DEBUG_PRINTLN("USER POWER: Sending reply");
    delay(1000);
    if (status){
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

    char text[TRACKER_BUFFER_LARGE];
    char link[TRACKER_BUFFER_LARGE];
    char timeStamp[TRACKER_BUFFER_SHORT];
    char position[TRACKER_BUFFER_MEDIUM];
    
    memset(text, 0, TRACKER_BUFFER_LARGE);
    memset(link, 0, TRACKER_BUFFER_LARGE);
    memset(timeStamp, 0, TRACKER_BUFFER_SHORT);
    memset(position, 0, TRACKER_BUFFER_MEDIUM);

    if (!updateGPSStatusInfo()){
        sendSMS("ERROR UPDATING GPS INFO", _phoneNumber);
        return;
    }

    // Date and time
    if (strlen(_date) && strlen(_time)){
        sprintf(timeStamp, "TIMESTAMP:\n %s %s UTC", _date, _time);
    } else {
        sprintf(timeStamp, "TIMESTAMP:\n NO DATA");
    }

    // Link and position coordinates
    if (strlen(_latitude) && strlen(_longitude)){

        sprintf(position, "POSITION:\n %s, %s",_latitude, _longitude);

        if (_mapLinkSrc == 0){
            sprintf(link, "LOCATION:\r\nhttp://maps.google.com/maps?q=%s,%s",_latitude, _longitude);
        } else {
            sprintf(link, "LOCATION:\r\nhttps://www.openstreetmap.org/?mlat=%s&mlon=%s",_latitude, _longitude);
        }
    } else {
        strcpy(position, "POSITION: NO DATA");
        strcpy(link, "LOCATION: NO DATA");
    }

    sprintf(text, "%s\r\n%s\r\n%s", timeStamp, position, link);

    DEBUG_PRINTLN("USER LOCATION: Sending location data");
    if(!sendSMS(text, _phoneNumber)){
        DEBUG_PRINTLN("USER LOCATION: failed to send");
    }

}

void GPSTracker::userStatus(){

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
    if (setMasterNumber(phoneNumber)){
        DEBUG_PRINTLN("USER MASTER SET: Sending reply");
        sendSMS("MASTER NUMBER SET", _phoneNumber);
    } else {
        DEBUG_PRINTLN("USER MASTER SET: Failed");
        sendSMS("FAILED TO SET MASTER NUMBER", _phoneNumber);
    }
}

void GPSTracker::userResetMasterNumber(){
    resetMasterNumber();
    DEBUG_PRINTLN("USER MASTER RESET: Sending reply");
    sendSMS("MASTER NUMBER RESET", _phoneNumber);
}

void GPSTracker::userSetMapLinkSrc(const uint8_t linkSel){

    if (linkSel != 0 && linkSel != 1) return;

    setMapLinkSrc(linkSel);

    DEBUG_PRINTLN("USER LINK SET: Sending reply");
    if (linkSel == 0){
        sendSMS("GOOGLE MAP LINK SELECTED", _phoneNumber);
    } else {
        sendSMS("OPENSTREETMAP LINK SELECTED", _phoneNumber);
    }
}





