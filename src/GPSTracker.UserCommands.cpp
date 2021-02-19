#include <GPSTracker.h>

void GPSTracker::userGPSPower(const char * SMSGPSPower){

    unsigned int status = 0;

    if(sscanf(SMSGPSPower, "GPS %u", &status) != 1){
        //sendSMS("INVALID COMMAND FORMAT\nCOMMAND SHOULD BE: \"GPS 0/1\"", phoneNum);
        return;
    }

    if (status != 0 && status != 1){
        //sendSMS("INVALID COMMAND FORMAT\nCOMMAND SHOULD BE: \"GPS 0/1\"", phoneNum);
        return;
    }

    // TODO check if this works
    if( !powerGPS(status) ) {
        //sendSMS("FAILED SET GPS POWER", phoneNum);
        return;
    };

    if (status){
        //sendSMS("GPS POWERED UP", phoneNum);
    } else {
        //sendSMS("GPS POWERED DOWN", phoneNum);
    }
}

