#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>
// #include "addr_from_stdin.h"
#include <math.h>

#define HOST_IP_ADDR        "192.168.1.99"//"172.20.10.3"//"10.197.1.15"//"192.168.1.183"//183"//"192.168.43.113"//"192.168.1.99"//"172.20.10.3"
#define MULTICAST_IPV4_ADDR "239.0.0.21"
#define MULTICAST_TTL       10
#define PORT                3333

void connect_init();

void start_tcp(in_addr_t ip, uint16_t port);
int send_tcp(void* payload, size_t size);

void start_udp();
int send_payload(void* payload, size_t size);


#ifdef __cplusplus
}
#endif
