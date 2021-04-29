#pragma once

#define SIM_RX		10	///< GSM Module RXD
#define SIM_TX	    11	///< GSM Module TXD

#define SIM_PWR		9	///< GSM Module D9
#define SIM_RST		8	///< GSM Module RST
#define SIM_DTR     6   ///< GSM Module DTR
#define RST_BTN     21  ///< GSM Module RESET BUTTON

#define BATTERY_VPIN A5
#define BATTERY_VMAX 4.2 ///< Max voltage of battery at full charge
#define BATTERY_VMIN 3.0 ///< Min voltage of battery at 0 %

#define SIM808_BAUDRATE 4800    ///< Control the baudrate use to communicate with the SIM808 module
