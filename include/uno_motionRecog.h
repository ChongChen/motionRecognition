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



typedef void (*MOTION_STATE_CB)(int state, float energy);

void motionRecogInitCB(MOTION_STATE_CB motion_cb, float period, int Fs);
void motionRecogInit(float period, int Fs);
#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_UNO_MOTIONRECOG_H_ */
