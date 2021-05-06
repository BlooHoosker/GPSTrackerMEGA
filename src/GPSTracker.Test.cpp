#include <GPSTracker.h>

void GPSTracker::test(){

	wdt_enable(WDTO_2S);
	wdt_reset();
	wdt_enable(WDTO_8S);
	while(1);

	// char buffer[TRACKER_BUFFER_LARGE];
	// memset(buffer, 0, TRACKER_BUFFER_LARGE);

    // char time[TRACKER_BUFFER_TIME];
	// char date[TRACKER_BUFFER_DATE];

    // DEBUG_PRINTLN("========== TESTING TIME AND DATE PARSING ==========");
	// if (parseTimeAndDate("+CGNSINF: 1,1,20210218230108.000,50.468168,13.421028,385.700,3.39,209.6,1,,5.0,5.1,1.0,,9,3,,,27,,", time, TRACKER_BUFFER_SHORT, date, TRACKER_BUFFER_SHORT)){
	// 	DEBUG_PRINTLN("TIME AND DATE TEST SUCCESS");
    //     DEBUG_PRINT("TIME: ");
	// 	DEBUG_PRINTLN(time);
	// 	DEBUG_PRINT("DATE: ");
	// 	DEBUG_PRINTLN(date);
	// } else {
    //     DEBUG_PRINTLN("TIME AND DATE TEST FAILED");
    // }

	// if (parseTimeAndDate("+CGNSINF: 1,1,20.000,50.468168,13.421028,385.700,3.39,209.6,1,,5.0,5.1,1.0,,9,3,,,27,,", time, TRACKER_BUFFER_SHORT, date, TRACKER_BUFFER_SHORT)){
	// 	DEBUG_PRINTLN("TIME AND DATE TEST FAILED");
	// } else {
    //     DEBUG_PRINTLN("TIME AND DATE TEST SUCCESS");
    // }

	// if (parseTimeAndDate("+CGNSINF: 1,1,2021absc8230108.000,50.468168,13.421028,385.700,3.39,209.6,1,,5.0,5.1,1.0,,9,3,,,27,,", time, TRACKER_BUFFER_SHORT, date, TRACKER_BUFFER_SHORT)){
    //     DEBUG_PRINTLN("TIME AND DATE TEST FAILED");
	// } else {
    //     DEBUG_PRINTLN("TIME AND DATE TEST SUCCESS");
    // }



	// while(1);
	// // DEBUG_PRINTLN("recieving");
	// // while(1){
	// // 	if (receiveAT(trackerReceiveBuffer, TRACKER_BUFFER_SHORT, 5*TRACKER_SECOND)){
    // // 		DEBUG_PRINT(trackerReceiveBuffer);
	// // 	}
	// // }
}