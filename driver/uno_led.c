/*
 * led.c
 *
 *  Created on: Mar 5, 2015
 *      Author: root
 */
#include <stdio.h>
#include "gpio_ctrl.h"
#include "uno_led.h"
#include "uno_led_i.h"

typedef struct _UNO_LED_PATTERN{
	unsigned int cnt;
	int compare;
	int period;
	LED_STATE init_state;
}UNO_LED_PATTERN_T;

UNO_LED_PATTERN_T uno_led[2];
#define TOTAL_LED_NUM 	2


int uno_led_init(void){
	int i;
	for(i = 0; i < 2; i++){
		uno_led[i].compare = 101;
		uno_led[i].period = 100;
		uno_led[i].init_state = UNO_LED_ON;
	}
	int led_ret;
	led_ret = gpio_export(LED2J6);
	led_ret += gpio_export(LED3J4);
	led_ret += gpio_set_dir(LED2J6, OUTPUT_PIN);
	led_ret += gpio_set_dir(LED3J4, OUTPUT_PIN);
	return led_ret;
}


int uno_led_ctrl(int led, LED_STATE state){
	switch(led){
	case 1:
		return (gpio_set_value(LED2J6, state));
		break;
	case 2:
		return (gpio_set_value(LED3J4, state));
		break;
	default:
		return -1;
	}
}


void uno_led_toggle(int led){
	unsigned int led1_state, led2_state;
	switch(led){
	case 1:
		gpio_get_value(LED2J6, &led1_state);
		if(led1_state == UNO_LED_OFF){
			uno_led_ctrl(1, UNO_LED_ON);
		}else{
			uno_led_ctrl(1, UNO_LED_OFF);
		}
		break;
	case 2:
		gpio_get_value(LED3J4, &led2_state);
		if(led2_state == UNO_LED_OFF){
			uno_led_ctrl(2, UNO_LED_ON);
		}else{
			uno_led_ctrl(2, UNO_LED_OFF);
		}
		break;
	default:
		break;
	}
}





void uno_led_pattern(int led, LED_STATE init_state, int compare, int period){
	if((led > 0)&&(led <= TOTAL_LED_NUM)){
		uno_led[led-1].init_state = init_state;
		uno_led[led-1].compare = compare;
		uno_led[led-1].period = period;
		uno_led[led-1].cnt = 0;
	}
}

void uno_led_on_update_counter(void){
	int i;
	for(i = 0 ; i < 2; i++){
		if(uno_led[i].cnt < uno_led[i].period){
			uno_led[i].cnt++;
		}else{
			uno_led[i].cnt = 0;
			uno_led_ctrl(i+1, uno_led[i].init_state);
		}

		if(uno_led[i].cnt == uno_led[i].compare){
			uno_led_toggle(i+1);
		}
	}
}

void uno_led_set_predef_pattern(int led, LED_PATTERN_T pattern){
	switch(pattern){
	case UNO_LED_PATTERN_ON:
		uno_led_pattern(led, UNO_LED_ON, 101, 100);
		break;
	case UNO_LED_PATTERN_OFF:
		uno_led_pattern(led, UNO_LED_OFF, 101, 100);
		break;
	case UNO_LED_PATTERN_BREATH:
		uno_led_pattern(led, UNO_LED_OFF, 100, 200);
		break;
	case UNO_LED_PATTERN_FLASH:
		uno_led_pattern(led, UNO_LED_OFF, 25, 50);
		break;
	default:
		break;
	}
}

void uno_led_uninit(void){
	gpio_unexport(LED2J6);
	gpio_unexport(LED3J4);
}
