/*
 * sp485.c
 * see sp485.h for details
 *
 *  Created on: Dec 29, 2014
 *      Author: root
 */


#include <stdio.h>
#include <termios.h>	/*PPSIX 终端控制定义*/
#include <unistd.h>
#include <fcntl.h>	/*文件控制定义*/
#include <sys/select.h>
#include "uno_rs485.h"

#include "gpio_ctrl.h"



#define TRUE 1
#define FALSE 0
//GPIO_PB1(V9/ UART4_CTS_L)	-> 485 ctrl
#define RS485_RXTX_CTRL		9
#define RS485_RX_ENABLE 	gpio_set_value(RS485_RXTX_CTRL, HIGH)
#define RS485_TX_ENABLE 	gpio_set_value(RS485_RXTX_CTRL, LOW)


static int set_Parity(int fd, int databits, int stopbits, int parity);
static int set_speed(int fd, int speed);



int uno_rs485_init(const char *dev_str){
	gpio_export(RS485_RXTX_CTRL);
	//RS485_RX_ENABLE;
	gpio_set_dir(RS485_RXTX_CTRL, OUTPUT_PIN);
	RS485_RX_ENABLE;

	int fd;
	// O_NOCTTY选项防止程序受键盘控制中止操作键等影响
	// O_NDELAY  告诉 UNIX 不必另一端端口是否启用.(检测 DCD信号线状态)
	fd = open(dev_str, O_RDWR | O_NOCTTY | O_NDELAY );
	if(fd == -1){
		//perror("unable to open /dev/ttyxxx\n");
		printf("unable to open /dev/ttyxxx\n");
		return -1;
	}
	fcntl(fd, F_SETFL, 0);	//change 0 to FNDELAY to immediately return after read()

	if(set_speed(fd, BAUD_RATE) != TRUE){
		perror("Unable to set baudrate\n");
		return -1;
	}
	if(set_Parity(fd, 8, 1, 'N') != TRUE){
		perror("Set Parity Error\n");
		return -1;
	}
	return fd;
}

void uno_uninit_rs485(int fd){
	gpio_unexport(RS485_RXTX_CTRL);
	close(fd);
}

int uno_rs485_block_read(int fd, char* readbuff, unsigned int size){

	int bytes_read;
	RS485_RX_ENABLE;
	bytes_read = read(fd, readbuff, size);
	if( bytes_read < 0){
		perror("sp485 block read error\n");
		return -1;
	}

	return bytes_read;
}
/*-----------------------------------------------------------------------------
  函数名:      serial_read
  参数:        int fd,char *str,unsigned int len,unsigned int timeout
  返回值:      在规定的时间内读取数据，超时则退出，超时时间为ms级别
  描述:        向fd描述符的串口接收数据，长度为len，存入str，timeout 为超时时间
 *-----------------------------------------------------------------------------*/
int uno_rs485_read(int fd, char *str, unsigned int len, unsigned int timeout)
{
	fd_set rfds;
	struct timeval tv;
	int ret;								//每次读的结果
	int sret;								//select监控结果
	int readlen = 0;						//实际读到的字节数
	char * ptr;

	RS485_RX_ENABLE;
	ptr = str;							//读指针，每次移动，因为实际读出的长度和传入参数可能存在差异

	FD_ZERO(&rfds);						//清除文件描述符集合
	FD_SET(fd,&rfds);					//将fd加入fds文件描述符，以待下面用select方法监听

	/*传入的timeout是ms级别的单位，这里需要转换为struct timeval 结构的*/
	tv.tv_sec  = timeout / 1000;
	tv.tv_usec = (timeout%1000)*1000;

	/*防止读数据长度超过缓冲区*/
	//if(sizeof(&str) < len)
	//	len = sizeof(str);
	while(readlen < len)
	{
		sret = select(fd+1,&rfds,NULL,NULL,&tv);		//检测串口是否可读

		if(sret == -1)										//检测失败
		{
			perror("select:");
			break;
		}
		else if(sret > 0)									//检测成功可读
		{
			ret = read(fd,ptr,1);
			//printf("sec: %d,usec: %d\n",tv.tv_sec,tv.tv_usec);
			if(ret < 0)
			{
				perror("read err:");
				break;
			}
			else if(ret == 0)
				break;

			readlen += ret;									//更新读的长度
			ptr     += ret;									//更新读的位置
		}
		else													//超时
		{
			//printf("timeout!\n");	//	timeout at least once
			break;
		}
	}
	return readlen;
}


int uno_rs485_write(int fd, char* writebuff, int size){

/*	int fd;

	fd = sp485_init();
	if(fd == -1){
		perror("sp485_init failed when write\n");
		return -1;
	}*/
	int bytes_written;

	RS485_TX_ENABLE;
	bytes_written = write(fd, writebuff, size);
	if(bytes_written < 0){
		perror("write failed\n");
		return -1;
	}
	usleep((long long)size*1000000*BIT_PER_BYTE/BAUD_RATE);
	//printf("write return delayed : %lld", (long long)size*1000000*BIT_PER_BYTE/BAUD_RATE);
	RS485_RX_ENABLE;
	//sp485_unload(fd);
	return bytes_written;
}


/**
 *@brief   设置串口波特率
 *@param  fd     类型  int  打开的串口文件句柄
 *@param  speed 类型  int 波特率数值
 *@return 0:ok
 */
static int speed_arr[] = { B38400, B19200, B9600, B4800, B2400, B1200, B300,
                      B38400, B19200, B9600, B4800, B2400, B1200, B300, B115200};
static int name_arr[] = {38400,  19200,  9600,  4800,  2400,  1200,  300, 38400,
                                   19200,  9600, 4800, 2400, 1200,  300, 115200};
static int set_speed(int fd, int speed){
       int   i;
       int   status;
       struct termios   Opt;
       //Get the current options for the port

       tcgetattr(fd, &Opt);

       for ( i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++) {
              if  (speed == name_arr[i]) {
                     cfsetispeed(&Opt, speed_arr[i]);
                     cfsetospeed(&Opt, speed_arr[i]);
                     status = tcsetattr(fd, TCSANOW, &Opt);
                     if  (status != 0) {
                            perror("tcsetattr fd");
                            return FALSE;
                     }
                     tcflush(fd,TCIOFLUSH);
                     return TRUE;
              }
       }
       return FALSE;	//should return before
}

/**
 *@brief   设置串口数据位，停止位和效验位
 *@param  fd     类型  int  打开的串口文件句柄
 *@param  databits 类型  int 数据位   取值 为 7 或者8
 *@param  stopbits 类型  int 停止位   取值为 1 或者2
 *@param  parity  类型  int  效验类型 取值为N,E,O,,S
 *@return 1:ok
 */
static int set_Parity(int fd, int databits, int stopbits, int parity) {
	struct termios options;
	if (tcgetattr(fd, &options) != 0) {
		perror("SetupSerial 1");
		return (FALSE);
	}
	options.c_cflag &= ~CSIZE;
	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); /*Input*/
	options.c_oflag &= ~OPOST; /*Output*/

	switch (databits) /*设置数据位数*/
	{
	case 7:
		options.c_cflag |= CS7;
		break;
	case 8:
		options.c_cflag |= CS8;
		break;
	default:
		fprintf(stderr, "Unsupported data size/n");
		return (FALSE);
	}
	switch (parity) {
	case 'n':
	case 'N':
		options.c_cflag &= ~PARENB; /* Clear parity enable */
		options.c_iflag &= ~INPCK; /* Enable parity checking */
		break;
	case 'o':
	case 'O':
		options.c_cflag |= (PARODD | PARENB); /* 设置为奇效验*/
		options.c_iflag |= INPCK; /* Disnable parity checking */
		break;
	case 'e':
	case 'E':
		options.c_cflag |= PARENB; /* Enable parity */
		options.c_cflag &= ~PARODD; /* 转换为偶效验*/
		options.c_iflag |= INPCK; /* Disnable parity checking */
		break;
	case 'S':
	case 's': /*as no parity*/
		options.c_cflag &= ~PARENB;
		options.c_cflag &= ~CSTOPB;
		break;
	default:
		fprintf(stderr, "Unsupported parity/n");
		return (FALSE);
	}
	/* 设置停止位*/
	switch (stopbits) {
	case 1:
		options.c_cflag &= ~CSTOPB;
		break;
	case 2:
		options.c_cflag |= CSTOPB;
		break;
	default:
		fprintf(stderr, "Unsupported stop bits/n");
		return (FALSE);
	}
	/* Set input parity option */
	if (parity != 'n')
		options.c_iflag |= INPCK;
	tcflush(fd, TCIFLUSH);
	options.c_cc[VTIME] = READ_WAIT_TIME; /* waiting time n 100ms, (start from 1st byte if VMIN not zero)*/
	options.c_cc[VMIN] = MIN_NUM_RX; /* define the minimum bytes data to be readed*/
	if (tcsetattr(fd, TCSANOW, &options) != 0) {
		perror("SetupSerial 3");
		return (FALSE);
	}
	return (TRUE);
}

