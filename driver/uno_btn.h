/*
 * uno_btn.h
 *
 *  Created on: Apr 1, 2015
 *      Author: root
 */

#ifndef DRIVER_UNO_BTN_H_
#define DRIVER_UNO_BTN_H_

#define RECOVERY_PIN	65
#define RECOVERY_PIN_PRESSED 0

typedef enum BTN_STATE{
	UNO_BTN_NOT_PRESSED=0,
	UNO_BTN_PRESSED,
	UNO_BTN_PRESSHOLD
}UNO_BTN_STATE;

typedef void (*UNO_BTN_CB)(UNO_BTN_STATE state);

//return 0 when success
int uno_btn_init(void);
int uno_btn_pressed(void);
int uno_btn_check(void);
void uno_btn_uninit(void);



#endif /* DRIVER_UNO_BTN_H_ */
