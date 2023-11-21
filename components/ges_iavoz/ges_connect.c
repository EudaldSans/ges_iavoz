#include "ges_connect.h"
#include "ges_events.h"

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"

/* The examples use WiFi configuration that you can set via project configuration menu

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/
#define ESP_WIFI_SSID CONFIG_ESP_WIFI_SSID
#define ESP_WIFI_PASS CONFIG_ESP_WIFI_PASSWORD
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
#define KEEPALIVE_IDLE              CONFIG_EXAMPLE_KEEPALIVE_IDLE
#define KEEPALIVE_INTERVAL          CONFIG_EXAMPLE_KEEPALIVE_INTERVAL
#define KEEPALIVE_COUNT             CONFIG_EXAMPLE_KEEPALIVE_COUNT

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

    start_tcp();
};


// TCP
static const char* TAG_TCP = "[socket TCP]";
int tcp_port = PORT;
int tcp_sock = 0;
int ip_protocol = 0;
struct sockaddr_in tcp_dest_addr;
SemaphoreHandle_t tcp_mutex = NULL;
char host_ip[] = HOST_IP_ADDR;
// int keepAlive = 1;
// int keepIdle = KEEPALIVE_IDLE;
// int keepInterval = KEEPALIVE_INTERVAL;
// int keepCount = KEEPALIVE_COUNT;

static void client_task(void *pvParameters)
{
    int client_sock = (int)pvParameters;
    ESP_LOGI(TAG_TCP, "TCP Receiver task started");
    uint8_t tcp_rx_buffer[128];
    uint8_t nerr = 0;
    while(true){
        int len = recv(client_sock, tcp_rx_buffer, sizeof(tcp_rx_buffer) - 1, 0);
        if (len < 0) {
            ESP_LOGE(TAG_TCP, "tcp recv failed: errno %d", errno);
            nerr++;
            if(nerr > 10){
                shutdown(client_sock, 0);
                close(client_sock);
                break;
            }
        } else {
            nerr = 0;
            esp_event_post_to(events_conn_loop_h, EVENTS_CONN, tcp_rx_buffer[0], &tcp_rx_buffer[1], sizeof(char)*len - 1, portMAX_DELAY);
        }
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
    esp_event_post_to(events_conn_loop_h, EVENTS_CONN, EVENT_CONN_DROPPED, NULL, 0, portMAX_DELAY);
    vTaskDelete(NULL);
}


void init_tcp(){
    tcp_dest_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    tcp_dest_addr.sin_family = AF_INET;
    tcp_dest_addr.sin_port = htons(PORT);

    tcp_sock =  socket(tcp_dest_addr.sin_family, SOCK_STREAM, ip_protocol);
    if (tcp_sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        vTaskDelete(NULL);
        return;
    }

    int opt = 1;
    setsockopt(tcp_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    ESP_LOGI(TAG_TCP, "Socket created, connecting to %s:%d", host_ip, PORT);

    int err = bind(tcp_sock, (struct sockaddr *)&tcp_dest_addr, sizeof(tcp_dest_addr));
    if (err != 0) {
        ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        // ESP_LOGE(TAG, "IPPROTO: %d", addr_family);
        close(tcp_sock);
        vTaskDelete(NULL);
    }
    ESP_LOGI(TAG, "Socket bound, port %d", PORT);

    tcp_mutex = xSemaphoreCreateMutex();
    if (tcp_mutex == NULL) ESP_LOGI(TAG_TCP, "Failed to create TCP mutex");
}

void tcp_server_task(void *param) {
    char addr_str[128];
    int err = listen(tcp_sock, 1);
    if (err != 0) {
        ESP_LOGE(TAG_TCP, "Error occurred during listen: errno %d", errno);
        close(tcp_sock);
        vTaskDelete(NULL);
    }
    ESP_LOGI(TAG_TCP, "TCP Receiver task started");
    uint8_t tcp_rx_buffer[128];
    uint8_t nerr = 0;
    while (1) {
        ESP_LOGI(TAG_TCP, "Socket listening");

        while (1) {
            struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
            socklen_t addr_len = sizeof(source_addr);
            int sock = accept(tcp_sock, (struct sockaddr *)&source_addr, &addr_len);
            if (sock < 0) {
                ESP_LOGE(TAG_TCP, "Unable to accept connection: errno %d", errno);
                break;
            }

            // Set tcp keepalive option
            // setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &keepAlive, sizeof(int));
            // setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, &keepIdle, sizeof(int));
            // setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, &keepInterval, sizeof(int));
            // setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, &keepCount, sizeof(int));
            // Convert ip address to string
            if (source_addr.ss_family == PF_INET) {
                inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr, addr_str, sizeof(addr_str) - 1);
            }
            ESP_LOGI(TAG_TCP, "Socket accepted ip address: %s", addr_str);
            
            if (xTaskCreate(client_task, "client", 4096, (void*)(intptr_t)sock, 5, NULL) != pdPASS) {
                ESP_LOGE(TAG, "Failed to create client task");
                close(sock);
            }
        }
    }
    vTaskDelete(NULL);
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

void start_tcp(){
    init_tcp();
    xTaskCreate(tcp_server_task, "tcp_server", 4096, (void*)AF_INET, 5, NULL);
}