#include <GPSTracker.h>

void GPSTracker::userGPSPower(const char * SMSGPSPower, const char * phoneNumber){

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
        if (!sendSMS("GPS POWERED UP", phoneNumber)){
            Serial.println("Failed to send sms");
        }
    } else {
        if (!sendSMS("GPS POWERED DOWN", phoneNumber)){
            Serial.println("Failed to send sms");
        }
    }
}

