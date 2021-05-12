#include <GPSTracker.h>

void GPSTracker::test(){

	char buffer[TRACKER_BUFFER_LARGE];
	memset(buffer, 0, TRACKER_BUFFER_LARGE);

	char rightFormatGPS[TRACKER_BUFFER_LARGE] = "+CGNSINF: 1,1,20210218230108.000,50.468168,13.421028,385.700,3.39,209.6,1,,5.0,5.1,1.0,,9,3,,,27,," ;
	char badFormatGPS[TRACKER_BUFFER_LARGE] = "+CGNSIasdasdNF: 1,1,20210218230108.000,50.468168,13.421028,385.700,3.39,209.6,1,,5.0,5.1,1.0,,9,3,,,27,,";
	char incompleteFormatGPS[TRACKER_BUFFER_LARGE] = "+CGNSINF: ";

    char time[TRACKER_BUFFER_SHORT];
	char date[TRACKER_BUFFER_SHORT];

    DEBUG_PRINTLN("========== TESTING TIME AND DATE PARSING ==========");
	if (parseTimeAndDate(rightFormatGPS, time, TRACKER_BUFFER_SHORT, date, TRACKER_BUFFER_SHORT)){
		DEBUG_PRINTLN("TIME AND DATE TEST SUCCESS");
        DEBUG_PRINT("TIME: ");
		DEBUG_PRINTLN(time);
		DEBUG_PRINT("DATE: ");
		DEBUG_PRINTLN(date);
	} else {
        DEBUG_PRINTLN("TIME AND DATE TEST FAILED");
    }

	if (parseTimeAndDate(badFormatGPS, time, TRACKER_BUFFER_SHORT, date, TRACKER_BUFFER_SHORT)){
		DEBUG_PRINTLN("TIME AND DATE TEST FAILED");
	} else {
        DEBUG_PRINTLN("TIME AND DATE TEST SUCCESS");
    }

	if (parseTimeAndDate(incompleteFormatGPS, time, TRACKER_BUFFER_SHORT, date, TRACKER_BUFFER_SHORT)){
		DEBUG_PRINTLN("TIME AND DATE TEST FAILED");
	} else {
        DEBUG_PRINTLN("TIME AND DATE TEST SUCCESS");
    }


	if (parseTimeAndDate("+CGNSINF: 1,1,20.000,50.468168,13.421028,385.700,3.39,209.6,1,,5.0,5.1,1.0,,9,3,,,27,,", time, TRACKER_BUFFER_SHORT, date, TRACKER_BUFFER_SHORT)){
		DEBUG_PRINTLN("TIME AND DATE TEST FAILED");
	} else {
        DEBUG_PRINTLN("TIME AND DATE TEST SUCCESS");
    }

	if (parseTimeAndDate("+CGNSINF: 1,1,2021absc8230108.000,50.468168,13.421028,385.700,3.39,209.6,1,,5.0,5.1,1.0,,9,3,,,27,,", time, TRACKER_BUFFER_SHORT, date, TRACKER_BUFFER_SHORT)){
        DEBUG_PRINTLN("TIME AND DATE TEST FAILED");
	} else {
        DEBUG_PRINTLN("TIME AND DATE TEST SUCCESS");
    }

	DEBUG_PRINTLN("========== TESTING GPS POSITION PARSING ==========");

	if (parseGPSPosition(rightFormatGPS, date, time, TRACKER_BUFFER_SHORT)){
		DEBUG_PRINTLN("GPS POSITION TEST SUCCESS");
        DEBUG_PRINT("LATITUDE: ");
		DEBUG_PRINTLN(date);
		DEBUG_PRINT("LONGITUDE: ");
		DEBUG_PRINTLN(time);
	} else {
        DEBUG_PRINTLN("GPS POSITION TEST FAILED");
    }

	if (parseGPSPosition(badFormatGPS, date, time, TRACKER_BUFFER_SHORT)){
		DEBUG_PRINTLN("GPS POSITION TEST FAILED");
	} else {
        DEBUG_PRINTLN("GPS POSITION TEST SUCCESS");
    }

	if (parseGPSPosition(incompleteFormatGPS, date, time, TRACKER_BUFFER_SHORT)){
		DEBUG_PRINTLN("GPS POSITION TEST FAILED");
	} else {
        DEBUG_PRINTLN("GPS POSITION TEST SUCCESS");
    }

	DEBUG_PRINTLN("========== TESTING GPS FIX PARSING ==========");

	if (parseGPSFixStatus(rightFormatGPS)){
		DEBUG_PRINTLN("GPS FIX TEST SUCCESS");
	} else {
        DEBUG_PRINTLN("GPS FIX TEST FAILED");
    }

	if (parseGPSFixStatus(badFormatGPS)){
		DEBUG_PRINTLN("GPS FIX TEST FAILED");
	} else {
        DEBUG_PRINTLN("GPS FIX TEST SUCCESS");
    }

	if (parseGPSFixStatus(incompleteFormatGPS)){
		DEBUG_PRINTLN("GPS FIX TEST FAILED");
	} else {
        DEBUG_PRINTLN("GPS FIX TEST SUCCESS");
    }

	DEBUG_PRINTLN("========== TESTING GPS POWER PARSING ==========");

	if (parseGPSPowerStatus(rightFormatGPS)){
		DEBUG_PRINTLN("GPS POWER TEST SUCCESS");
	} else {
        DEBUG_PRINTLN("GPS POWER TEST FAILED");
    }

	if (parseGPSPowerStatus(badFormatGPS)){
		DEBUG_PRINTLN("GPS POWER TEST FAILED");
	} else {
        DEBUG_PRINTLN("GPS POWER TEST SUCCESS");
    }

	if (parseGPSPowerStatus(incompleteFormatGPS)){
		DEBUG_PRINTLN("GPS POWER TEST FAILED");
	} else {
        DEBUG_PRINTLN("GPS POWER TEST SUCCESS");
    }

	DEBUG_PRINTLN("========== TESTING QUEUE BUFFER ==========");

    printQueue();

	queueInsert("Test string 1");
	queueInsert("Test string 2");

	printQueue();

	queueExtract(date, TRACKER_BUFFER_SHORT);
	DEBUG_PRINT("EXTRACTED: ");
	DEBUG_PRINTLN(date);

	printQueue();

	queueExtract(date, TRACKER_BUFFER_SHORT);
	DEBUG_PRINT("EXTRACTED: ");
	DEBUG_PRINTLN(date);

	printQueue();

	queueInsert("Test string 1");
	queueInsert("Test string 2");
	queueInsert("Test string 3");
	queueInsert("Test string 4");
	queueInsert("Test string 5");

	printQueue();

	queueExtract(date, TRACKER_BUFFER_SHORT);
	DEBUG_PRINT("EXTRACTED: ");
	DEBUG_PRINTLN(date);

	printQueue();

	queueExtract(date, TRACKER_BUFFER_SHORT);
	queueExtract(date, TRACKER_BUFFER_SHORT);
	queueExtract(date, TRACKER_BUFFER_SHORT);
	if (queueExtract(date, TRACKER_BUFFER_SHORT)){
		DEBUG_PRINTLN("QUEUE TEST FAILED");
	} else {
        DEBUG_PRINTLN("QUEUE TEST SUCCESS");
    }
	
	DEBUG_PRINTLN("========== TESTING PHONE NUMBER PARSING ==========");

	char rightNumber[TRACKER_BUFFER_LARGE] = "+CMGR: \"REC READ\",\"+420123456789\",\"\",\"20/12/20,01:59:44+04\"";
	char incompleteNumber[TRACKER_BUFFER_LARGE] = "+CMGR: \"REC READ\",\"+420123";

	if (parseSMSPhoneNumber(rightNumber, date, TRACKER_BUFFER_SHORT)){
		DEBUG_PRINTLN("PHONE NUMBER TEST SUCCESS");
		DEBUG_PRINT("PHONE NUMBER: ");
		DEBUG_PRINTLN(date);
	} else {
        DEBUG_PRINTLN("PHONE NUMBER TEST FAILED");
    }

	if (parseSMSPhoneNumber(incompleteNumber, date, TRACKER_BUFFER_SHORT)){
		DEBUG_PRINTLN("PHONE NUMBER TEST FAILED");
	} else {
        DEBUG_PRINTLN("PHONE NUMBER TEST SUCCESS");
    }

	while(1);
	// DEBUG_PRINTLN("recieving");
	// while(1){
	// 	if (receiveAT(trackerReceiveBuffer, TRACKER_BUFFER_SHORT, 5*TRACKER_SECOND)){
    // 		DEBUG_PRINT(trackerReceiveBuffer);
	// 	}
	// }
}