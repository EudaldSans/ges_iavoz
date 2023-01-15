/* Copyright 2019 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "driver/gpio.h"
#include <esp_log.h>

#include "ges_iavoz_command_responder.h"

#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_OUTPUT_IO          GPIO_NUM_3 // Define the output GPIO
#define LEDC_CHANNEL            LEDC_CHANNEL_0
#define LEDC_DUTY_RES           LEDC_TIMER_13_BIT // Set duty resolution to 13 bits
#define LEDC_DUTY               (4095) // Set duty to 50%. ((2 ** 13) - 1) * 50% = 4095
#define LEDC_FREQUENCY          (1500) // Frequency in Hertz. Set frequency at 5 kHz

#define GPIO_BUZZER_ENABLE      GPIO_NUM_1
#define GPIO_RELE               GPIO_NUM_4
#define GPIO_LED                GPIO_NUM_23
#define GPIO_OUTPUT_PIN_SEL     ((1ULL<<GPIO_BUZZER_ENABLE) | (1ULL<<GPIO_RELE) | (1ULL<<GPIO_LED))
// #define GPIO_OUTPUT_PIN_SEL     (1ULL<<GPIO_BUZZER_ENABLE)

#define BEEP
// #define USE_LED


uint32_t activations = 0;
uint32_t first_commands = 0;
uint32_t second_commands = 0;
uint8_t status = 0;

const char * RESPONDER_TAG = "IAVOZ_RESPONDER";

void beep(void) {
	// ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, LEDC_DUTY));
	// ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));

	// vTaskDelay(100/portTICK_RATE_MS);
	
	// ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 0));
	// ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));

	// vTaskDelay(100/portTICK_RATE_MS);
}

void enciende(void) {
    if (status == 1) return;

    ESP_LOGI(RESPONDER_TAG, "Enciende!!"); 
    status = 1;
    return;
    
	// gpio_set_level(GPIO_BUZZER_ENABLE, 1);
    // gpio_set_level(GPIO_LED, 1);
    // gpio_set_level(GPIO_RELE, 1);
	// beep();
	// gpio_set_level(GPIO_BUZZER_ENABLE, 0);
}

void apaga(void) {
    if (status == 0) return;
    
    ESP_LOGI(RESPONDER_TAG, "Apaga!!");
    status = 0;
    return;

	// gpio_set_level(GPIO_BUZZER_ENABLE, 1);
    // gpio_set_level(GPIO_RELE, 0);
    // gpio_set_level(GPIO_LED, 0);
	// beep();
	// beep();
	// gpio_set_level(GPIO_BUZZER_ENABLE, 0);
}

void ayuda(void) {
    ESP_LOGI(RESPONDER_TAG, "Ayuda!!");
    return;

	// gpio_set_level(GPIO_BUZZER_ENABLE, 1);
    // gpio_set_level(GPIO_LED, !status);
	// beep();
    // gpio_set_level(GPIO_LED, status);
	// beep();
    // gpio_set_level(GPIO_LED, !status);
	// beep();
    // gpio_set_level(GPIO_LED, status);
	// gpio_set_level(GPIO_BUZZER_ENABLE, 0);
}

// The default implementation writes out the name of the recognized command
// to the error console. Real applications will want to take some custom
// action instead, and should implement their own versions of this function.
void RespondToCommand(IAVOZ_KEY_t found_command) {
    ESP_LOGI(RESPONDER_TAG, "Responding to command: %d", found_command);
    // return;

    switch (found_command) {
        case IAVOZ_KEY_HEYLOLA: return;
        case IAVOZ_KEY_ENCIENDE: enciende(); break;
        case IAVOZ_KEY_APAGA: apaga(); break;
        case IAVOZ_KEY_SOCORRO: ayuda(); break;
        default:
            ESP_LOGW(RESPONDER_TAG, "Received unknown command %d", found_command);
    }
}

void initCommandResponder() {
    return;

	// //zero-initialize the config structure.
    // gpio_config_t io_conf = {};
    // //disable interrupt
    // io_conf.intr_type = GPIO_INTR_DISABLE;
    // //set as output mode
    // io_conf.mode = GPIO_MODE_OUTPUT;
    // //bit mask of the pins that you want to set,e.g.GPIO18/19
    // io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    // //disable pull-down mode
    // io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    // //disable pull-up mode
    // io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    // //configure GPIO with the given settings
    // gpio_config(&io_conf);

    // gpio_set_level(GPIO_BUZZER_ENABLE, 1);
    // ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, LEDC_DUTY));
    // ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));

	// vTaskDelay(100/portTICK_RATE_MS);
    // ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 0));
    // ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));
	// gpio_set_level(GPIO_BUZZER_ENABLE, 0);

	// // Prepare and then apply the LEDC PWM timer configuration
    // ledc_timer_config_t ledc_timer = {
    //     .speed_mode       = LEDC_MODE,
    //     .duty_resolution  = LEDC_DUTY_RES,
    //     .timer_num        = LEDC_TIMER,
    //     .freq_hz          = LEDC_FREQUENCY,  // Set output frequency at 5 kHz
    //     .clk_cfg          = LEDC_AUTO_CLK
    // };
    // ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // // Prepare and then apply the LEDC PWM channel configuration
    // ledc_channel_config_t ledc_channel = {
    //     .gpio_num       = LEDC_OUTPUT_IO,
    //     .speed_mode     = LEDC_MODE,
    //     .channel        = LEDC_CHANNEL,
    //     .intr_type      = LEDC_INTR_DISABLE,
    //     .timer_sel      = LEDC_TIMER,
    //     .duty           = 0, // Set duty to 0%
    //     .hpoint         = 0
    // };
    // ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));

    // beep();
}
