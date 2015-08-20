/*
 * utility.c
 *
 *  Created on: Jan 22, 2015
 *      Author: root
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "uno_ctrl.h"

int file_contain(FILE *fd, char * string){

	if(fd == NULL){
		perror("no file, so no contain!\n");
		return 0;
	}
	size_t line_size;
	char *line_buffer = NULL;
	DEBUG(" looking up for %s\n", string);
	while(getline(&line_buffer, &line_size, fd) > 0){
		DEBUG("current line: %s\n", line_buffer);
		if(strstr(line_buffer, string) != NULL){
			return 1;
		}
	}
	return 0;
}
