#ifndef __WifiConfig_h__
#define __WifiConfig_h__

#define ESP_WIFI_SSID "ESP_WIFI_MUTY"
#define ESP_WIFI_PASS "mypassword"
#define ESP_WIFI_CHANNEL 5
#define MAX_STA_CONN 10

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1
#define ESP_MAXIMUM_RETRY 10

#define ESP_AP_IP_ADDRESS "192.168.0.100"
#define ESP_AP_MASK_ADDRESS "255.255.255.0"
#define ESP_AP_GW_ADDRESS "192.168.0.100"

#endif //__WifiConfig_h__