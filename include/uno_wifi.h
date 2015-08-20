/*
 * uno_wifi.h
 *
 *  Created on: Apr 22, 2015
 *      Author: root
 */

#ifndef INCLUDE_UNO_WIFI_H_
#define INCLUDE_UNO_WIFI_H_

//功能切换的状态由回调函数通知:
typedef void (*UNO_WIFI_CB_T)(void *caller, int uno_wifi_state, int err_no);

typedef enum uno_wifi_status{
	UNO_WIFI_IDLE,
	UNO_WIFI_STA_ING,
	UNO_WIFI_DHCP_ING,
	UNO_WIFI_AP,
	UNO_WIFI_STA,
}UNO_WIFI_STATUS_T;

//wifi初始化函数。需要传入回调函数地址。
int uno_wifi_init(UNO_WIFI_CB_T wifi_cb_func, void *caller);

//注销函数。
void uno_wifi_uninit(void);

// 请求建立ap函数。ap模式仅支持Open和wpa2加密。建ap完成由回调函数告知。
// ap_ssid最长32个字符，ap_pwd最长128个字符。ap_channel取值范围为1~13。
// 如果使用无密码的open ap，ap_pwd请传入NULL。
// 返回值	意义
// 0		正常
// -1		正在连接路由器中，忽略建立ap请求
// -2		上一次wifi请求未完成，忽略建立ap请求
// -3		ap_ssid不合法，忽略建立ap请求
// -4		ap_pwd不合法，忽略建立ap请求
int uno_wifi_switch_ap(char* ap_ssid, char* ap_pwd, int ap_channel);

// 请求连接到路由器函数。连路由结果由回调给出。可能返回用户名密码错误(回调中err_no为1)，或者DHCP超时(回调中err_no为2)。
// router_ssid最长32个字符，router_pwd最长128个字符。
// 函数返回值意义类似 uno_wifi_switch_ap
int uno_wifi_switch_sta(char* router_ssid, char* router_pwd);

#endif /* INCLUDE_UNO_WIFI_H_ */
