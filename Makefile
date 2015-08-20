TARGET = unosdktest
CC = gcc
CFLAGS = -Wall -pthread -O3 -I include -I driver -std=gnu99
SOURCES = $(wildcard *.c)
SOURCES += $(wildcard driver/*.c)
SOURCES += $(wildcard driver/i2c/*.c)

# ar rvl libuno_sdk.a driver/uno_rs485.o driver/gpio_ctrl.o driver/uno_alarm.o driver/uno_led.o driver/uno_imu.o
# gcc main.c -o main -static -L. -luno_sdk

OBJS = $(patsubst %.c, %.o, $(SOURCES))
INCLUDES =
LIBS = -lm -pthread -lusb-1.0

%.o : %.c
	$(CC) $(CFLAGS) -c $(INCLUDES) $< -o $@
all: $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $^ $(LIBS)

unolib:  driver/uno_rs485.o driver/gpio_ctrl.o driver/uno_alarm.o driver/uno_led.o driver/uno_imu.o
	ar rvl libuno_sdk.a driver/uno_rs485.o driver/gpio_ctrl.o driver/uno_alarm.o driver/uno_led.o driver/uno_imu.o	
	
clean:
	rm -f $(OBJS)
	rm -f $(TARGET)

