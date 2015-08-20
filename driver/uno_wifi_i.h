/*
 * uno_wifi.h
 *
 *  Created on: Apr 22, 2015
 *      Author: root
 */

#ifndef DRIVER_UNO_WIFI_I_H_
#define DRIVER_UNO_WIFI_I_H_

typedef enum uno_wifi_trigger_input{
	UNO_WIFI_SWITCH_NONE,
	UNO_WIFI_SWITCH_AP,
	UNO_WIFI_SWITCH_STA,
	UNO_WIFI_DHCP_TIMEOUT,
	UNO_WIFI_DHCP_COMPLETE,
	UNO_WIFI_STA_COMPLETE,
	UNO_WIFI_STA_FAIL,
}UNO_WIFI_TRIG_IN_T;

int uno_wifi_trigger(UNO_WIFI_TRIG_IN_T trig_in);

#endif /* DRIVER_UNO_WIFI_I_H_ */
