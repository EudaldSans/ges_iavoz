#include "ges_connect.h"
// #include "events.h"

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

/* The examples use WiFi configuration that you can set via project configuration menu

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/
#define ESP_WIFI_SSID      CONFIG_ESP_WIFI_SSID
#define ESP_WIFI_PASS      CONFIG_ESP_WIFI_PASSWORD
#define ESP_MAXIMUM_RETRY  CONFIG_ESP_MAXIMUM_RETRY

#if CONFIG_ESP_WIFI_AUTH_OPEN
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_OPEN
#elif CONFIG_ESP_WIFI_AUTH_WEP
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WEP
#elif CONFIG_ESP_WIFI_AUTH_WPA_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA2_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA_WPA2_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_WPA2_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA3_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA3_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA2_WPA3_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_WPA3_PSK
#elif CONFIG_ESP_WIFI_AUTH_WAPI_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WAPI_PSK
#endif

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static int s_retry_num = 0;

static const char* TAG = "[connect]";

const char* IPHOST = HOST_IP_ADDR;

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG,"connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}


void wifi_init_sta(void) {
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = ESP_WIFI_SSID,
            .password = ESP_WIFI_PASS,
            /* Setting a password implies station will connect to all security modes including WEP/WPA.
             * However these modes are deprecated and not advisable to be used. Incase your Access point
             * doesn't support WPA2, these mode can be enabled by commenting below line */
	     .threshold.authmode = ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                 ESP_WIFI_SSID, ESP_WIFI_PASS);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                 ESP_WIFI_SSID, ESP_WIFI_PASS);
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }
}

void connect_init(){
    ESP_LOGI(TAG, "Initialising connection module");

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    wifi_init_sta();

    // ESP_ERROR_CHECK(nvs_flash_init());
    // ESP_ERROR_CHECK(esp_netif_init());
    // // ESP_ERROR_CHECK(esp_event_loop_create_default());

    // // ESP_ERROR_CHECK(example_connect());
    // start_udp();

    // in_addr_t new_addr = inet_addr((char *) IPHOST);
    // start_tcp(new_addr);
};


// TCP
static const char* TAG_TCP = "[socket TCP]";
int tcp_port = PORT;
int tcp_sock = 0;
struct sockaddr_in tcp_dest_addr;
SemaphoreHandle_t tcp_mutex = NULL;


void init_tcp(in_addr_t ip, uint16_t port){
    tcp_dest_addr.sin_addr.s_addr = ip;
    tcp_dest_addr.sin_family = AF_INET;
    tcp_dest_addr.sin_port = htons(port);

    tcp_sock =  socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_sock < 0) {
        ESP_LOGE(TAG_TCP, "Unable to create socket: errno %d", errno);
        return;
    }

    ESP_LOGI(TAG_TCP, "Socket created, connecting to %s:%d", inet_ntoa(ip), port);

    int err = connect(tcp_sock, (struct sockaddr *)&tcp_dest_addr, sizeof(struct sockaddr_in));
    if (err != 0) {
        ESP_LOGE(TAG_TCP, "Socket unable to connect: errno %d", errno);
        return;
    }
    ESP_LOGI(TAG_TCP, "Successfully connected");

    tcp_mutex = xSemaphoreCreateMutex();
    if (tcp_mutex == NULL) ESP_LOGI(TAG_TCP, "Failed to create TCP mutex");
}

int send_tcp(void* payload, size_t size){
    int err = 0;
    if(xSemaphoreTake(tcp_mutex, portMAX_DELAY)){
        err = send(tcp_sock, payload, size, 0);
        xSemaphoreGive(tcp_mutex);
    }
    if(err < 0) ESP_LOGE(TAG_TCP, "tcp send failed");
    return err;
}

void recv_task_tcp(void *param) {
    ESP_LOGI(TAG_TCP, "TCP Receiver task started");
    uint8_t tcp_rx_buffer[128];
    uint8_t nerr = 0;
    while(true){
        int len = recv(tcp_sock, tcp_rx_buffer, sizeof(tcp_rx_buffer) - 1, 0);
        if (len < 0) {
            ESP_LOGE(TAG_TCP, "tcp recv failed: errno %d", errno);
            nerr++;
            if(nerr > 10){
                shutdown(tcp_sock, 0);
                close(tcp_sock);
                break;
            }
        } else {
            nerr = 0;
            // esp_event_post_to(events_conn_loop_h, EVENTS_CONN, tcp_rx_buffer[0], &tcp_rx_buffer[1], sizeof(char)*len - 1, portMAX_DELAY);
        }
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
    // esp_event_post_to(events_conn_loop_h, EVENTS_CONN, EVENT_CONN_DROPPED, NULL, 0, portMAX_DELAY);
    vTaskDelete(NULL);
}

void start_tcp(in_addr_t ip, uint16_t port){
    init_tcp(ip, port);
    xTaskCreate(&recv_task_tcp, "TCP", 4096, NULL, 5, NULL);
}

// UDP Multicast
static const char* TAG_UDP = "[socket udp]";
char udp_host_ip[] = MULTICAST_IPV4_ADDR;
int udp_port = PORT;
int udp_addr_family = AF_INET;
int udp_ip_protocol = IPPROTO_IP;
int udp_sock = 0;
struct sockaddr_in udp_dest_addr;
SemaphoreHandle_t udp_mutex = NULL;

int init_udp() {
    udp_dest_addr.sin_addr.s_addr = inet_addr(udp_host_ip);
    udp_dest_addr.sin_family = AF_INET;
    udp_dest_addr.sin_port = htons(udp_port);

    udp_mutex = xSemaphoreCreateMutex();
    if (udp_mutex == NULL) ESP_LOGI(TAG_UDP, "Failed to create UDP M mutex");

    struct sockaddr_in saddr = {0};
    int err = 0;

    udp_sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (udp_sock < 0) {
        ESP_LOGE(TAG_UDP, "Failed to create socket. Error %d", errno);
        return -1;
    }

    // Bind the socket to any address
    saddr.sin_family = PF_INET;
    saddr.sin_port = htons(udp_port);
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);
    err = bind(udp_sock, (struct sockaddr *)&saddr, sizeof(struct sockaddr_in));
    if (err < 0) {
        ESP_LOGE(TAG_UDP, "Failed to bind socket. Error %d", errno);
        goto err;
    }

    // Assign multicast TTL (set separately from normal interface TTL)
    uint8_t ttl = MULTICAST_TTL;
    setsockopt(udp_sock, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(uint8_t));
    if (err < 0) {
        ESP_LOGE(TAG_UDP, "Failed to set IP_MULTICAST_TTL. Error %d", errno);
        goto err;
    }

    // set destination multicast addresses for sending from these sockets
    struct sockaddr_in sdestv4 = {
        .sin_family = PF_INET,
        .sin_port = htons(udp_port),
    };
    // We know this inet_aton will pass because we did it above already
    inet_aton(udp_host_ip, &sdestv4.sin_addr.s_addr);

    struct ip_mreq imreq = { 0 };
    struct in_addr iaddr = { 0 };

    // Configure source interface
    imreq.imr_interface.s_addr = IPADDR_ANY;

    // Configure multicast address to listen to
    err = inet_aton(udp_host_ip, &imreq.imr_multiaddr.s_addr);
    if (err != 1) {
        ESP_LOGE(TAG_UDP, "Configured IPV4 multicast address '%s' is invalid.", udp_host_ip);
        goto err;
    }
    ESP_LOGI(TAG_UDP, "Configured IPV4 Multicast address %s", inet_ntoa(imreq.imr_multiaddr.s_addr));
    if (!IP_MULTICAST(ntohl(imreq.imr_multiaddr.s_addr))) {
        ESP_LOGW(TAG_UDP, "Configured IPV4 multicast address '%s' is not a valid multicast address. This will probably not work.", udp_host_ip);
    }

    // Assign the IPv4 multicast source interface, via its IP
    // (only necessary if this socket is IPV4 only)
    err = setsockopt(udp_sock, IPPROTO_IP, IP_MULTICAST_IF, &iaddr,
                        sizeof(struct in_addr));
    if (err < 0) {
        ESP_LOGE(TAG_UDP, "Failed to set IP_MULTICAST_IF. Error %d", errno);
        goto err;
    }

    err = setsockopt(udp_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                         &imreq, sizeof(struct ip_mreq));
    if (err < 0) {
        ESP_LOGE(TAG_UDP, "Failed to set IP_ADD_MEMBERSHIP. Error %d", errno);
        goto err;
    }

    return ESP_OK;
 
 err:
    close(udp_sock);
    return err;
}


void recv_task_udp(void *param) {
    ESP_LOGI(TAG_UDP, "UDP receiver task started");
    char recvbuf[48];
    struct sockaddr_in raddr;
    socklen_t socklen = sizeof(raddr);
    while(true){
        int len = recvfrom(udp_sock, recvbuf, sizeof(recvbuf) - 1, 0, (struct sockaddr *)&raddr, &socklen);
        if (len < 0) {
            ESP_LOGE(TAG_UDP, "multicast recvfrom failed: errno %d", errno);
        } else {
            // esp_event_post_to(events_conn_loop_h, EVENTS_CONN, recvbuf[0], recvbuf + 1, sizeof(char)*len, portMAX_DELAY);
        }
    }
}

int send_payload(void* payload, size_t size){
    int err = 0;
    if(xSemaphoreTake(udp_mutex, portMAX_DELAY)){
        err = sendto(udp_sock, payload, size, 0, (struct sockaddr *)&udp_dest_addr, sizeof(udp_dest_addr));
        xSemaphoreGive(udp_mutex);
    }
    if(err < 0) ESP_LOGE(TAG_TCP, "udp send failed");
    return err;
}

void start_udp(){
    ESP_ERROR_CHECK(init_udp());
    xTaskCreate(&recv_task_udp, "UDP", 4096, NULL, 5, NULL);
}