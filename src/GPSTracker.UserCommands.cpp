#include <GPSTracker.h>

void GPSTracker::userGPSPower(const char * SMSGPSPower){

    unsigned int status = 0;

    if(sscanf(SMSGPSPower, "GPS POWER: %u", &status) != 1){
        Serial.println("Invalid command format");
        //sendSMS("INVALID COMMAND FORMAT\nCOMMAND SHOULD BE: \"GPS 0/1\"", phoneNum);
        return;
    }

    if (status != 0 && status != 1){
        Serial.println("Invalid command format2");
        //sendSMS("INVALID COMMAND FORMAT\nCOMMAND SHOULD BE: \"GPS 0/1\"", phoneNum);
        return;
    }

    // TODO check if this works
    if( !powerGPS(status) ) {
        Serial.println("GPS failed to power up");
        //sendSMS("FAILED SET GPS POWER", phoneNum);
        return;
    };

    Serial.println("GPS POWER: Sending reply");
    if (status){
        if (!sendSMS("GPS POWERED UP", _phoneNumber)){
            Serial.println("Failed to send sms");
        }
    } else {
        if (!sendSMS("GPS POWERED DOWN", _phoneNumber)){
            Serial.println("Failed to send sms");
        }
    }
}

void GPSTracker::userLocation(){

    char link[TRACKER_BUFFER_SIZE];

    memset(link, 0, TRACKER_BUFFER_SIZE);

    updateGPSStatusInfo();

    if (_fixStatus){
        if (strlen(_latitude) && strlen(_longitude)){
            sprintf(link, "LOCATION:\nhttp://maps.google.com/maps?q=%s,%s",_latitude, _longitude);
        } else {
            strcpy(link, "LOCATION: NO DATA");
        }
    } else {
        if (strlen(_latitude) && strlen(_longitude)){
            sprintf(link, "LAST LOCATION:\nhttp://maps.google.com/maps?q=%s,%s",_latitude, _longitude);
        } else {
            strcpy(link, "LAST LOCATION: NO DATA");
        }
    }

    Serial.println("LOCATION: Sending location data");
    if(!sendSMS(link, _phoneNumber)){
        Serial.println("LOCATION: failed to send");
    }

}

void GPSTracker::userStatus(){

    char text[TRACKER_BUFFER_SIZE];
    char position[TRACKER_BUFFER_SIZE];

    memset(position, 0, TRACKER_BUFFER_SIZE);
    memset(text, 0, TRACKER_BUFFER_SIZE);
    
    char power[TRACKER_PHONE_NUBER_SIZE];
    char fix[TRACKER_PHONE_NUBER_SIZE];
    char master[TRACKER_PHONE_NUBER_SIZE];

    memset(master, 0, TRACKER_PHONE_NUBER_SIZE); 
    memset(power, 0, TRACKER_PHONE_NUBER_SIZE);
    memset(fix, 0, TRACKER_PHONE_NUBER_SIZE);


    updateGPSStatusInfo();

    if (_powerStatus){
        strcpy(power, "GPS POWER: ON");
    } else {
        strcpy(power, "GPS POWER: OFF");
    }

    if (_fixStatus){
        strcpy(fix, "GPS FIX: FIX AQUIRED");

        if (strlen(_latitude) && strlen(_longitude)){
            sprintf(position, "POSITION: %s, %s",_latitude, _longitude);
        } else {
            strcpy(position, "POSITION: NO DATA");
        }
    } else {
        strcpy(fix, "GPS FIX: NO FIX"); 

        if (strlen(_latitude) && strlen(_longitude)){
            sprintf(position, "LAST POSITION:\n %s, %s",_latitude, _longitude);
        } else {
            strcpy(position, "LAST POSITION: NO DATA");
        }
    }

    if (_masterNumberSet){
        strcpy(master, "MASTER NUM: SET");
    } else {
        strcpy(master, "MASTER NUM: NOT SET");
    }

    sprintf(text, "%s\n%s\n%s\n%s", power, fix, position, master);

    Serial.println("STATUS: Sending status info");
    if(!sendSMS(text, _phoneNumber)){
        Serial.println("STATUS: failed to send");
    }
}

void GPSTracker::userSetMasterNumber(const char * phoneNumber){
    if (setMasterNumber(phoneNumber)){
        sendSMS("MASTER NUMBER SET", _phoneNumber);
    } else {
        //sendSMS("FAILED TO SET MASTER NUMBER", _phoneNumber);
    }
}

void GPSTracker::userResetMasterNumber(){
    resetMasterNumber();
    sendSMS("MASTER NUMBER RESET", _phoneNumber);
}








