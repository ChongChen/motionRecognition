/*
 * uno_motionRecog.h
 *
 *  Created on: 2015.08.03
 *   Edited on:
 *      Author: Chen Chong
 */

#ifndef INCLUDE_UNO_MOTIONRECOG_H_
#define INCLUDE_UNO_MOTIONRECOG_H_

#ifdef __cplusplus
extern "C"{
#endif


/*
float max(float x, float y);
float arr_sum(float x[], int N);
float arr_max(float x[], int N);
float arr_min(float x[], int N);
float arr_avg(float x[], int N);
float arr_std(float x[], int N);

void arr_add(float x[], float f, int N);
void arr_sub(float x[], float f, int N);


void updateBias(float arr_gyrX[], float arr_gyrY[], float arr_gyrZ[], float energy_acc[], int N);

void correctBias(float arr_gyrX[], float arr_gyrY[], float arr_gyrZ[], float energy_acc[], int N);

void getEnergy(float x[], float y[], float z[], float energy[], int N);

int calZeroCrossRate(float x[], int N);

float calEntrophy(float energy[], int N);

int detectVibration(float energy_gyr[], float energy_acc[], int N);

void extractFeatures(float energy_acc[], float energy_gyr[], int N);

int classifyVibration(float energy_acc[], float energy_gyr[], int N);


int recognizeState(float arr_accX[], float arr_accY[], float arr_accZ[], float arr_gyrX[], float arr_gyrY[], float arr_gyrZ[], float energy_acc[], float energy_gyr[] ,int interval);

void recognizeMotion(void* para);
*/
typedef void (*MOTION_STATE_CB)(int state, float energy);

void motionRecogInit(MOTION_STATE_CB motion_cb, int stepLen);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_UNO_MOTIONRECOG_H_ */
