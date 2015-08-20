/*
 * alarminout.c
 *
 *  Created on: Mar 5, 2015
 *      Author: root
 */

#include "uno_alarm.h"

#include "gpio_ctrl.h"


int uno_alarm_init(void){
	//init gpio
	int init_ret;
	init_ret = gpio_export(ALARM_IN0_PIN);
	init_ret += gpio_export(ALARM_IN1_PIN);
	init_ret += gpio_export(ALARM_OUT_PIN);
	init_ret += gpio_set_dir(ALARM_IN0_PIN, INPUT_PIN);
	init_ret += gpio_set_dir(ALARM_IN1_PIN, INPUT_PIN);
	init_ret += gpio_set_dir(ALARM_OUT_PIN, OUTPUT_PIN);
	return init_ret;
}


int uno_alarm_check(void){
	unsigned int in0, in1;
	gpio_get_value(ALARM_IN0_PIN, &in0);
	gpio_get_value(ALARM_IN1_PIN, &in1);
	if((in0 == ALARMED) && (in1 == NO_ALARMED)){
		return PORT_1_ALARMED;
	}else if( (in1 == ALARMED) && (in0 == NO_ALARMED)){
		return PORT_2_ALARMED;
	}else if((in0 == ALARMED) && (in1 == ALARMED)){
		return BOTH_PORT_ALARMED;
	}
	return NO_PORT_ALARMED;
}


int uno_alarm_set(void){
	return (gpio_set_value(ALARM_OUT_PIN, SET_ALARM));
}

int uno_alarm_unset(void){
	return (gpio_set_value(ALARM_OUT_PIN, UNSET_ALARM));
}

void uno_alarm_uninit(void){
	gpio_unexport(ALARM_IN0_PIN);
	gpio_unexport(ALARM_IN1_PIN);
	gpio_unexport(ALARM_OUT_PIN);
}
