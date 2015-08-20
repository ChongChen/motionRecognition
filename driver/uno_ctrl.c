/*
 * uno_ctrl.c
 *
 *	make a timer thread to take care of btn, led and imu serial
 *  Created on: Apr 2, 2015
 *      Author: root
 */

#include "uno_ctrl.h"
#include "uno_btn.h"
#include "uno_imu.h"
#include "uno_imu_i.h"

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include "uno_led.h"
#include "uno_led_i.h"

static void *uno_ctrl_caller;
static pthread_t ctrl_thread;
static short uno_ctrl_thread_running;
static UNO_EVT_CB uno_event_cb_fn;
static UNO_IMU_DATA_T imu_data;

void *uno_ctrl_thread(void *ptr) {

	void *p;

	while (uno_ctrl_thread_running) {
		// btn
		switch (uno_btn_check()) {
		case UNO_BTN_PRESSED:
			if (uno_event_cb_fn)
				uno_event_cb_fn(uno_ctrl_caller, UNOEVENT_BTN_PRESSED, NULL);
			break;
		case UNO_BTN_PRESSHOLD:
			if (uno_event_cb_fn)
				uno_event_cb_fn(uno_ctrl_caller, UNOEVENT_BTN_PRESSHOLD, NULL);
			break;
		case UNO_BTN_NOT_PRESSED:
			break;
		default:
			break;
		}

		// IMU
		UNO_IMU_STATE ret = uno_imu_commands_events(&p);
		switch (ret) {
		case UNO_IMU_NO_EVENT:
			//printf(".");
			break;

		case UNO_IMU_EVENT_MOTION:
			if (uno_event_cb_fn)
				uno_event_cb_fn(uno_ctrl_caller, UNOEVENT_IMU_MOTION, NULL);
			break;

		case UNO_IMU_EVENT_STILL:
			if (uno_event_cb_fn)
				uno_event_cb_fn(uno_ctrl_caller, UNOEVENT_IMU_STILL, NULL);
			break;

		case UNO_IMU_EVENT_ACC:
			imu_data.x = ((float*) p)[0];
			imu_data.y = ((float*) p)[1];
			imu_data.z = ((float*) p)[2];
			if (uno_event_cb_fn)
				uno_event_cb_fn(uno_ctrl_caller, UNOEVENT_IMU_ACC, (void*) &imu_data);
			break;

		case UNO_IMU_EVENT_GYR:
			imu_data.x = ((float*) p)[0];
			imu_data.y = ((float*) p)[1];
			imu_data.z = ((float*) p)[2];
			if (uno_event_cb_fn)
				uno_event_cb_fn(uno_ctrl_caller, UNOEVENT_IMU_GYR, (void*) &imu_data);
			break;

		case UNO_IMU_EVENT_MAG:
			imu_data.x = ((float*) p)[0];
			imu_data.y = ((float*) p)[1];
			imu_data.z = ((float*) p)[2];
			if (uno_event_cb_fn)
				uno_event_cb_fn(uno_ctrl_caller, UNOEVENT_IMU_MAG, (void*) &imu_data);
			break;

		case UNO_IMU_EXCEPTION_UNKNOWN_PACKET:
			if (uno_event_cb_fn)
				uno_event_cb_fn(uno_ctrl_caller, UNOEVENT_IMU_EXCEPTION_UNKNOWN_PACKET, NULL);
			break;

		case UNO_IMU_EXCEPTION_UNPLUG:
			if (uno_event_cb_fn)
				uno_event_cb_fn(uno_ctrl_caller, UNOEVENT_IMU_EXCEPTION_UNPLUG, NULL);
			break;

		default:
			printf("unknown UNO_IMU_STATE\n");
			break;
		}
		// led
		uno_led_on_update_counter();
		usleep(1000 * 10);
	}
	pthread_exit(NULL);
}

int uno_ctrl_init(UNO_EVT_CB evt_cb, void * caller) {

	int ret, err_flag = 0;

	uno_ctrl_caller = caller;
	uno_event_cb_fn = evt_cb;

	if (uno_btn_init()) {
		perror("btn init error!\n");
		err_flag |= 0x01;
	}
	if (uno_imu_init()) {
		perror("IMU init error!\n");
		err_flag |= 0x02;
	}
	if (uno_led_init()) {
		perror("led init error\n");
		err_flag |= 0x04;
	}
	uno_ctrl_thread_running = 1;

	ret = pthread_create(&ctrl_thread, NULL, uno_ctrl_thread, NULL);
	if (ret != 0) {
		printf("create timer thread error: %d\n", (ret));
		err_flag |= 0x08;
	} else {
		DEBUG("Create timer thread success\n");
	}

	return -err_flag;
}

void uno_ctrl_uninit(void) {
	uno_ctrl_thread_running = 0;
	pthread_join(ctrl_thread, NULL);
	uno_btn_uninit();
	uno_led_uninit();
	if (uno_imu_uninit())
		printf("uno_imu_uninit() Error\n");
	else
		printf("IMU initiative exit\n");
}
