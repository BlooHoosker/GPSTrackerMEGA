#include <GPSTracker.h>
#include <EEPROM.h>

char calculateCRC(const char* buffer){
    char result = 0;
    uint8_t length = strlen(buffer);

    for (uint8_t i = 0; i < length; i++){
        result ^= buffer[i];
    }

    return result;
}

bool GPSTracker::setMasterNumber(const char * phoneNumber){

    uint8_t length = strlen(phoneNumber);
    char crc = 0;

    // Check if length of number is longer than storage space
    // Check if phoneNumber is empty
    if (length+5U > EEPROM.length() || !length) return false;

    // Calculate crc from length + phone number
    crc = calculateCRC(phoneNumber);
    crc ^= (char) length;

    // Write crc, length, phone number to EEPROM
    EEPROM.update(CRC_ADDR, crc);
    EEPROM.update(LENGTH_ADDR, length);
    for (uint8_t i = PHONE_NUM_ADDR; i < length; i++){
        EEPROM.update(i, phoneNumber[i]);
    }

    // Phone number is already copied into _phoneNumber
    // Set master number flag to EEPROM
    EEPROM.update(MASTERSET_ADDR, 0xFF);
    _masterNumberSet = 1;

    Serial.print("Master number set: ");
    Serial.println(_phoneNumber);

    return true;
}

void GPSTracker::getMasterNumber(){

    char crc = 0;
    uint8_t length = 0;

    // Checking master number flag in EEPROM
    if (EEPROM[MASTERSET_ADDR] != 0xFF) return;

    memset(_phoneNumber, 0, TRACKER_PHONE_NUBER_SIZE);

    _masterNumberSet = 0;

    // Length is located at the address LENGTH_ADDR of EEPROM
    length = (uint8_t) EEPROM[LENGTH_ADDR];
    
    // Check if length of number is longer than buffer
    if (length >= TRACKER_PHONE_NUBER_SIZE) return;

    // Copy number from EEPROM
    for (uint8_t i = PHONE_NUM_ADDR; i < length; i++){
        _phoneNumber[i] = EEPROM[i];
    }

    // Calculate crc from length + phone number
    crc = calculateCRC(_phoneNumber);
    crc ^= (char) length;

    // Check if crc matches
    if(crc != EEPROM[CRC_ADDR]){
        Serial.println("CRC don't match");
        memset(_phoneNumber, 0, TRACKER_PHONE_NUBER_SIZE);
    } else {
        Serial.print("Master loaded: ");
        Serial.println(_phoneNumber);
        _masterNumberSet = 1;
    }
}

void GPSTracker::resetMasterNumber(){
    _masterNumberSet = 0;
    // Reset master number flag in EEPROM to 0
    EEPROM[MASTERSET_ADDR] = 0;
}