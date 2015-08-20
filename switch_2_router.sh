sudo ./create_ap --stop wlan0
wpa_cli -iwlan0 disconnect
sudo wpa_cli terminate
sudo wpa_supplicant -d -Dnl80211 -c/etc/wpa_supplicant.conf -iwlan0 -B
wpa_cli -iwlan0 disconnect
for i in `wpa_cli -iwlan0 list_networks | grep ^[0-9] | cut -f1`; do wpa_cli -iwlan0 remove_network $i; done
wpa_cli -iwlan0 add_network
#wpa_cli -iwlan0 set_network 0 auth_alg OPEN
#wpa_cli -iwlan0 set_network 0 key_mgmt WPA-PSK
wpa_cli -iwlan0 set_network 0 psk \""$2"\"
#wpa_cli -iwlan0 set_network 0 psk '"topgear123"'
#wpa_cli -iwlan0 set_network 0 pairwise CCMP TKIP
#wpa_cli -iwlan0 set_network 0 group CCMP TKIP
#wpa_cli -iwlan0 set_network 0 mode 0
wpa_cli -iwlan0 set_network 0 ssid \""$1"\"
wpa_cli -iwlan0 select_network 0
#wpa_cli -iwlan0 enable_network 0
#wpa_cli -iwlan0 reassociate
#wpa_cli -iwlan0 status
#dhclient wlan0

#然后调用wpa_cli status看反馈是CONNECTED 还是 SCANNING ， 密码错的时候也是提示的asocciated。成功会提示：wpa_state=COMPLETED
#如果是CONNECTED，则进行sudo dhclient wlan0 来获得ip。
