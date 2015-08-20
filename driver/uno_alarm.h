/*
 * uno_alarm.h
 *
 *  Created on: Mar 5, 2015
 *      Author: root
 */

#ifndef UNO_ALARM_H_
#define UNO_ALARM_H_

typedef enum ALARM_STATUS{
	NO_PORT_ALARMED=0,
	PORT_1_ALARMED=1,
	PORT_2_ALARMED,
	BOTH_PORT_ALARMED
}ALARM_STATUS;


// return 0 when success
int uno_alarm_init(void);

/*
 * Check alarm input state.
 * return enum types, see uno_alarm.h, explained below:
 *  0: no alarm
 *  1: port 1 alarmed
 *  2: port 2 alarmed
 *  3: both ports alarmed
 */
int uno_alarm_check(void);

/*
 * Set an alarm out.
 */
int uno_alarm_set(void);
int uno_alarm_unset(void);
void uno_alarm_uninit(void);



#define ALARM_IN0_PIN	165
#define ALARM_IN1_PIN	166
#define ALARM_OUT_PIN	87

#define ALARMED 	LOW
#define NO_ALARMED 	HIGH

#define SET_ALARM 	LOW
#define UNSET_ALARM	HIGH



#endif /* UNO_ALARM_H_ */
