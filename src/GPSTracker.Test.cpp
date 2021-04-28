#include <GPSTracker.h>

void GPSTracker::test(){
	// char buffer[TRACKER_BUFFER_LARGE];
	// memset(buffer, 0, TRACKER_BUFFER_LARGE);

    // char time[TRACKER_BUFFER_TIME];
	// char date[TRACKER_BUFFER_DATE];

    // Serial.println("========== TESTING TIME AND DATE PARSING ==========");
	// if (parseTimeAndDate("+CGNSINF: 1,1,20210218230108.000,50.468168,13.421028,385.700,3.39,209.6,1,,5.0,5.1,1.0,,9,3,,,27,,", time, TRACKER_BUFFER_SHORT, date, TRACKER_BUFFER_SHORT)){
	// 	Serial.println("TIME AND DATE TEST SUCCESS");
    //     Serial.print("TIME: ");
	// 	Serial.println(time);
	// 	Serial.print("DATE: ");
	// 	Serial.println(date);
	// } else {
    //     Serial.println("TIME AND DATE TEST FAILED");
    // }

	// if (parseTimeAndDate("+CGNSINF: 1,1,20.000,50.468168,13.421028,385.700,3.39,209.6,1,,5.0,5.1,1.0,,9,3,,,27,,", time, TRACKER_BUFFER_SHORT, date, TRACKER_BUFFER_SHORT)){
	// 	Serial.println("TIME AND DATE TEST FAILED");
	// } else {
    //     Serial.println("TIME AND DATE TEST SUCCESS");
    // }

	// if (parseTimeAndDate("+CGNSINF: 1,1,2021absc8230108.000,50.468168,13.421028,385.700,3.39,209.6,1,,5.0,5.1,1.0,,9,3,,,27,,", time, TRACKER_BUFFER_SHORT, date, TRACKER_BUFFER_SHORT)){
    //     Serial.println("TIME AND DATE TEST FAILED");
	// } else {
    //     Serial.println("TIME AND DATE TEST SUCCESS");
    // }



	// while(1);
	// // Serial.println("recieving");
	// // while(1){
	// // 	if (receiveAT(trackerReceiveBuffer, TRACKER_BUFFER_SHORT, 5*TRACKER_SECOND)){
    // // 		Serial.print(trackerReceiveBuffer);
	// // 	}
	// // }
}