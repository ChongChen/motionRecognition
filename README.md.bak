UnoSDK by
Topgear, Deepglint 2015

针对UNO板，提供获取IMU信息，按键信息，控制亮灯等功能。

源码集成方法：
1. 复制driver目录和include目录所有的文件。
2. 在你的工程中初始化的地方先#include "uno_api.h"，然后uno_ctrl_init，传入你自己写的回调函数，在该回调函数里进行相应处理（可以参考main.c中的uno_event_cb_func）。
3. 注意回调函数回调函数是在子线程中执行, 需要快速返回，建议不要做阻塞性操作，否则会影响imu检测和按键检测。

===================================================

函数使用方法：
0.默认约定：
如果不说明，函数返回0表示函数执行成功。
uno_ctrl_init返回值意义如下：
return negative value means some module init error, each bit represents one module:
bit:      3      2      1       0
module:	thread	led    imu    button
for example:
-7 : btn, imu, led, init error
-5 : btn, led, init error
-4 : led, init error
-2 : imu init error

1. IMU信息模块：

int uno_imu_request_acc(void);
该函数是异步方式请求ACC数据, ACC的结果通过回调函数返回(通过UNOEVENT_IMU_ACC事件）。

int uno_imu_get_acc(float *x, float *y, float *z);
该函数是以阻塞方式获得ACC数据。

void orientation_from_acc(float acc_x, float acc_y, float acc_z, float *acc_x2plumb_line, float *acc_y2plumb_line, float *acc_z2plumb_line);	// unit is radian

int uno_imu_set_motion_threshold_acc(float x, float y, float z);	// unit is g
int uno_imu_set_motion_threshold_gyr(float x, float y, float z);	// unit is radian/s
int uno_imu_set_still_threshold_acc(float var_x, float var_y, float var_z);	// parameters are variances, unit is g square
int uno_imu_set_still_threshold_gyr(float var_x, float var_y, float var_z);	// parameters are variances, unit is (radian/s) square
以上几个都是阈值设置函数，可以在初始化之后调用。UnoSDK已经初始化了推荐的阈值，如下：
motion_threshold_acc[3] = {0.2, 0.5, 0.2};
motion_threshold_gyr[3] = {0.5, 0.1, 0.5};
variance_threshold_acc[3] = {0.0001, 0.0001, 0.0001};
variance_threshold_gyr[3] = {0.0001, 0.0001, 0.0001};


2. 点灯模块：
void uno_led_set_predef_pattern(int led, LED_PATTERN_T pattern);
参数led可以是1或者2，对应led1和led2.
pattern可以是亮，灭，呼吸，快闪，一共四种。


void uno_led_pattern(int led, LED_STATE init_state, int compare, int period);
也可以自定义闪烁占空比，init_state是初始状态，ON或者OFF，compare值是占空比，period是周期，单位都是10ms。
period规定了闪烁的频率，compare规定了亮灯状态的比例。
灯亮：               _________________             _________________
灯灭：   ____________|                |____________|                |_____
	             	
计数器： 0         compare          period(0)     compare          period(0)

3. 按键模块：
只有回调接口，在回调事件中会有UNOEVENT_BTN_PRESSHOLD和UNOEVENT_BTN_PRESSED事件。按键3秒以上会触发UNOEVENT_BTN_PRESSHOLD事件。

4. wifi模块：
uno wifi实现了两个功能的封装：
a. 建立热点ap。
b. 连接wifi路由器。

功能切换的状态由回调函数通知。回调函数定义：
typedef void (*UNO_WIFI_CB_T)(void *caller, int uno_wifi_state, int err_no);

该模块提供以下接口
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

// 请求连接到路由器函数。连路由结果由回调给出。可能返回用户名密码错误，或者DHCP超时。
// router_ssid最长32个字符，router_pwd最长128个字符。
// 函数返回值意义类似 uno_wifi_switch_ap
int uno_wifi_switch_sta(char* router_ssid, char* router_pwd);

uno_wifi模块依赖:
1. wpa_supplicant/wpa_cli
2. dhclient

