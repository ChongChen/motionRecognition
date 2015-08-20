/*
 * main.c
 *
 *  Created on: Mar 5, 2015
 *      Author: root
 */

#include "uno_api.h"

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include <time.h>

char ap_ssid[] = "tk123";
char ap_pwd[] = "1234567890";
//char sta_ssid[] = "topgear";
//char sta_pwd[] = "topgear1234";
char sta_ssid[] = "deepglint";
char sta_pwd[] = "On1shiuva4";
int counter;

// uno imu, led, btn and wifi callback example:
// Press to switch to ap mode. Presshold to connect wifi router.
void uno_event_cb_func(void *caller, UNO_EVENT _event, void * para){
	UNO_IMU_DATA_T *data = para;

	switch(_event){
	case UNOEVENT_BTN_PRESSHOLD:
		printf("callback button press hold\n");
		uno_wifi_switch_ap(ap_ssid, ap_pwd, 5);
		break;
	case UNOEVENT_BTN_PRESSED:
		printf("callback button pressed!\n");
		uno_wifi_switch_sta(sta_ssid, sta_pwd);
		break;
	case UNOEVENT_IMU_MOTION:
		printf("callback function IMU moved\n");
		break;
	case UNOEVENT_IMU_STILL:
		printf("callback function IMU still\n");
		break;
	case UNOEVENT_IMU_ACC:
		//counter++;
		//if(counter % 1000 == 0)
			//printf("%dk\n", counter/1000);
	//	..printf("callback function IMU acc\n");
	//	printf("acc data: %6.2f, %6.2f, %6.2f\n", data->x, data->y, data->z);
//		float orientation[3];
//		radian_from_acc(data->x, data->y, data->z, orientation, orientation+1, orientation+2);
	//	printf("orientation: %6.2f, %6.2f, %6.2f\n", orientation[0] * 180.0 / 3.14159, orientation[1] * 180.0 / 3.14159, orientation[2] * 180.0 / 3.14159);
		break;
	case UNOEVENT_IMU_GYR:
		//counter++;
		//if(counter % 1000 == 0)
			//printf("%dk\n", counter/1000);
		
		//printf("callback function IMU gyr\n");
		//printf("gyr data: %6.2f, %6.2f, %6.2f\n", data->x, data->y, data->z);
		break;
	case UNOEVENT_IMU_MAG:
		//counter++;
		//if(counter % 1000 == 0)
			//printf("%dk\n", counter/1000);
		//printf("callback function IMU mag\n");
		//printf("mag data: %6.2f, %6.2f, %6.2f\n", data->x, data->y, data->z);
		break;
	case UNOEVENT_IMU_EXCEPTION_UNKNOWN_PACKET:
		printf("UNOEVENT_IMU_EXCEPTION_UNKNOWN_PACKET\n");
		break;
	case UNOEVENT_IMU_EXCEPTION_REQUEST_TIMEOUT:
		printf("UNOEVENT_IMU_EXCEPTION_REQUEST_TIMEOUT\n");
		break;
	case UNOEVENT_IMU_EXCEPTION_UNPLUG:
		printf("UNOEVENT_IMU_EXCEPTION_UNPLUG\n");
		break;
	default:
		break;
	}
}

// wifi callback example:
// if switch to wifi router with wrong password or dhcp timeout, switch to ap mode without password.
void uno_wifi_cb_func(void * caller, int uno_wifi_state, int err_no){
	switch(uno_wifi_state){
	case UNO_WIFI_AP:
		if(err_no == 0){
			printf("cb func: wifi ap state\n");
		}else{
			printf("cb func: wifi ap already!\n");
		}
		break;
	case UNO_WIFI_STA:
		printf("cb func: wifi sta\n");
		break;
	case UNO_WIFI_STA_ING:
		if(err_no == 0){
			printf("cb func: sta ing\n");
		}else if(err_no == 1){
			printf("cb func: request to ap ignored\n");
		}else{
			printf("cb func: request to sta ignored\n");
		}
		break;
	case UNO_WIFI_IDLE:

		if(err_no == 1){
			printf("cb func: request to sta error, maybe wrong router ssid/pwd?\n");
			//example: when switch to sta with wrong password, then switch to ap
			uno_wifi_switch_ap(ap_ssid, NULL, 1);
		}
		if(err_no == 2){
			printf("cb func: DHCP timeout \n");\
			uno_wifi_switch_ap(ap_ssid, NULL, 1);
		}
		break;
	default:
		break;
	}
}


void uno_motion_cb_func(int state, float energy){
	switch (state){
		case 0: printf("Still, "); break;
		case 1: printf("Vibrating, "); break;
		case 2: printf("Moving, "); break;
		case 3: printf("Rotating, "); break;
	}
	printf("Energy sum: %f\n\n\n", energy);
}


int main(int argc, char *argv[]){

	int stepLen = 15;
	//recognizeMotion();
	motionRecogInit(uno_motion_cb_func, stepLen);


	while(1){
		printf("main func running...\n\n");
		sleep(1);
	}


	/*
	//read sn demo
	unsigned char buffer[16];
//	get_uno_sn(buffer);
//	printf("uno sn: %s\n", buffer);

	//uno_evt demo:
	int ret;
	ret = uno_ctrl_init(NULL, NULL);
	if(ret == 0){
		DEBUG("init success\n");
	}else{
		printf("unoo_ctrl_init error, error number:%d\n", ret);
	}

	//read imu sn demo
	uno_imu_get_version(buffer);
	printf("uno imu version[0]: %d\n", buffer[0]);
	printf("uno imu version[1]: %d\n", buffer[1]);

	//get the SN of the sensor
	memset(buffer,0,sizeof(16));
	uno_imu_get_sn(buffer);
	printf("uno imu sn: %s\n", buffer);


#if 1
	// IMU demo
	UNO_IMU_DATA_T data_acc;
	UNO_IMU_DATA_T data_gyr;
	UNO_IMU_DATA_T data_mag;

	// fragment length: about 1 second
	const int frag_len = 30;
	float frag_accX[frag_len];
	float frag_accY[frag_len];
	float frag_accZ[frag_len];

	float frag_gyrX[frag_len];
	float frag_gyrY[frag_len];
	float frag_gyrZ[frag_len];

	float energy_acc[frag_len];
	float energy_gyr[frag_len];

	
	while(1){
		
		//time_t start;
		//time(&start);
		
		for (int i = 0; i < frag_len; i++){
			uno_imu_get_acc(&(data_acc.x), &(data_acc.y), &(data_acc.z));
			uno_imu_get_gyr(&(data_gyr.x), &(data_gyr.y), &(data_gyr.z));

			frag_accX[i] = data_acc.x;
			frag_accY[i] = data_acc.y;
			frag_accZ[i] = data_acc.z;

			frag_gyrX[i] = data_gyr.x;
			frag_gyrY[i] = data_gyr.y;
			frag_gyrZ[i] = data_gyr.z;

		}
	
		//Get energy for Correcting bias
		getEnergy(frag_gyrX, frag_gyrY, frag_gyrZ, energy_gyr, frag_len);
		//Get energy_acc with bias
		getEnergy(frag_accX, frag_accY, frag_accZ, energy_acc, frag_len);
		float energy_gyr_max = arr_max(energy_gyr, frag_len);
		printf("Biased gyr energy:%f\n", energy_gyr_max);

		if (energy_gyr_max < 0.05){
			updateBias(frag_gyrX, frag_gyrY, frag_gyrZ, energy_acc, frag_len);
		}
		// correct bias for gyr, energy_acc
		correctBias(frag_gyrX, frag_gyrY, frag_gyrZ, energy_acc, frag_len);

		// Get energy_gyr without bias
		getEnergy(frag_gyrX, frag_gyrY, frag_gyrZ, energy_gyr, frag_len);

		//printf("unbiased energy: %f,%f\n\n", arr_max(energy_acc, frag_len), arr_max(energy_gyr, frag_len));
		
		// Recognize the motion state
		int state = recognizeState(frag_accX, frag_accY, frag_accZ, frag_gyrX, frag_gyrY, frag_gyrZ, energy_acc, energy_gyr, frag_len);


		switch (state){
			case 0: printf("still\n"); break;
			case 2: printf("moving\n"); break;
			case 3: printf("rotating\n"); break;
			case 1: printf("vibration\n"); break;
		}
		
		
		//time_t finish;
		//time(&finish);
		//double duration = (double)(finish-start);
	
		//printf("time comsuming: %f\n seconds", duration);
	}
	


	// Collecting data into .txt file

	float euler_angle[3];

	char fileName[50];
	strcpy(fileName, "data/");
	strcat(fileName, argv[1]);

	int N = 300;
	FILE *fp;
	if ((fp=fopen(fileName, "w"))==NULL) {
		printf("Cannot open file strke any key exit!");
		return 0;
	}

	printf("Collecting start\n");
	time_t start;
	time(&start);

	while(N--){
	
	//#if 1
//		uno_imu_request_acc();
		uno_imu_get_acc(&(data_acc.x), &(data_acc.y), &(data_acc.z));
		//printf("Acc    x: %6.5f   y: %6.5f    z: %6.5f\n", data_acc.x, data_acc.y, data_acc.z);
		

		uno_imu_get_gyr(&(data_gyr.x), &(data_gyr.y), &(data_gyr.z));
		//printf("Gyr    x: %6.2f   y: %6.2f    z: %6.2f\n", data_gyr.x, data_gyr.y, data_gyr.z);


		uno_imu_get_mag(&(data_mag.x), &(data_mag.y), &(data_mag.z));
		//printf("Mag    x: %6.2f   y: %6.2f    z: %6.2f\n\n\n", data_mag.x, data_mag.y, data_mag.z);
		
		fprintf(fp, "%f %f %f %f %f %f %f %f %f\n", data_acc.x, data_acc.y, data_acc.z, data_gyr.x, data_gyr.y, data_gyr.z, data_mag.x, data_mag.y, data_mag.z);
	

		printf("collecting\n");
	//	sleep(1);
			
	}
	
#endif

	fclose(fp);
	printf("Collecting complete\n");
	
	time_t finish;
	time(&finish);
	double duration = (double)(finish-start);
	
	printf("time comsuming: %f\n seconds", duration);
	*/




#if 0
	//wifi demo:
	ret = uno_wifi_init(uno_wifi_cb_func, NULL);
	if(ret == 0){
		DEBUG("init success\n");
	}else{
		printf("uno_wifi_init error, err_no: %d\n", ret);
	}
#endif
	//below for test/demo
	

	/*char ch;

	while (1)
	{
		ch = getchar();

		switch (ch)
		{
			case 'd':// initiative exit
				uno_ctrl_uninit();
				uno_wifi_uninit();
				return 0;
				break;
			// IMU
			case 'a':
				uno_imu_request_acc();
				break;
			case 'g':
				uno_imu_request_gyr();
				break;
			case 'm':
				uno_imu_request_mag();
				break;

			case 'e':
				uno_led_set_predef_pattern(1, UNO_LED_PATTERN_FLASH);
				uno_led_set_predef_pattern(2, UNO_LED_PATTERN_FLASH);
				break;
			case 'o':
				uno_led_set_predef_pattern(1, UNO_LED_PATTERN_OFF);
				uno_led_set_predef_pattern(2, UNO_LED_PATTERN_OFF);
				break;
			case 'b':
				uno_led_set_predef_pattern(1, UNO_LED_PATTERN_BREATH);
				uno_led_set_predef_pattern(2, UNO_LED_PATTERN_BREATH);
				break;
			case 'n':
				uno_led_set_predef_pattern(1, UNO_LED_PATTERN_ON);
				uno_led_set_predef_pattern(2, UNO_LED_PATTERN_ON);
				break;
			case 'p':	//switch to ap test
				ret = uno_wifi_switch_ap(ap_ssid, ap_pwd, 1);
				printf("switch to ap request returned: %d\n", ret);
				break;
			case 's':	//switch to sta test
				ret = uno_wifi_switch_sta(sta_ssid, sta_pwd);
				printf("switch to sta returned: %d\n", ret);
				break;
			case 'q':	//switch to open ap test
				ret = uno_wifi_switch_ap("tk1_open_ssid", NULL, 1);
				printf("switchto open ap returned: %d\n", ret);
				break;
			case 'w':	//switch to sta with wrong password test
				ret = uno_wifi_switch_sta(sta_ssid, "wrong_password");
				printf("switch to sta with wrong psw returned: %d\n", ret);
				break;
		}
	}*/

	/*

	//sp485 demo:
	//1. init sp485 device.
	int fd_485;
	fd_485 = uno_rs485_init("/dev/ttyTHS0");
	if( fd_485 < 0){
		printf("sp485 init error\n");
	}else{
		//2. send data through sp485
		int bytes_written;
		bytes_written = uno_rs485_write(fd_485, "testing testing\n", 16);
		// you can check bytes_written here
		if(bytes_written < 16){
			printf("sp485 write error\n");
		}
		//3. receive data through sp485
		char read_buffer[128];
		int bytes_recieved;
		bytes_recieved = uno_rs485_read(fd_485, read_buffer, 16, 100);
		read_buffer[bytes_recieved] = '\0';
		printf("received bytes are : %s\n", read_buffer);

		//4. don't forget to uninit sp485
		uno_uninit_rs485(fd_485);
	}

	//alarm demo:
	if(uno_alarm_init() < 0){
		printf("alarm init error\n");
	}else{
		//detect alarm:
		switch(uno_alarm_check()){

		case NO_PORT_ALARMED:
			printf("no alarm detected\n");
			break;
		case PORT_1_ALARMED:
			printf("port 1 alarm detected\n");
			break;
		case PORT_2_ALARMED:
			printf("port 2 alarm detected\n");
			break;
		case BOTH_PORT_ALARMED:
			printf("both ports alarm detected\n");
			break;
		default:
			break;
		}

		//send an alarm
		uno_alarm_set();
		printf("out alarm sent\n");
		sleep(1);

		// cancel an alarm
		uno_alarm_unset();
		printf("out alarm stopped\n");

		//uninit Alarm
		uno_alarm_uninit();
	}*/
	return 0;
}
