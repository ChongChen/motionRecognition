/*
 * uno_imu.h
 *
 *  Created on: 2015.04.09
 *   Edited on: 2015.05.26
 *      Author: Yongpeng
 */

#ifndef INCLUDE_UNO_IMU_H_
#define INCLUDE_UNO_IMU_H_

#ifdef __cplusplus
extern "C" {
#endif

int uno_imu_request_acc(void);
int uno_imu_request_gyr(void);
int uno_imu_request_mag(void);
int uno_imu_get_acc(float *x, float *y, float *z);//blocking to get acc
int uno_imu_get_gyr(float *x, float *y, float *z);
int uno_imu_get_mag(float *x, float *y, float *z);

void radian_from_acc(	float acc_x, float acc_y, float acc_z,
						float *acc_x2plumb_line, float *acc_y2plumb_line, float *acc_z2plumb_line);	// unit is radian

// return -1 means parameters are invalid or set failed
int uno_imu_set_motion_threshold_acc(float x, float y, float z);	// unit is g
int uno_imu_set_motion_threshold_gyr(float x, float y, float z);	// unit is (radian/s)
int uno_imu_set_still_threshold_acc(float var_x, float var_y, float var_z);	// parameters are variances, unit is g square
int uno_imu_set_still_threshold_gyr(float var_x, float var_y, float var_z);	// parameters are variances, unit is (radian/s) square

//get the version of the sensor
int uno_imu_get_version(unsigned char *version);
//get the SN of the sensor
int uno_imu_get_sn(unsigned char *sn_data);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_UNO_IMU_H_ */
