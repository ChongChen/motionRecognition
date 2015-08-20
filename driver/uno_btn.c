/*
 * uno_btn.c
 *
 *  Created on: Apr 1, 2015
 *      Author: root
 */

#include "uno_btn.h"
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

#include "gpio_ctrl.h"

int uno_btn_init(void) {
	int btn_ret = 0;
	btn_ret += gpio_export(RECOVERY_PIN);
	btn_ret += gpio_set_dir(RECOVERY_PIN, INPUT_PIN);
	return btn_ret;
}

int uno_btn_pressed(void) {
	unsigned int in0;
	gpio_get_value(RECOVERY_PIN, &in0);
	if( in0 == RECOVERY_PIN_PRESSED){
		return UNO_BTN_PRESSED;
	}else{
		return UNO_BTN_NOT_PRESSED;
	}
}


#define PRESSHOLD_CNT_TH	300
#define PRESS_CNT_TH		10

int uno_btn_check(void){
	static int pressed_cnt = 0, pressed_cnt_pre = 0;
	if(uno_btn_pressed() == UNO_BTN_PRESSED){
		if(pressed_cnt < 65535){
			pressed_cnt++;
		}
	}else{
		pressed_cnt = 0;
	}

	UNO_BTN_STATE flag;
	if((pressed_cnt_pre == PRESSHOLD_CNT_TH)){
		flag = UNO_BTN_PRESSHOLD;
	}else if((pressed_cnt == 0)&&(pressed_cnt_pre > PRESS_CNT_TH)
			&&(pressed_cnt_pre < PRESSHOLD_CNT_TH)){
		flag = UNO_BTN_PRESSED;
	}else{
		flag = UNO_BTN_NOT_PRESSED;
	}

	pressed_cnt_pre = pressed_cnt;
	return flag;
}


void uno_btn_uninit(void) {

	gpio_unexport(RECOVERY_PIN);

}
