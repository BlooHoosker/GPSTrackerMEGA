#pragma once

#include <Arduino.h>
#include <Stream.h>
#include <avr/wdt.h>

//#define DEBUG
#ifdef DEBUG
    #define DEBUG_PRINTLN(s) Serial.println(s);
    #define DEBUG_PRINT(s) Serial.print(s);
    #define USART0_DISABLE
    #define PRINTSTATUS tracker.printStatus();
#else
    #define DEBUG_PRINTLN(s)
    #define DEBUG_PRINT(s)
    #define USART0_DISABLE power_usart0_disable();
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

    /**
     * @brief GPSTracker object constructor
     * @param SIM_RESET_PIN pin on arduino connected to SIM808 reset pin
     * @param SIM_PWR_PIN pin on arduino connected to SIM808 power pin
     * @param RST_BTN_PIN pin on arduino connected to user reset button
     * @param BATTERY_PIN pin on arduino connected to battery catode for voltage mesurement
     * */
    GPSTracker(uint8_t SIM_RESET_PIN, uint8_t SIM_PWR_PIN, /*uint8_t SIM_DTR_PIN,*/ uint8_t RST_BTN_PIN, uint8_t BATTERY_PIN);

    ~GPSTracker();

    /**
     * @brief Start method which powers and inicializes GSM module
     * 
     * @param serial initialized software or hardware serial object
     * @return true on success
     * @return false on failure
     */
    bool start(Stream &serial);

    /**
     * @brief Public recieve method. Waits on AT command sequence from serial
     * 
     */
    void receive();

    /**
     * @brief Checks battery percentage, if bellow 20 % sends warning SMS
     */
    void checkBatteryPercentage();

    /**
     * @brief Checks button status, if pressed for 3 seconds resets device to factory settings
     * 
     */
    void checkButton();

    /**
     * @brief Checks if GSM module is active and responding correctly, otherwise attempts to restart module
     * 
     */
    void checkGSM();

    /**
     * @brief Enables built in LED on arduino
     * 
     */
    void builtInLedOn();

    /**
     * @brief Disables built in LED on arduino
     * 
     */
    void builtInLedOff();

    /**
     * @brief Built in LED starts blinking quickly
     * 
     */
    void builtInLedFastBlink();

    /**
     * @brief Updates internal location variables with data from GPS system
     * 
     */
    void updateGPSLocation();

    // void gsmSleep();

    // void gsmWake();

    // bool enableGsmSleepMode();

    // bool disableGsmSleepMode();

private:

    Stream *_serialPort;
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


    // ============================================================================
    // General locator control
    // ============================================================================

    /**
    * @brief Powers up module
    * 
    * @return true if module powers up or is already powered
    * @return false if module fails to power up
    */
    bool powerOn();

    /**
     * @brief Checks if module is powered
     * 
     * @return true if module is powered
     * @return false if module is off
     */
    bool powered();

    /**
     * @brief Initializes module and applies configuration. After initialization, the module is ready for communication
     * 
     * @return true on success
     * @return false if something failed to be configured
     */
    bool init();

    /**
     * @brief Resets module using the reset pin
     */
    void reset();

    /**
     * @brief User induced restart. Retarts device using watchdog and sets restart flag into EEPROM
     * 
     */
    void restart();

    /**
     * @brief Updates all locator's internal status variables
     * 
     * @return true on success
     * @return false if fails to update variable
     */
    bool updateStatus();

    /**
     * @brief Resets EEPROM to factory state
     */
    void resetEEPROM();


    // ============================================================================
    // GSM module configuration
    // ============================================================================

    /**
     * @brief Sets SMS message mode
     * 
     * @param text true for Text mode, false for PDU mode
     * @return true on success
     * @return false on failure
     */
    bool setSMSMessageMode(bool text);

    /**
     * @brief Sets echoing mode of sent AT commands
     * 
     * @param echo true to enable echo mode, false to disable echo mode
     * @return true on success
     * @return false on failure
     */
    bool setEchoMode(bool echo);

    /**
     * @brief Set the Sms storage as GSM module itself
     * 
     * @return true on success
     * @return false on failure
     */
    bool setSmsStorage();


    // ============================================================================
    // Serial communication
    // ============================================================================

    /**
     * @brief Reads an AT command from serial
     * 
     * @param buffer buffer into which received command is received. It is erased at the start. Cannot be null.
     * @param size size of buffer.
     * @param timeout timeout how long it should wait.
     * @return number of characters received. 
     */
    uint16_t readAT(char *buffer, size_t size, uint16_t *timeout);

    /**
     * @brief Waits until requested command is received. 
     * @remark Discards any other previously received commands. If new SMS notification is recieved, stores it into buffer.
     * 
     * @param command AT command it should wait for. Cannot be null.
     * @param timeout Timeout how long it should wait
     * @return true if requested command is received
     * @return false if requested command is not received
     */
    bool waitFor(const char *command, uint16_t timeout = TRACKER_DEFAULT_TIMEOUT);

    /**
     * @brief Waits until requested command is received. Puts received command into buffer
     * @remark Discards any other previously received commands. If new SMS notification is recieved, stores it into buffer.
     * 
     * @param buffer buffer into which received command is received. It is erased at the start. Cannot be null.
     * @param bufferSize size of buffer.
     * @param command AT command it should wait for. Cannot be null.
     * @param timeout Timeout how long it should wait
     * @return true if requested command is received
     * @return false if requested command is not received
     */
    bool waitFor(char *buffer, size_t bufferSize, uint16_t timeout, const char *command);


    // ============================================================================
    // AT command sequence operations
    // ============================================================================

    /**
     * @brief Waits until receives an AT sequence from module.
     * 
     * @param buffer buffer into which received command is received. It is erased at the start. Cannot be null.
     * @param size size of buffer.
     * @param timeout timeout how long it should wait.
     * @return number of characters received. If it's <= 2 it returns zero. 
     */
    uint16_t receiveAT(char *buffer, size_t size, uint16_t timeout);

    /**
     * @brief Processes a recieved AT command. Basically start for whole reply process
     * 
     * @param ATCommand full AT command sequence. Cannot be null.
     */
    void processAT(const char *ATCommand);
    
    /**
     * @brief Sends formatted AT command with "AT" at the start and "\r\n" at the end
     * 
     * @param AT command to send. Cannot be null.
     */
    void sendAT(const char *AT = "");
    
    /**
     * @brief Decodes which AT command was received, supports only commands i required.
     * 
     * @param ATCommand command to decode. Cannot be null.
     * @return 0 if CMTI was received indicating new SMS message
     * @return -1 if unknown command is received 
     */
    int8_t decodeAT(const char *ATCommand);

    /**
     * @brief Comparing received AT command with requested AT command. Used to compare first part of the AT command without parameters
     * 
     * @param receivedAT AT command sequence received from module.
     * @param expectedAT AT command sequence we are expecting.
     * @return true if AT commands match
     * @return false if AT commands don't match
     */
    bool compareAT(const char *receivedAT, const char *expectedAT);


    // ============================================================================
    // SMS operations
    // ============================================================================

    /**
     * @brief Parses SMS index of location in module's storage.
     * 
     * @param ATCMTI must be full received CMTI AT command sequence! For example "+CMTI: "SM",2\r\n"
     * @return returns SMS index on success, -1 on failure.
     */
    int8_t parseSMSIndex(const char *ATCMTI);

    /**
     * @brief Parses sender's phone number from +CMGR sequence.
     * 
     * @param ATCMGR CMGR sequence. Must be in correct format which is for example: "+CMGR: "REC READ","+420123456789","","20/12/20,01:59:44+04"
     * @param numberBuffer buffer where phone number will be stores. Cannot be null.
     * @param numberBufferSize size of phone buffer.
     * @return true on success.
     * @return false on failure.
     */
    bool parseSMSPhoneNumber(const char *ATCMGR, char *numberBuffer, size_t numberBufferSize);
    
    /**
     * @brief Procedure for processing received SMS after CMTI sequence is received. And selecting operation based on received user command.
     * 
     * @param ATCMTI CMTI sequence indicating new message is received. Cannot be null.
     */
    void ATSMS(const char *ATCMTI);

    /**
     * @brief Decodes which command does SMS text contain. 
     * 
     * @param SMSTEXT SMS user command. Must be just without \r\n
     * @return 0 for LOCATION
     * @return 1 for STATUS
     * @return 2 for GPS POWER
     * @return 3 for SET MASTER
     * @return 4 for RESET MASTER
     * @return 5 for RESTART
     * @return 6 for GOOGLE
     * @return 7 for OPENSTREETMAP
     * @return -1 for unknown command
     */
    int decodeSMSText(const char *SMSTEXT);

    /**
     * @brief Removes last two \r\n characters
     * 
     * @param SMSText SMS text received from GSM module.
     */
    void extractSMSText(char *SMSText);

    /**
     * @brief Sends text as an SMS to given phone number.
     * 
     * @param text Text to send. Maximum length of text is 160 characters.
     * @param phoneNumber Phone number that should be with +XXX.
     * @return true on success.
     * @return false on failure.
     */
    bool sendSMS(const char *text, const char *phoneNumber);

    /**
     * @brief Reads SMS in module's storage.
     * 
     * @param smsIndex Index of SMS to be read from module's storage
     * @param text buffer for SMS text. Cannot be null.
     * @param phoneNumber buffer for phone number of requested SMS. Cannot be null.
     * @param textSize size of text buffer.
     * @param phoneNumSize size of phone number buffer.
     * @return true on success.
     * @return false on failure or internal timeout.
     */
    bool readSMS(uint8_t smsIndex, char *text, char *phoneNumber, size_t textSize, size_t phoneNumSize);

    /**
     * @brief Deletes all SMS messages stored in module's storage
     * 
     * @return true on success.
     * @return false on failure.
     */
    bool deleteAllSMS();

        /**
     * @brief Deletes SMS message stored in module's storage
     * @param index index of SMS message to be deleted from module's storage
     * 
     * @return true on success.
     * @return false on failure.
     */
    bool deleteSMS(uint8_t index);

    /**
     * @brief Support method for sending SMS. Basically waits until module sends "> " 
     * 
     * @param timeout how long it should wait.
     * @return true on success.
     * @return false on failure.
     */
    bool waitForPromt(uint16_t timeout);


    // ============================================================================
    // User commands operations
    // ============================================================================

    /**
     * @brief User command procedure that enables/disables GPS
     * @remark On success sends message to user with GPS power status. On failure sends error message to user.
     * 
     * @param SMSGPSPower SMS text command from user. Format: "GPS POWER: 0/1"
     */
    void userGPSPower(const char *SMSGPSPower);

    /**
     * @brief Sends user current location as a google maps link. In case of disabled GPS or no fix, sends last known location.
     * 
     */
    void userLocation();
    
    /**
     * @brief Sends user current device status information.
     * 
     */
    void userStatus();

    /**
     * @brief Sets master phone number for the device. After this the device only responds to commands from this number.
     * 
     * @param phoneNumber phone number to be set as master.
     */
    void userSetMasterNumber(const char *phoneNumber);

    /**
     * @brief Resets master phone number. After this the device will react to commands from all phone numbers.
     * 
     */
    void userResetMasterNumber();

    /**
     * @brief Selects location link format.
     * 
     * @param linkSel type of link to be selected. 0 for Google, 1 for OpenStreetMap
     */
    void userSetMapLinkSrc(const uint8_t linkSel);


    // ============================================================================
    // Master number operations
    // ============================================================================

    /**
     * @brief Sets master phone number for the device. Stores it in EEPROM and sets master number flag in EEPROM.
     * 
     * @param phoneNumber phone number to be set as master.
     * @return true on success.
     * @return false on failure.
     */
    bool setMasterNumber(const char *phoneNumber);

    /**
     * @brief Resets master number flag in EEPROM
     * 
     */
    void resetMasterNumber();

    /**
     * @brief Loads master number from EEPROM if master flag is set. If CRC doesn't match number will not be loaded.
     * 
     */
    void getMasterNumber();


    // ============================================================================
    // GPS control
    // ============================================================================

    /**
     * @brief Gets current data from GPS and updates internal variables.
     * 
     * @return true on success.
     * @return false on failure.
     */
    bool updateGPSStatusInfo();

    /**
     * @brief Powers up/down GPS.
     * 
     * @param on on = 1, off = 0
     * @return true on success.
     * @return false on failure.
     */
    bool powerGPS(bool on);

    /**
     * @brief Gets GNSS info sequence from GPS.
     * 
     * @param buffer buffer for GNSS info sequence. Cannot be null.
     * @param bufferSize size of buffer.
     * @return true on success.
     * @return false on failure.
     */
    bool getGPSInfo(char *buffer, size_t bufferSize);

    /**
     * @brief Returns GPS power status.
     * 
     * @return 1 = GPS is on
     * @return 0 = GPS is off
     * @return -1 = Error
     */
    int8_t getGPSPowerStatus();

    /**
     * @brief Gets GPS position and stores it into latitude and longtitude
     * @remark Both buffers have to be same size and long enough to store value.
     * 
     * @param latitude Buffer for latitude. Cannot be null.
     * @param longitude Buffer for longitude. Cannot be null.
     * @param size size of both buffers.
     * @return true on success
     * @return false on error/GPS is off/No fix
     */
    bool getGPSPosition(char *latitude, char *longitude, size_t size);

    /**
     * @brief Gets GPS fix status.
     * 
     * @return 0 = No fix
    *  @return 1 = Fix aquired
    *  @return -1 = Error
     */
    int8_t getGPSFixStatus();

    /**
     * @brief Parses value from GPS info sequence on valuePosition place (starting from 0).
     * 
     * @param CGNSINF GNSS info sequence. Cannot be null.
     * @param valuePosition position of value requested.
     * @param value buffer for parsed value. Cannot be null.
     * @param valueSize size of buffere for value.
     * @return true on success.
     * @return false on failure.
     */
    bool parseGPSValue(const char *CGNSINF, uint8_t valuePosition, char *value, uint8_t valueSize);

    /**
     * @brief Parses GPS power status from GNSS (CGNSINF) sequence.
     * 
     * @param CGNSINF 
     * @return 1 = GPS on
     * @return 0 = GPS off
     * @return -1 = wrong format
     */
    int8_t parseGPSPowerStatus(const char *CGNSINF);

    /**
     * @brief Parses GPS position from GNSS (CGNSINF) sequence.
     * 
     * @param CGNSINF CGNS info sequence. Cannot be null.
     * @param latitude Buffer for latitude. Cannot be null.
     * @param longitude Buffer for longitude. Cannot be null.
     * @param bufferSize size of both buffers.
     * @return true on success.
     * @return false on wrong format.
     */
    bool parseGPSPosition(const char *CGNSINF, char *latitude, char *longitude, size_t bufferSize);
    
    /**
     * @brief Parses GPS fix status from GNSS (CGNSINF) sequence.
     * 
     * @param CGNSINF 
     * @return 1 = Fix aquired
     * @return 0 = No fix
     * @return -1 = wrong format.
     */
    int8_t parseGPSFixStatus(const char *CGNSINF);

    /**
     * @brief Parses time and date from CGNS info sequence.
     * 
     * @param CGNSINF CGNS info sequence. Cannot be null.
     * @param date buffer for date. Cannot be null.
     * @param dateSize size of date buffer.
     * @param time buffer for time. Cannot be null.
     * @param timeSize size of time buffer.
     * @return true on success.
     * @return false on wrong format.
     */
    bool parseTimeAndDate(const char *CGNSINF, char *date, uint8_t dateSize, char *time, uint8_t timeSize);


    // ============================================================================
    // Location map link operations
    // ============================================================================

    /**
     * @brief Sets map link type for location.
     * 
     * @param linkSel 0 for Google, 1 for OpenStreetMaps. 
     */
    void setMapLinkSrc(const uint8_t linkSel);

    /**
     * @brief Gets map link type for location from EEPROM and saves it into internal variable.
     * 
     */
    void getMapLinkSrc();


    // ============================================================================
    // Battery status reading
    // ============================================================================

    /**
     * @brief Get the battery percentage.
     * 
     * @return battery percentage.
     */
    uint8_t getBatteryPercentage();


    // ============================================================================
    // SMS Queue buffer operations
    // ============================================================================

    /**
     * @brief Inserts a new message notification into queue buffer.
     * @remark If queue is full. It won;t be inserted.
     * 
     * @param CMTI new message notification AT sequence. Cannot be null.
     */
    void queueInsert(const char *CMTI);

    /**
     * @brief Extracts a new message notification from queue buffer
     * 
     * @param CMTI buffer for new message notification AT sequence. Cannot be null.
     * @param CMTIBufferSize size of buffer.
     * @return true on success.
     * @return false if queue is empty.
     */
    bool queueExtract(char *CMTI, uint8_t CMTIBufferSize);
};