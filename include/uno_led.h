/*
 * uno_led.h
 *
 *  Created on: Apr 9, 2015
 *      Author: root
 */

#ifndef INCLUDE_UNO_LED_H_
#define INCLUDE_UNO_LED_H_

typedef enum _LED_STATE{
	UNO_LED_OFF=0,
	UNO_LED_ON
}LED_STATE;

typedef enum _LED_PATTERN{
	UNO_LED_PATTERN_OFF=0,
	UNO_LED_PATTERN_ON,
	UNO_LED_PATTERN_BREATH,
	UNO_LED_PATTERN_FLASH
}LED_PATTERN_T;

/*
 * int led can be 1 or 2 for led1 or led2
 */
void uno_led_set_predef_pattern(int led, LED_PATTERN_T pattern);
void uno_led_pattern(int led, LED_STATE init_state, int compare, int period);


#endif /* INCLUDE_UNO_LED_H_ */
