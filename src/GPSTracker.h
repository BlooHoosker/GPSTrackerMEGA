#pragma once

#include <Arduino.h>
#include <Stream.h>

#define DEBUG
#ifdef DEBUG
    #define DEBUG_PRINTLN(s) Serial.println(s);
    #define DEBUG_PRINT(s) Serial.print(s);
    #define USART0_DISABLE
    #define PRINTSTATUS tracker.printStatus();
#else
    #define DEBUG_PRINTLN(s)
    #define DEBUG_PRINT(s)
    #define USART0_DISABLE   power_usart0_disable();
    #define PRINTSTATUS 
#endif

#define TRACKER_BUFFER_LARGE 200
#define TRACKER_BUFFER_MEDIUM 64
#define TRACKER_BUFFER_SHORT 32

#define TRACKER_QUEUE_SIZE 4

#define TRACKER_BUFFER_DATE 15
#define TRACKER_BUFFER_TIME 15

#define TRACKER_DEFAULT_TIMEOUT 2000
#define TRACKER_SECOND 1000

#define RESTART_ADDR 0
#define LINK_SRC_ADDR 1
#define MASTERSET_ADDR 2
#define CRC_ADDR 3
#define LENGTH_ADDR 4 
#define PHONE_NUM_ADDR 5 

class GPSTracker
{
public:

    // Methods for testing
    void test();
    void printStatus();

    // Provide reset and power pin
    GPSTracker(uint8_t SIM_RESET_PIN, uint8_t SIM_PWR_PIN, /*uint8_t SIM_DTR_PIN,*/ uint8_t RST_BTN_PIN, uint8_t BATTERY_PIN);
    ~GPSTracker();
    
    /* 
    * Boot procedure into an initialized state
    * trackerSerial must be an already initialized SoftwareSerial/HardwareSerial
    * Returns true on success
    * Returns false on failure
    */ 
    bool start(Stream &serial);

    void receive();
    
    void processAT(const char *ATCommand);

    void checkBatteryPercentage();

    void checkButton();

    void checkGSM();

    void builtInLedOn();

    void builtInLedOff();

    void builtInLedFastBlink();

    void updateGPSLocation();

    // void gsmSleep();

    // void gsmWake();

    // bool enableGsmSleepMode();

    // bool disableGsmSleepMode();

private:

    Stream* _serialPort;
    uint8_t _resetPin;
    uint8_t _powerPin;
    //uint8_t _dtrPin;
    uint8_t _buttonPin;
    uint8_t _batteryPin;

    char _phoneNumber[TRACKER_BUFFER_SHORT];
    char _latitude[TRACKER_BUFFER_SHORT];
    char _longitude[TRACKER_BUFFER_SHORT];

    // 2021-02-18 (10 chars + \0) 23:01:08 (8 chars + \0)
    char _date[TRACKER_BUFFER_DATE];
    char _time[TRACKER_BUFFER_TIME];

    char _commandQueue[TRACKER_QUEUE_SIZE][TRACKER_BUFFER_SHORT];
    uint8_t _queueCommandNum;
    uint8_t _queueHead;
    uint8_t _queueTail;

    uint8_t _gpsPowerStatus;
    uint8_t _gpsFixStatus;
    uint8_t _masterNumberSet;
    uint8_t _mapLinkSrc;

    uint8_t _batteryPercentage;
    bool _batteryWarningSent;

    void queueInsert(const char * CMTI);

    bool queueExtract(char * CMTI, uint8_t CMTIBufferSize);

    void resetEEPROM();

    /* 
    * User induced restart
    * Retarts device using watchdog and sets restart flag into EEPROM
    */ 
    void restart();

    /*
    * Powers up module
    * Returns true if module powers up or is already powered
    * Returns false if module fails to power up
    */
    bool powerOn();

    /*
    * Checks if module is powered
    * Returns true if module is powered
    * Returns false if module is off
    */ 
    bool powered();

    /* 
    * Resets module using the reset pin
    */ 
    void reset();

    /*
    * Initializes module and applies configuration
    * After initialization, the module is ready for communication
    * Returns true on success
    * Returns false if something failed to be configured
    */
    bool init();

    /*
    * Sets SMS message mode
    * text = true for Text mode
    * text = false for PDU mode
    * Returns true if operation was successful, false if not
    */ 
    bool setSMSMessageMode(bool text);

    /*
    * Sets echoing mode of sent AT commands
    * echo = true to enable echo mode
    * echo = false to disable echo mode
    * Returns true if operation was successful
    * Returns false on failure
    */ 
    bool setEchoMode(bool echo);

    bool setSmsStorage();

    /*
    * Waits until requested command is received
    * Discards any other previously received commands
    * Returns true if requested command is received
    * Returns false if timeout runs out
    */
    bool waitFor(const char *command, uint16_t timeout = TRACKER_DEFAULT_TIMEOUT);

    /*
    * Same method but requested command is stored in buffer
    */
    bool waitFor(char * buffer, size_t bufferSize, uint16_t timeout, const char * command);

    /* 
    * Reads an AT command from serial
    * Returns number of characters read including \r\n
    * Returns 0 if \r\n sequence is not reached or timeout runs out
    */
    size_t readAT(char *buffer, size_t size, uint16_t *timeout);

    /*
    * Waits until receives an AT sequence from module
    * Received command is stored in buffer of size "size"
    * TODO figure out if timeout is necessary
    * Returns length of received command including \r\n
    */ 
    uint16_t receiveAT(char *buffer, size_t size, uint16_t timeout);

    /*
    * Sends formatted AT command with "AT" and "\r\n"
    */ 
    void sendAT(const char * AT = "");

    /*
    * Decodes which AT command was received, supports only commands we require
    * Returns:
    * 0 if CMTI was received indicating new SMS message
    * -1 if unknown command is received 
    */ 
    int8_t decodeAT(const char *ATCommand);

    /* 
    * Comparing received AT command with requested AT command
    * Used to compare first part of the AT command without parameters
    * receivedAT = AT received from module
    * expectedAT = AT we are expecting
    * Returns true if AT commands match
    * Returns false if they don't
    */ 
    bool compareAT(const char * receivedAT, const char * expectedAT);

    /*
    * Parses SMS index of location in module's storage
    * CMTI must be full received CMTI AT command! 
    * For example +CMTI: "SM",2\r\n
    * If parsing is successful, returns index
    * If parsing fails, returns -1
    */
    int8_t parseSMSIndex(const char *ATCMTI);

    /*
    * Parses sender's phone number from +CMGR sequence
    * Sequence must be in correct +CMGR format which is for example:
    * +CMGR: "REC READ","+420123456789","","20/12/20,01:59:44+04"
    * Returns true on success
    * Returns false on wrong format
    */
    bool parseSMSPhoneNumber(const char * ATCMGR, char * numberBuffer, size_t numberBufferSize);

    /*
    * Procedure for processing received SMS after CMTI sequence is received
    * indicating new message is received
    */
    void ATSMS(const char * ATCMTI);

    /* 
    * Decodes which command does SMS text contain
    * SMSTEXT must be just SMS text without \r\n
    * Returns:
    * 0 for LOCATION
    * 1 for STATUS
    * -1 if command is not recognized
    */ 
    int decodeSMSText(const char *SMSTEXT);

    //Removes last two \r\n characters
    void extractSMSText(char * SMSText);

    /* 
    * Sends text as an SMS to given phone number
    * Maximum length of text is 160 characters
    * Phone number should be with +XXX
    * Returns true if sending SMS was successful, false if not
    */
    bool sendSMS(const char * text, const char * phoneNumber);

    /*
    * Reads SMS in module's storage at smsIndex
    * Message text is stored in "text" which is of size "textSize"
    * Phone number of the message is stored in "phoneNumber" which is of size "phoneNumSize"
    * Returns true on success
    * Returns false on failure or timeout
    */ 
    bool readSMS(uint8_t smsIndex, char * text, char * phoneNumber, size_t textSize, size_t phoneNumSize);

    // Deletes all SMS messages stored in module's storage
    bool deleteAllSMS();

    // Deletes SMS message stored in module's storage at index
    bool deleteSMS(uint8_t index);

    /*
    * Support method for sending SMS
    * Basically waits until module sends "> " 
    * Then we can send SMS text 
    */ 
    bool waitForPromt(uint16_t timeout);

    /* 
    * User command routine that enables/disables GPS
    * SMSGPSPower: SMS command from user
    * Command format: "GPS POWER: 0/1"
    * On success sends message to user with GPS power status
    * On failure sends error message to user
    */ 
    void userGPSPower(const char * SMSGPSPower);

    /*
    * Sends user current location as a google maps link
    * In case of disabled GPS or no fix, sends last known location
    */ 
    void userLocation();

    /*
    * Sends user current device status information
    * In case of disabled GPS or no fix, sends last known position
    */
    void userStatus();

    /*
    * Sets master phone number for the device
    * After this the device only responds to commands from this number
    * phoneNumber: phone number to be set as master
    * Sends reply to user
    */  
    void userSetMasterNumber(const char * phoneNumber);

    /*
    * Resets master phone number
    * After this the device will react to commands from all numbers
    */ 
    void userResetMasterNumber();

    /*
    * Sets master phone number for the device
    * Stores it in EEPROM and sets master number flag in EEPROM
    * phoneNumber: phone number to be set as master
    */  
    bool setMasterNumber(const char * phoneNumber);

    // Resets master number flag in EEPROM
    void resetMasterNumber();

    /*
    * Loads master number from EEPROM if master flag is set
    * If CRC doesn't match number will not be loaded
    */ 
    void getMasterNumber();

    /*
    * Gets current data from GPS and updates internal variables
    */ 
    bool updateGPSStatusInfo();

    /*
    * Powers up/down GPS
    * on = 1
    * off = 0
    * Returns true on0 success
    */ 
    bool powerGPS(bool on);

    /*
    * Gets GNSS info sequence from GPS
    * Sequence gets written into buffer
    * Returns true on success, false on error/failure
    */  
    bool getGPSInfo(char * buffer, size_t bufferSize);

    /*
    * Returns GPS power status 
    * Returns:
    * 1 = GPS is on
    * 0 = GPS is off
    * -1 = Error
    */ 
    int8_t getGPSPowerStatus();

    /*
    * Gets GPS position and stores it into latitude and longtitude
    * Both buffers have to be same size and long enough to store value
    * Buffers are size of "size"
    * Returns true on sucess
    * Returns false if GPS is off/no GPS fix/error
    */ 
    bool getGPSPosition(char * latitude, char * longitude, size_t size);

    /*
    * Gets GPS fix status
    * Returns:
    * 0 = No fix
    * 1 = Fix aquired
    * -1 = Error
    */ 
    int8_t getGPSFixStatus();

    /*
    * Parses value from GPS info sequence on valuePosition place (starting from 0)
    * Buffer "value" is of size "valueSize" and should be large enough to fit falue
    */ 
    bool parseGPSValue(const char * CGNSINF, uint8_t valuePosition, char * value, uint8_t valueSize);

    /* 
    * Parses GPS power status from GNSS (CGNSINF) sequence
    * Returns power status, -1 on wrong format
    * 1 = GPS on
    * 0 = GPS off
    */  
    int8_t parseGPSPowerStatus(const char * CGNSINF);

    /*
    * Parses GPS position from GNSS (CGNSINF) sequence
    * Stores it into latitude and longtitude
    * Both buffers have to be same size and long enough to store value
    * Buffers are size of "size"
    * Returns true on success
    * Returns false on wrong format
    */ 
    bool parseGPSPosition(const char * CGNSINF, char * latitude, char * longitude, size_t bufferSize);

    /* 
    * Parses GPS fix status from GNSS (CGNSINF) sequence
    * Returns fix status, -1 on wrong format
    * 1 = Fix aquired
    * 0 = No fix
    */ 
    int8_t parseGPSFixStatus(const char * CGNSINF);

    //bool getTimeStamp(char * timeStampText, size_t timeStampTextSize);

    bool parseTimeAndDate(const char * CGNSINF, char * date, uint8_t dateSize, char * time, uint8_t timeSize);

    void setMapLinkSrc(const uint8_t linkSel);

    void getMapLinkSrc();

    void userSetMapLinkSrc(const uint8_t linkSel);

    uint8_t getBatteryPercentage();

    bool updateStatus();

};