#include "gpio_ctrl.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>


/*
 *   echo 57 > /sys/class/gpio/export
  echo out > /sys/class/gpio/gpio57/direction
  echo 1 > /sys/class/gpio/gpio57/value
   echo 57 > /sys/class/gpio/unexport
  exit
 *
 */

/****************************************************************
 * gpio_export
 ****************************************************************/
int gpio_export(unsigned int gpio)
{
	int fd, len;
	char buf[MAX_BUF];

	//printf("gpio_export: %d\n", gpio);
	char cmd_line[256];
	char gpio_str[4];
	sprintf(gpio_str,"%d", gpio);
	strcpy(cmd_line, "echo ");
	strcat(cmd_line, gpio_str);
	strcat(cmd_line, " > /sys/class/gpio/export");
	//printf("gpio cmd_line: %s\n", cmd_line);/**/
	int ret;
	ret = system(cmd_line);
	if(ret != 0){
		printf("gpio export cmd lint error\n");
	}
	usleep(50000);

	fd = open(SYSFS_GPIO_DIR "/export", O_WRONLY);
	if (fd < 0) {
		perror("gpio/export");
		return fd;
	}

	len = snprintf(buf, sizeof(buf), "%d", gpio);
	if(write(fd, buf, len) == 0){
		perror("gpio export write error\n");
		return -1;
	}
	close(fd);

	return 0;
}

/****************************************************************
 * gpio_unexport
 ****************************************************************/
int gpio_unexport(unsigned int gpio)
{
	int fd, len;
	char buf[MAX_BUF];

	fd = open(SYSFS_GPIO_DIR "/unexport", O_WRONLY);
	if (fd < 0) {
		perror("gpio/export");
		return fd;
	}

	len = snprintf(buf, sizeof(buf), "%d", gpio);
	if(write(fd, buf, len) == 0){
		perror("gpio unexport write error\n");
		return -1;
	}
	close(fd);
	return 0;
}

/****************************************************************
 * gpio_set_dir
 ****************************************************************/
int gpio_set_dir(unsigned int gpio, PIN_DIRECTION out_flag)
{
	int fd;
	char buf[MAX_BUF];

	snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR  "/gpio%d/direction", gpio);

	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		perror("gpio/direction");
		return fd;
	}

	if (out_flag == OUTPUT_PIN){
		if(write(fd, "out", 4) == 0){
			perror("gpio set value error\n");
			return -1;
		}
	}else{
		if(write(fd, "in", 3) == 0){
			perror("gpio set value error\n");
			return -1;
		}
	}
	close(fd);
	return 0;
}

/****************************************************************
 * gpio_set_value
 ****************************************************************/
int gpio_set_value(unsigned int gpio, PIN_VALUE value)
{
	int fd;
	char buf[MAX_BUF];

	snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);

	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		//perror("gpio/set-value");
		return fd;
	}

	if (value==LOW){
		if(write(fd, "0", 2) == 0){
			perror("gpio set value error\n");
			return -1;
		}
	}
	else{
		if(write(fd, "1", 2) == 0){
			perror("gpio set value error\n");
			return -1;
		}
	}

	close(fd);
	return 0;
}

/****************************************************************
 * gpio_get_value
 ****************************************************************/
int gpio_get_value(unsigned int gpio, unsigned int *value)
{
	int fd;
	char buf[MAX_BUF];
	char ch;

	snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);

	fd = open(buf, O_RDONLY);
	if (fd < 0) {
		//perror("gpio/get-value");
		return fd;
	}

	if(read(fd, &ch, 1) != 1){
		perror("gpio read error");
		return -1;
	}

	if (ch != '0') {
		*value = 1;
	} else {
		*value = 0;
	}

	close(fd);
	return 0;
}


/****************************************************************
 * gpio_set_edge
 ****************************************************************/

int gpio_set_edge(unsigned int gpio, char *edge)
{
	int fd;
	char buf[MAX_BUF];

	snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/edge", gpio);

	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		perror("gpio/set-edge");
		return fd;
	}

	if(write(fd, edge, strlen(edge) + 1) == 0){
		return -1;
	}
	close(fd);
	return 0;
}

/****************************************************************
 * gpio_fd_open
 ****************************************************************/

int gpio_fd_open(unsigned int gpio)
{
	int fd;
	char buf[MAX_BUF];

	snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);

	fd = open(buf, O_RDONLY | O_NONBLOCK );
	if (fd < 0) {
		perror("gpio/fd_open");
	}
	return fd;
}

/****************************************************************
 * gpio_fd_close
 ****************************************************************/

int gpio_fd_close(int fd)
{
	return close(fd);
}