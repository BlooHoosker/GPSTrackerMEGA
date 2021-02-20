#pragma once

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <Stream.h>

#define TRACKER_BUFFER_SIZE 128
#define TRACKER_PHONE_NUBER_SIZE 32
#define TRACKER_DEFAULT_TIMEOUT 2000
#define TRACKER_SECOND 1000

class GPSTracker
{
public:

    // Method for testing
    void test();

    // Provide reset and power pin
    GPSTracker(uint8_t SIM_RESET_PIN, uint8_t SIM_PWR_PIN);
    ~GPSTracker();
    
    /* 
    * Boot procedure into an initialized state
    * trackerSerial must be an already initialized SoftwareSerial/HardwareSerial
    * Returns true on success
    * Returns false on failure
    */ 
    bool start(Stream &serial);

    /*
    * Waits until receives an AT command from module
    * Received command is stored in buffer of size "size"
    * TODO figure out if timeout is necessary
    * Returns length of received command including \r\n
    */ 
    uint16_t receiveAT(char *buffer, size_t size, uint16_t timeout);
    
    void processAT(const char *ATCommand);

    /*
    * Parses sender's phone number from +CMGR sequence
    * Sequence must be in correct +CMGR format which is for example:
    * +CMGR: "REC READ","+420123456789","","20/12/20,01:59:44+04"
    * Returns true on success
    * Returns false on wrong format
    */
    bool parseSMSPhoneNumber(const char * ATCMGR, char * numberBuffer, size_t numberBufferSize);
private:

    Stream* _serialPort;
    uint8_t _resetPin;
    uint8_t _powerPin;
    char _phoneNum[TRACKER_PHONE_NUBER_SIZE];

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
    * Reads characters available from serial port until reaches stop char or timeout runs out
    * Returns number of characters read including \r\n
    */
    size_t readNext(char *buffer, size_t size, uint16_t *timeout, char stop);

    /*
    * Sets echoing mode of sent AT commands
    * echo = true to enable echo mode
    * echo = false to disable echo mode
    * Returns true if operation was successful
    * Returns false on failure
    */ 
    bool setEchoMode(bool echo);

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
    * Sets SMS message mode
    * text = true for Text mode
    * text = false for PDU mode
    * Returns true if operation was successful, false if not
    */ 
    bool setSMSMessageMode(bool text);

    /*
    * Parses SMS index of location in module's storage
    * CMTI must be full received CMTI AT command! 
    * For example +CMTI: "SM",2\r\n
    * If parsing is successful, returns index
    * If parsing fails, returns -1
    */
    int8_t parseSMSIndex(const char *ATCMTI);

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
    * Support method for sending SMS
    * Basically waits until module sends "> " 
    * Then we can send SMS text 
    */ 
    bool waitForPromt(uint16_t timeout);

    void userGPSPower(const char * SMSGPSCommand);

    void userLocation();

    void userStatus();

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

    int8_t getGPSAccuracy();

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

    int8_t parseGPSAccuracy(const char * CGNSINF);

    /* 
    * Parses GPS fix status from GNSS (CGNSINF) sequence
    * Returns fix status, -1 on wrong format
    * 1 = Fix aquired
    * 0 = No fix
    */ 
    int8_t parseGPSFixStatus(const char * CGNSINF);


};