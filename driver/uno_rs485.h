/*
 * uno_rs485.h
 *
 *  Created on: Dec 30, 2014
 *      Author: root
 */

#ifndef UNO_RS485_H_
#define UNO_RS485_H_


/*
 * rs485初始化，需要传入串口设备文件名。Linux下可能是/dev/ttyS0或者/dev/ttyUSB0
 */
int uno_rs485_init(const char * dev_str);
void uno_uninit_rs485(int fd);

/*
 * 在规定的时间内读取数据，超时则退出，超时时间为ms级别. timeout_in_ms 为超时时间.该时间段内函数会阻塞在此.
 * 返回值为读到的字节数
 */
int uno_rs485_read(int fd, char *str, unsigned int len, unsigned int timeout_in_ms);

/*
 * 返回值为发出的字节数
 */
int uno_rs485_write(int fd, char* writebuff, int size);


/*
 * 该函数会一直阻塞到收到规定长度的数据，不推荐使用
 */
int uno_rs485_block_read(int fd, char* readbuff, unsigned int size);

#define MIN_NUM_RX 		1
#define READ_WAIT_TIME	1		// in 100ms
#define BAUD_RATE 		115200
#define BIT_PER_BYTE	10


#endif /* UNO_RS485_H_ */
