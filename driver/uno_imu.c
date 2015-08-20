/*
 * IMU_API.c
 *
 *  Created on: 2015.03.23
 *   Edited on: 2015.05.26
 *      Author: Yongpeng
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <math.h>
#include <pthread.h>
#include <libusb-1.0/libusb.h>

#include "uno_imu.h"
#include "uno_imu_i.h"

#define DEBUG	0

#ifndef PI
#define PI	3.14159
#endif

#define IMU_USB_VID				0x0483
#define IMU_USB_PID				0x5760

#define	USB_EP1_IN_SIZE			8
#define	USB_EP3_OUT_SIZE		8

#define USB_VENDOR			 0X40
#define USB_EP0_IN 						0X80
#define USB_EP0_OUT 					0X00
#define USB_EP0_SIZE_VERSION    2
#define USB_EP0_SIZE_SN      12

#define IMU_HEADER_LEN			2
#define IMU_RAW_DATA_LEN		6
#define	IMU_PACKET_LEN			(IMU_HEADER_LEN + IMU_RAW_DATA_LEN)
#define	IMU_BUFFER_LEN			IMU_PACKET_LEN * 5

#define OFFSET_ACC				0x01
#define OFFSET_GYR				0x02
#define OFFSET_MAG				0x04

// Device --> Host
#define D2H_SYNC_BYTE			0xAA
#define D2H_DATA_CLASS			0x00
#define D2H_MOTION_CLASS		0xA0
#define D2H_STILL_CLASS			0xB0
#define D2H_ERROR_CLASS			0xE0

// Host --> Device
#define H2D_SYNC_BYTE			0x5A
#define H2D_DATA_REQUEST_CLASS	0x00
#define H2D_MOTION_THR_CLASS	0x10
#define H2D_STILL_THR_CLASS		0x20

#define REQUEST_OVERFLOW		100
#define GET_USLEEP_MS			1
#define GET_TIMEOUT_MS			1000

static int gl_fd;
static struct libusb_context *gl_libusb_context = NULL;
static struct libusb_device **gl_device;
static struct libusb_device_handle *gl_device_handle = NULL;
static UNO_IMU_USB_TYPE gl_imu_usb_type;
static float gl_adv_xyz[3]; // AD value from MEMS IC via I2C
static volatile int gl_request_acc, gl_request_gyr, gl_request_mag; // indicating request amount

static void uart_init(int fd)
{
	struct termios newtio;
	memset(&newtio, 0, sizeof(newtio));	// clear struct for new port settings

	newtio.c_cflag = B115200 | CS8 | CLOCAL | CREAD;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;
	newtio.c_lflag = 0;

	newtio.c_cc[VTIME] = 10; // inter-character 10 * 0.1 = 1s
	newtio.c_cc[VMIN] = 8;

	tcflush(fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &newtio);
}

static int open_custom_usb(void)
{
	gl_device_handle = libusb_open_device_with_vid_pid(gl_libusb_context,
	IMU_USB_VID,
	IMU_USB_PID);

	if (gl_device_handle == NULL)
	{
		printf("libusb_open_device_with_vid_pid() failed.\n");
		return -1;
	}
	else
	{
		libusb_free_device_list(gl_device, 1);

		// Decide to use the kernel driver or not
		if (libusb_kernel_driver_active(gl_device_handle, 0) == 1)
		{
			printf("Kernel driver.\n");
			if (libusb_detach_kernel_driver(gl_device_handle, 0) == 0) // detach it
				printf("Kernel driver Detached.\n");
		}

		// claim the interface
		if (libusb_claim_interface(gl_device_handle, 0) < 0)
		{
			printf("libusb_claim_interface() failed.\n");
			return -1;
		}
	}

	return 0;
}

static int imu_open(void)
{
	// CDC USB device
	char filename[] = "/dev/ttyACMx", ch;

	for (ch = '0'; ch <= '9'; ch++)
	{
		filename[11] = ch;
		gl_fd = open(filename, O_RDWR | O_NOCTTY | O_NDELAY | O_NONBLOCK);
		if (gl_fd != -1) // open success
		{
#if DEBUG
			printf("CDC USB found.\n");
#endif
			uart_init(gl_fd);
			gl_imu_usb_type = UNO_IMU_USB_CDC;
			return 0;
		}
	}

	// Custom USB device
	struct libusb_device_descriptor desc;

	int ret = libusb_init(&gl_libusb_context);
	if (ret < 0)
	{
		printf("libusb_init() failed.\n");
		return -1;
	}

	ret = libusb_get_device_list(gl_libusb_context, &gl_device);
	if (ret > 0)
		for (int i = 0; i < ret; i++)
		{
			libusb_get_device_descriptor(gl_device[i], &desc);
			if (desc.idVendor == IMU_USB_VID && desc.idProduct == IMU_USB_PID)
			{
#if DEBUG
				printf("Custom USB found.\n");
#endif
				gl_imu_usb_type = UNO_IMU_USB_CUSTOM;
				return open_custom_usb();
			}
		}

	return -1;
}

// MEMS IC AD value --> physical
static void convert_adv2phy(char offset_axis, short *adv_s16, float *x,
		float *y, float *z)
{
	switch (offset_axis)
	{
	case OFFSET_ACC: // unit: g
		*x = (float) adv_s16[0] / 16384.0;
		*y = (float) adv_s16[1] / 16384.0;
		*z = (float) adv_s16[2] / 16384.0;
		break;
	case OFFSET_GYR: // unit: radian/s
		*x = (float) adv_s16[0] / 16.4 / 180.0 * PI;
		*y = (float) adv_s16[1] / 16.4 / 180.0 * PI;
		*z = (float) adv_s16[2] / 16.4 / 180.0 * PI;
		break;
	case OFFSET_MAG: // unit: uT
		*x = (float) adv_s16[0] * 0.3;
		*y = (float) adv_s16[1] * 0.3;
		*z = (float) adv_s16[2] * 0.3;
		break;
	}
}

// physical --> MEMS IC AD value
static int convert_phy2adv_u16(char offset_axis, float x, float y, float z,
		unsigned short *adv_u16)
{
	float adv_f[3] =
	{ 0.0 };
	int flag = 0;

	switch (offset_axis)
	{
	case OFFSET_ACC:
		adv_f[0] = (float) x * 16384.0;
		adv_f[1] = (float) y * 16384.0;
		adv_f[2] = (float) z * 16384.0;
		break;
	case OFFSET_GYR:
		adv_f[0] = (float) x * 16.4 * 180.0 / PI;
		adv_f[1] = (float) y * 16.4 * 180.0 / PI;
		adv_f[2] = (float) z * 16.4 * 180.0 / PI;
		break;
	case OFFSET_MAG:
		adv_f[0] = (float) x / 0.3;
		adv_f[1] = (float) y / 0.3;
		adv_f[2] = (float) z / 0.3;
		break;
	default:
		flag = -1;
		break;
	}

	// output adv(AD_Value) valid check is necessary!
	for (int i = 0; i < 3; i++)
		if (adv_f[i] > 0xFFFF)
		{
			adv_u16[i] = 0xFFFF;
			flag = -1;
		}
		else if (adv_f[i] < 0.0)
		{
			adv_u16[i] = 0;
			flag = -1;
		}
		else
			adv_u16[i] = adv_f[i];

	return flag;
}

static int send_packet(unsigned char request_type, unsigned short *adv_u16)
{
	unsigned char packet[IMU_PACKET_LEN] =
	{ H2D_SYNC_BYTE, request_type };

	for (int i = 0; i < 3; i++)
	{ // fixed big-endian
		packet[i * 2 + IMU_HEADER_LEN] = adv_u16[i] >> 8;
		packet[i * 2 + IMU_HEADER_LEN + 1] = adv_u16[i] & 0x00FF;
	}

	if (gl_imu_usb_type == UNO_IMU_USB_CDC)
	{
		if (write(gl_fd, packet, IMU_PACKET_LEN) < 0)
		{
			perror("write() Error");
			return -1;
		}
	}
	else if (gl_imu_usb_type == UNO_IMU_USB_CUSTOM)
	{
		int actual;
		libusb_bulk_transfer(gl_device_handle, (3 | LIBUSB_ENDPOINT_OUT),
				packet, USB_EP3_OUT_SIZE, &actual, 1);
#if DEBUG
		static int counter = 0;
		printf("%d: OUT(%d)\n", counter++, actual);
#endif
	}
	else
		return -1;

	return 0;
}

static int set_motion_threshold(int offset_axis, float x, float y, float z)
{
	unsigned short adv_u16[3];

	if (convert_phy2adv_u16(offset_axis, x, y, z, adv_u16) == -1)
		return -1;
	if (send_packet(H2D_MOTION_THR_CLASS + offset_axis, adv_u16) == -1)
		return -1;

	return 0;
}

int uno_imu_set_motion_threshold_acc(float x, float y, float z)
{
	return set_motion_threshold(OFFSET_ACC, x, y, z);
}

int uno_imu_set_motion_threshold_gyr(float x, float y, float z)
{
	return set_motion_threshold(OFFSET_GYR, x, y, z);
}

static int set_still_threshold(int offset_axis, float var_x, float var_y,
		float var_z)
{
	float std_x, std_y, std_z;

	if ((var_x < 0) || (var_y < 0) || (var_z < 0))
		return -1;

	std_x = sqrt(var_x);
	std_y = sqrt(var_y);
	std_z = sqrt(var_z);

	// standard deviation in communication
	unsigned short adv_u16[3];

	if (convert_phy2adv_u16(offset_axis, std_x, std_y, std_z, adv_u16) == -1)
		return -1;
	if (send_packet(H2D_STILL_THR_CLASS + offset_axis, adv_u16) == -1)
		return -1;

	return 0;
}

int uno_imu_set_still_threshold_acc(float var_x, float var_y, float var_z)
{
	return set_still_threshold(OFFSET_ACC, var_x, var_y, var_z);
}

int uno_imu_set_still_threshold_gyr(float var_x, float var_y, float var_z)
{
	return set_still_threshold(OFFSET_GYR, var_x, var_y, var_z);
}

static void get_adv(unsigned char *buf, short *ad_value)
{
	for (int i = 0; i < 3; i++)
		ad_value[i] = (buf[i * 2] << 8) | buf[i * 2 + 1];	// fixed big-endian
}

static UNO_IMU_STATE parse_packet(unsigned char *buf, void **pp)
{
	unsigned char sync_byte = buf[0], packet_type_h = buf[1] & 0xF0,
			packet_type_l = buf[1] & 0x0F;

	// No.1---sync_byte
	if (sync_byte == D2H_SYNC_BYTE)
	{
		// No.2---packet_type
		switch (packet_type_h)
		{
		case D2H_DATA_CLASS:
			// No.3---packet_sub_type
			switch (packet_type_l)
			{
			case OFFSET_ACC:
			{
				short acc[3];
				get_adv(buf + IMU_HEADER_LEN, acc);
				convert_adv2phy(OFFSET_ACC, acc, &gl_adv_xyz[0], &gl_adv_xyz[1],
						&gl_adv_xyz[2]);
				*pp = gl_adv_xyz;
				if (gl_request_acc > 0)
					gl_request_acc--;
				return UNO_IMU_EVENT_ACC;
			}
			case OFFSET_GYR:
			{
				short gyr[3];
				get_adv(buf + IMU_HEADER_LEN, gyr);
				convert_adv2phy(OFFSET_GYR, gyr, &gl_adv_xyz[0], &gl_adv_xyz[1],
						&gl_adv_xyz[2]);
				*pp = gl_adv_xyz;
				if (gl_request_gyr > 0)
					gl_request_gyr--;
				return UNO_IMU_EVENT_GYR;
			}
			case OFFSET_MAG:
			{
				short mag[3];
				get_adv(buf + IMU_HEADER_LEN, mag);
				convert_adv2phy(OFFSET_MAG, mag, &gl_adv_xyz[0], &gl_adv_xyz[1],
						&gl_adv_xyz[2]);
				*pp = gl_adv_xyz;
				if (gl_request_mag > 0)
					gl_request_mag--;
				return UNO_IMU_EVENT_MAG;
			}
			}
			break;

		case D2H_MOTION_CLASS:
			return UNO_IMU_EVENT_MOTION;
			break;

		case D2H_STILL_CLASS:
			return UNO_IMU_EVENT_STILL;
			break;

		case D2H_ERROR_CLASS:
			return UNO_IMU_EXCEPTION_UNKNOWN_PACKET;
			break;
		}
	}

	return UNO_IMU_EXCEPTION_UNKNOWN_PACKET;
}

UNO_IMU_STATE uno_imu_commands_events(void **pp)
{
	//static int buf_offset = 0;	// only can be multiple of IMU_PACKET_LEN
	//static int buf_volume = 0;	// how many bytes left in the buf[]
	int read_len;
	unsigned char buf[IMU_BUFFER_LEN];
	unsigned short adv_u16[3] =
	{ 0xAABB, 0xCCDD, 0xEEFF };

	if (gl_request_acc > 0)
		send_packet(H2D_DATA_REQUEST_CLASS + OFFSET_ACC, adv_u16);

	if (gl_request_gyr > 0)
		send_packet(H2D_DATA_REQUEST_CLASS + OFFSET_GYR, adv_u16);

	if (gl_request_mag > 0)
		send_packet(H2D_DATA_REQUEST_CLASS + OFFSET_MAG, adv_u16);

#if 1
	if (gl_imu_usb_type == UNO_IMU_USB_CDC)
	{
		read_len = read(gl_fd, buf, IMU_PACKET_LEN);

		if (read_len == -1)	// ??? when the USB channel read nothing
			return UNO_IMU_NO_EVENT;
		if (read_len == 0)	// ??? when the USB cable unplugged
			return UNO_IMU_EXCEPTION_UNPLUG;
	}
	else if (gl_imu_usb_type == UNO_IMU_USB_CUSTOM)
	{
		libusb_bulk_transfer(gl_device_handle, (1 | LIBUSB_ENDPOINT_IN), buf,
		USB_EP1_IN_SIZE, &read_len, 1);
#if DEBUG
		static int counter = 0;
		printf("%d: IN(%d)\n", counter++, read_len);
#endif
		if (read_len == 0)	// ??? when the USB channel read nothing
			return UNO_IMU_NO_EVENT;
	}

	if (read_len == IMU_PACKET_LEN)
		return parse_packet(buf, pp);
	else
		return UNO_IMU_EXCEPTION_UNKNOWN_PACKET;
#else
	int read_len = read(gl_fd, buf + buf_offset + buf_volume, IMU_PACKET_LEN);
	if (read_len == 0)	// IMU detached!
	return uno_imu_error;
	if (read_len > 0)// read something
	{
		buf_volume += read_len;
		if (buf_offset + buf_volume > IMU_BUFFER_LEN)
		return uno_imu_error;
	}

	if (buf_offset > 0)	// move to the front
	memcpy(buf, buf + buf_offset, buf_volume);
	buf_offset = 0;

	// it is ensured: there are buf_volume bytes start from buf[0]
	if (buf_volume < IMU_PACKET_LEN)
	return uno_imu_no_event;
	else if (buf_volume == IMU_PACKET_LEN)
	{
		buf_volume = 0;
		buf_offset = 0;
		return parse_packet(buf);
	}
	else	// buf_offset > IMU_PACKET_LEN)
	{
		buf_volume -= IMU_PACKET_LEN;
		buf_offset = IMU_PACKET_LEN;
		return parse_packet(buf);
	}
#endif
}

int uno_imu_init(void)
{
	if (imu_open() == -1)
		return -1;

	float motion_threshold_acc[3] =
	{ 0.2, 0.5, 0.2 };	// specifically consider the horizontal plane
	float motion_threshold_gyr[3] =
	{ 0.5, 0.1, 0.5 };// specifically consider the rotation with the axis vertical to the horizontal plane
	uno_imu_set_motion_threshold_acc(motion_threshold_acc[0],
			motion_threshold_acc[1], motion_threshold_acc[2]);
	uno_imu_set_motion_threshold_gyr(motion_threshold_gyr[0],
			motion_threshold_gyr[1], motion_threshold_gyr[2]);

	float variance_threshold_acc[3] =
	{ 0.0001, 0.0001, 0.0001 };	// may not be bigger than 0.0001 for each
	float variance_threshold_gyr[3] =
	{ 0.0001, 0.0001, 0.0001 };	// may not be bigger than 0.001 for each
	uno_imu_set_still_threshold_acc(variance_threshold_acc[0],
			variance_threshold_acc[1], variance_threshold_acc[2]);
	uno_imu_set_still_threshold_gyr(variance_threshold_gyr[0],
			variance_threshold_gyr[1], variance_threshold_gyr[2]);

	return 0;
}

int uno_imu_uninit(void)
{
	if (gl_imu_usb_type == UNO_IMU_USB_CDC)
		return close(gl_fd);
	else if (gl_imu_usb_type == UNO_IMU_USB_CUSTOM)
	{
		libusb_close(gl_device_handle);
		libusb_exit(gl_libusb_context);
		return 0;
	}
	else
		return -1;
}

int uno_imu_request_acc(void)
{
	if (gl_request_acc > REQUEST_OVERFLOW)
		return -1; // overflow
	gl_request_acc++;
	return 0;
}

int uno_imu_request_gyr(void)
{
	if (gl_request_gyr > REQUEST_OVERFLOW)
		return -1; // overflow
	gl_request_gyr++;
	return 0;
}

int uno_imu_request_mag(void)
{
	if (gl_request_mag > REQUEST_OVERFLOW)
		return -1; // overflow
	gl_request_mag++;
	return 0;
}

int uno_imu_get_acc(float *x, float *y, float *z)
{
	int count = GET_TIMEOUT_MS / GET_USLEEP_MS;

	int ret = uno_imu_request_acc();
	if (ret < 0)
		return ret;

	
	while (gl_request_acc != 0 && count > 0)
	{
		usleep(1000 * GET_USLEEP_MS);
		count--;
	} 



	if (gl_request_acc == 0) //response got
	{
		*x = gl_adv_xyz[0];
		*y = gl_adv_xyz[1];
		*z = gl_adv_xyz[2];

		return 0;
	}
	else
		return -2; //timeout
}

int uno_imu_get_gyr(float *x, float *y, float *z)
{
	int count = GET_TIMEOUT_MS / GET_USLEEP_MS;

	int ret = uno_imu_request_gyr();
	if (ret < 0)
		return ret;

	while (gl_request_gyr != 0 && count > 0)
	{
		usleep(1000 * GET_USLEEP_MS);
		count--;
	}

	if (gl_request_gyr == 0) //response got
	{
		*x = gl_adv_xyz[0];
		*y = gl_adv_xyz[1];
		*z = gl_adv_xyz[2];

		return 0;
	}
	else
		return -2; //timeout
}

int uno_imu_get_mag(float *x, float *y, float *z)
{
	int count = GET_TIMEOUT_MS / GET_USLEEP_MS;

	int ret = uno_imu_request_mag();
	if (ret < 0)
		return ret;

	while (gl_request_mag != 0 && count > 0)
	{
		usleep(1000 * GET_USLEEP_MS);
		count--;
	}

	if (gl_request_mag == 0) //response got
	{
		*x = gl_adv_xyz[0];
		*y = gl_adv_xyz[1];
		*z = gl_adv_xyz[2];

		return 0;
	}
	else
		return -2; //timeout
}

void radian_from_acc(float acc_x, float acc_y, float acc_z,
		float *acc_x2plumb_line, float *acc_y2plumb_line,
		float *acc_z2plumb_line)
{
	*acc_x2plumb_line = atan2(sqrt(acc_y * acc_y + acc_z * acc_z), acc_x);
	*acc_y2plumb_line = atan2(sqrt(acc_x * acc_x + acc_z * acc_z), acc_y);
	*acc_z2plumb_line = atan2(sqrt(acc_x * acc_x + acc_y * acc_y), acc_z);
}

int uno_imu_get_version(unsigned char *version)
{
	unsigned char ret;
	ret = libusb_control_transfer(gl_device_handle, USB_EP0_IN + USB_VENDOR, GET_VERSION, 0, 0, version, USB_EP0_SIZE_VERSION, 0);
	if (ret != USB_EP0_SIZE_VERSION){
		printf("EP0_IN: read data failed ! %d\n", ret);
		return -1;
	}
	return 0;

}

int uno_imu_get_sn(unsigned char *sn_data)
{
	unsigned char ret;
	ret = libusb_control_transfer(gl_device_handle, USB_EP0_IN + USB_VENDOR, GET_SN, 0, 0, sn_data, USB_EP0_SIZE_SN, 0);
	if (ret != USB_EP0_SIZE_SN){
		printf("EP0_IN: read data failed ! %d\n", ret);
		return -1;
	}
	return 0;

}

int uno_imu_set_sn(unsigned char *sn_data)
{
	unsigned char ret;
	ret = libusb_control_transfer(gl_device_handle, USB_EP0_OUT + USB_VENDOR, SET_SN, 0, 0, sn_data, USB_EP0_SIZE_SN, 0);
	if (ret != USB_EP0_SIZE_SN){
		printf("EP0_IN: write data failed ! %d\n", ret);
		return -1;
	}
	return 0;
}
