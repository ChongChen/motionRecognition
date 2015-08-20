/*
 * uno_led.h
 *
 *  Created on: Mar 5, 2015
 *      Author: root
 */

#ifndef UNO_LED_H_
#define UNO_LED_H_




//return 0 when success
int uno_led_init(void);

/*
 * Led Control.
 * "int led" can be "1" or "2".
 */
int uno_led_ctrl(int led, LED_STATE state);
void uno_led_toggle(int led);

/*
 * parameters:
 * led: 1 or 2 for led1 or led2.
 */

void uno_led_on_update_counter(void);	//TODO: add to uno_led_i.h, include others
void uno_led_uninit(void);

#define LED2J6		161
#define LED3J4		160


#endif /* UNO_LED_H_ */
