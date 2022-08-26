#include "freertos/FreeRTOS.h"
#include "esp_log.h"

#include "ges_iavoz.h"

static const char * TAG = "Main";

void iavoz_callback_dummy ( IAVOZ_KEY_t xKeyWord, uint64_t uiPower)
{
    ESP_LOGI(TAG, "Received %d with %llu", (int) xKeyWord, uiPower);
}

void app_main ( void )
{
    ESP_LOGI(TAG, "Starting System");
    IAVOZ_Init(1, iavoz_callback_dummy);
}
