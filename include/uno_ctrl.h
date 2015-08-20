/*
 * uno_ctrl.h
 *
 *  Created on: Apr 2, 2015
 *      Author: root
 */

#ifndef DRIVER_UNO_CTRL_H_
#define DRIVER_UNO_CTRL_H_


typedef enum uno_evt{
	UNOEVENT_BTN_PRESSHOLD,		// 3 seconds at least
	UNOEVENT_BTN_PRESSED,
	UNOEVENT_IMU_MOTION,		//camera 发生移动
    UNOEVENT_IMU_STILL,			//camera 恢复静止状态
	UNOEVENT_IMU_ACC,			//返回 ACC values
	UNOEVENT_IMU_GYR,
	UNOEVENT_IMU_MAG,
	UNOEVENT_IMU_EXCEPTION_UNKNOWN_PACKET,
	UNOEVENT_IMU_EXCEPTION_REQUEST_TIMEOUT,
	UNOEVENT_IMU_EXCEPTION_UNPLUG
}UNO_EVENT;

typedef struct UNO_IMU_DATA{
	float x;
	float y;
	float z;
}UNO_IMU_DATA_T;

typedef void (*UNO_EVT_CB)(void *caller, UNO_EVENT _event, void * param);

/*
 * return negative value means some module init error, each bit represents one module:
 *  bit: 	3		2		1		0
 *  module:	thread	led		imu		button
 *	for example:
 *  -7 : btn, imu, led, init error
 *  -5 : btn, led, init error
 *  -4 : led, init error
 *  -2 : imu init error
 */
int uno_ctrl_init(UNO_EVT_CB ctrl_cb, void * caller);
void uno_ctrl_uninit(void);

#define DEBUG_TEST 0
#define DEBUG(msg, args...) do {if (DEBUG_TEST) fprintf(stderr, "[%s] " msg "", __FUNCTION__ , ##args );} while (0)

#endif /* DRIVER_UNO_CTRL_H_ */
