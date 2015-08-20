/*
 * IMU_API.h
 *
 *  Created on: 2015.03.23
 *   Edited on: 2015.05.26
 *      Author: Yongpeng
 */

#ifndef UNO_IMU_INTERNAL_H_
#define UNO_IMU_INTERNAL_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum{
	UNO_IMU_USB_CDC=0,
	UNO_IMU_USB_CUSTOM
}UNO_IMU_USB_TYPE;

typedef enum{
	UNO_IMU_NO_EVENT=0,
	UNO_IMU_EVENT_MOTION,
	UNO_IMU_EVENT_STILL,
	UNO_IMU_EVENT_ACC,
	UNO_IMU_EVENT_GYR,
	UNO_IMU_EVENT_MAG,
	UNO_IMU_EXCEPTION_UNKNOWN_PACKET,
	UNO_IMU_EXCEPTION_UNPLUG
}UNO_IMU_STATE;

typedef enum{
	GET_VERSION=0,
	GET_SN,
	SET_SN
}USB_REQUESTS;

// return -1 means failed
int uno_imu_init(void);
int uno_imu_uninit(void);

// send commands and receive events, recommend to use in the while() loop
UNO_IMU_STATE uno_imu_commands_events(void **pp);

//write the SN to the sensor
int uno_imu_set_sn(unsigned char *sn_data);

#ifdef __cplusplus
}
#endif

#endif /* UNO_IMU_INTERNAL_H_ */
