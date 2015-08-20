/*
 * utility.h
 *
 * example:
 *
   FILE *log_fd;
   char err_str[] = "Failed to set private addr: -121";
   log_fd = fopen("/var/log/syslog", "r");
   ret = file_contain(log_fd, err_str, 33);

 *  Created on: Jan 22, 2015
 *      Author: root
 */

#ifndef DRIVER_UNO_UTILITY_H_
#define DRIVER_UNO_UTILITY_H_

int file_contain(FILE *fd, char * string);

#endif /* DRIVER_UNO_UTILITY_H_ */
