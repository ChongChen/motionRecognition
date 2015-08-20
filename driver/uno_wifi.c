
#include <stdio.h>

#include "uno_wifi.h"
#include "uno_wifi_i.h"
#include "uno_ctrl.h"
#include "uno_api.h"
#include "uno_utility.h"

#include <string.h>

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <netdb.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <arpa/inet.h>

#define UNO_WIFI_SSID_MAX	32		//	max number of characters
#define UNO_WIFI_PWD_MAX	128

static pthread_t wifi_thread;
static void *uno_wifi_caller;
static UNO_WIFI_CB_T uno_wifi_cb_func;
static short uno_wifi_thread_running;

static short uno_wifi_state;	//machine state
static UNO_WIFI_TRIG_IN_T uno_wifi_request;		//user switch request
static int sta_ing_counter ;
static int dhcp_ing_counter;
static char uno_router_ssid[UNO_WIFI_SSID_MAX + 1];
static char uno_router_pwd[UNO_WIFI_PWD_MAX + 1];
static char uno_ap_ssid[UNO_WIFI_SSID_MAX + 1];
static char uno_ap_pwd[UNO_WIFI_PWD_MAX + 1];
static int uno_ap_channel;

int no_ip_addr(void);

#define UNO_WIFI_LED_IN 1

void uno_wifi_sta_ing(void){
#ifdef UNO_WIFI_LED_IN
	uno_led_set_predef_pattern(1, UNO_LED_PATTERN_OFF);
	uno_led_set_predef_pattern(2, UNO_LED_PATTERN_FLASH);
#endif
	if (uno_wifi_cb_func) uno_wifi_cb_func(uno_wifi_caller, UNO_WIFI_STA_ING, 0);
}
void uno_wifi_ap_ed(void){
#ifdef UNO_WIFI_LED_IN
	uno_led_set_predef_pattern(1, UNO_LED_PATTERN_BREATH);
	uno_led_set_predef_pattern(2, UNO_LED_PATTERN_OFF);
#endif
	if (uno_wifi_cb_func) uno_wifi_cb_func(uno_wifi_caller, UNO_WIFI_AP, 0);
}
void uno_wifi_sta_ed(void){
#ifdef UNO_WIFI_LED_IN
	uno_led_set_predef_pattern(1, UNO_LED_PATTERN_OFF);
	uno_led_set_predef_pattern(2, UNO_LED_PATTERN_BREATH);
#endif
	if (uno_wifi_cb_func) uno_wifi_cb_func(uno_wifi_caller, UNO_WIFI_STA, 0);
}


void *uno_wifi_thread(void *ptr) {
	int ret;
	ret = 0;
	ret = (int)ret;	//suppress warnings

	while (uno_wifi_thread_running) {
		//DEBUG("wifi statue: %d\n", uno_wifi_state);

		if (uno_wifi_state == UNO_WIFI_STA_ING) {	//check result every  500ms

			FILE *ret_fd;
			ret = system("wpa_cli status > /tmp/wpa_cli_status.log 2>&1");
			ret_fd = fopen("/tmp/wpa_cli_status.log", "r");

			usleep(500000);	//500ms
			sta_ing_counter += 500;

			if (file_contain(ret_fd, "COMPLETED")) {
				sta_ing_counter = 0;
				uno_wifi_trigger(UNO_WIFI_STA_COMPLETE);
			} else {
				if (sta_ing_counter >= 15000) {//15s timeout
					uno_wifi_trigger(UNO_WIFI_STA_FAIL);
				}
			}
		} else if(uno_wifi_state == UNO_WIFI_DHCP_ING){
			usleep(500000);	//500ms
			dhcp_ing_counter += 500;
			if(no_ip_addr()){
				//first connect to a router will take a long time
				if(dhcp_ing_counter > 30000){
					uno_wifi_trigger(UNO_WIFI_DHCP_TIMEOUT);
				}
			}else{
				dhcp_ing_counter = 0;
				uno_wifi_trigger(UNO_WIFI_DHCP_COMPLETE);
			}
		} else {
			 //handle use switch request
			if (uno_wifi_request == UNO_WIFI_SWITCH_AP
					|| uno_wifi_request == UNO_WIFI_SWITCH_STA) {
				uno_wifi_trigger(uno_wifi_request);
				uno_wifi_request = UNO_WIFI_SWITCH_NONE;//indicates that use request is handled already
			}
			usleep(100000);	//100ms
		}
	}
	pthread_exit(NULL);
}

int uno_wifi_trigger(UNO_WIFI_TRIG_IN_T trig_in){

	int ret;
	char cmd[1024];

	switch(trig_in){
	case UNO_WIFI_SWITCH_AP:
		DEBUG("UNO_WIFI_SWITCH_AP\n");
		//sh to switch to ap
		sprintf(cmd, "./switch_2_ap.sh %s %s %d > router_ap 2>&1\n",
				uno_ap_ssid, uno_ap_pwd, uno_ap_channel);
		//printf("cmd is %s\n", cmd);
		ret = system(cmd);
		// check return value to switch to idle with error or ap with success
		if(ret == 0){
			uno_wifi_state = UNO_WIFI_AP;
			uno_wifi_ap_ed();
		}/*else{
			printf("switch to ap failed\n");
			uno_wifi_status = UNO_WIFI_IDLE;
		}*/
		break;
	case UNO_WIFI_SWITCH_STA:
		DEBUG("UNO_WIFI_STA_ING\n");

		uno_wifi_sta_ing();
		// switch to router
		sprintf(cmd, "./switch_2_router.sh %s %s > router_log 2>&1\n",
				uno_router_ssid, uno_router_pwd);
		ret = system(cmd);
		sta_ing_counter = 0;
		dhcp_ing_counter = 0;
		uno_wifi_state = UNO_WIFI_STA_ING;
		break;
	case UNO_WIFI_STA_COMPLETE:
		uno_wifi_state =  UNO_WIFI_DHCP_ING;
		ret = system("dhclient wlan0 -nw"); // -nw means Become a daemon immediately
		break;
	case UNO_WIFI_DHCP_TIMEOUT:
		ret = system("dhclient -x");	//end dhclient thread created by dhclient wlan0 -nw. see "man dhclient" for details.
		ret = system("wpa_cli -iwlan0 disconnect");
		ret = system("wpa_cli terminate");
		uno_wifi_state = UNO_WIFI_IDLE;
		if (uno_wifi_cb_func) uno_wifi_cb_func(uno_wifi_caller, UNO_WIFI_IDLE, 2);
		break;
	case UNO_WIFI_DHCP_COMPLETE:
		uno_wifi_state = UNO_WIFI_STA;
		uno_wifi_sta_ed();
		break;
	case UNO_WIFI_STA_FAIL:
		printf("switch to router failed\n");
		ret = system("wpa_cli -iwlan0 disconnect");
		ret = system("wpa_cli terminate");
		uno_wifi_state = UNO_WIFI_IDLE;
		if (uno_wifi_cb_func) uno_wifi_cb_func(uno_wifi_caller, UNO_WIFI_IDLE, 1);
		break;
	default:
		break;
	}
	return 0;
}

int uno_wifi_switch_sta(char* router_ssid, char* router_pwd){

	if((router_ssid == NULL)||(strlen(router_ssid) > UNO_WIFI_SSID_MAX)){
		printf("router_ssid Illegal!\n");
		return -3;
	}else{
		strcpy(uno_router_ssid, router_ssid);
	}
	if((router_pwd == NULL)||( strlen(router_ssid) > UNO_WIFI_PWD_MAX)){
		printf("router password Illegal\n");
		return -4;
	}else{
		strcpy(uno_router_pwd, router_pwd);
	}
	DEBUG("sta ssid is %s\n", uno_router_ssid);
	DEBUG("sta pwd is %s\n", uno_router_pwd);


	if(uno_wifi_state == UNO_WIFI_STA_ING){
		uno_wifi_request = UNO_WIFI_SWITCH_NONE;
		return -1;
	}
	else if(uno_wifi_request != UNO_WIFI_SWITCH_NONE) { //xinghua: last user switch request is pending 
		return -2;
	}
	else{
		uno_wifi_request = UNO_WIFI_SWITCH_STA;
	}
	return 0;
}
/*
 * ap_ssid and ap_pwd must less then 256 bytes.
 * ap_channel must between 1~14
 */
int uno_wifi_switch_ap(char* ap_ssid, char* ap_pwd, int ap_channel){

	if((ap_ssid == NULL)||( strlen(ap_ssid) > UNO_WIFI_SSID_MAX)){
		printf("ap_ssid illegal\n");
		return -3;
	}else{
		strcpy(uno_ap_ssid, ap_ssid);
	}

	if(ap_pwd == NULL){
		strcpy(uno_ap_pwd, "NULL");	//open ap is allowed
	}else if( strlen(ap_pwd) <= UNO_WIFI_PWD_MAX){
		strcpy(uno_ap_pwd, ap_pwd);
	}else{
		printf(" ap_pwd illegal!\n");
		return -4;
	}
	if((ap_channel > 0)&&(ap_channel < 14)){
		uno_ap_channel = ap_channel;
	}else{
		uno_ap_channel = 1;
	}
	DEBUG("ap ssid is %s\n", uno_ap_ssid);
	DEBUG("ap pwd is %s\n", uno_ap_pwd);
	DEBUG("ap channel is %d\n", uno_ap_channel);

	if(uno_wifi_state == UNO_WIFI_STA_ING){
		uno_wifi_request = UNO_WIFI_SWITCH_NONE;
		return -1;
	}
	else if(uno_wifi_request != UNO_WIFI_SWITCH_NONE) { //xinghua: last user switch request is pending 
		return -2;
	}
	else{
		uno_wifi_request = UNO_WIFI_SWITCH_AP;
	}
	return 0;
}

int uno_wifi_init(UNO_WIFI_CB_T wifi_cb_func, void *caller){

	int ret;
	uno_wifi_state = UNO_WIFI_IDLE;
	uno_wifi_request = UNO_WIFI_SWITCH_NONE;
	uno_wifi_thread_running = 1;
	uno_wifi_cb_func = wifi_cb_func;
	uno_wifi_caller = caller;

	ret = pthread_create(&wifi_thread, NULL, uno_wifi_thread, NULL);
	if(ret != 0){
		printf("create wifi thread error: %d\n", ret);
		return -1;
	}else{
		DEBUG("Create uno wifi thread success!\n");
		return 0;
	}


}

void uno_wifi_uninit(void){
	uno_wifi_thread_running = 0;
	pthread_join(wifi_thread, NULL);

}

int no_ip_addr(void){
	struct ifaddrs *ifaddr, *ifa;
	int s;
	char host[NI_MAXHOST];
	int no_ip_addr = 1;

	if (getifaddrs(&ifaddr) == -1) {
		perror("getifaddrs");
		return 0;
	}

	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr == NULL)
			continue;

		s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host,
				NI_MAXHOST, NULL, 0, NI_NUMERICHOST);

		if (((strcmp(ifa->ifa_name, "wlan0") == 0))
				&& (ifa->ifa_addr->sa_family == AF_INET)) {
			if (s != 0) {
				printf("getnameinfo() failed: %s\n", gai_strerror(s));

			}
			no_ip_addr = 0;
			DEBUG("\tInterface : <%s>\n", ifa->ifa_name);
			DEBUG("\t  Address : <%s>\n", host);
		}

	}
	freeifaddrs(ifaddr);

	return no_ip_addr;
}
