#!/bin/bash
./create_ap --stop wlan0
# sleep to avoid "unhandle" error
sleep 1
wpa_cli terminate
sleep 1
ifdown wlan0
ifconfig wlan0 down
ifconfig wlan0 up
echo performance > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
null_string='NULL'
if [ "$2" != "$null_string" ];
then
	./create_ap -n wlan0 $1 $2 -c $3 --daemon
else
	./create_ap -n wlan0 $1 -c $3 --daemon
fi
