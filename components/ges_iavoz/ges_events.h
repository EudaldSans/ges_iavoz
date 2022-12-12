#pragma once

#ifndef __cplusplus
# include <stdatomic.h>
#else
# include <atomic>
# define _Atomic(X) std::atomic< X >
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "freertos/FreeRTOS.h"

#include "esp_err.h"
#include "esp_event_base.h"
#include "esp_log.h"

#include "ges_config.h"
#include "ges_connect.h"

esp_event_loop_handle_t events_audio_loop_h, events_conn_loop_h;

ESP_EVENT_DECLARE_BASE(EVENTS_AUDIO);
enum {
    VAD_START,
    EVENT_AUDIO_FRAME,
    EVENT_AUDIO_FINISHED
};

ESP_EVENT_DECLARE_BASE(EVENTS_CONN);
enum {
    EVENT_CONN_SYNC = mt_SYNCHRONIZATION,
    EVENT_CONN_AUDIO_REQ = mt_AUDIO_REQUEST,
    EVENT_CONN_AUDIO_STOP = mt_AUDIO_STOP,
    CONN_START_OF_STREAMING_MODE = mt_START_OF_STREAMING_MODE,
    CONN_STOP_OF_STREAMING_MODE = mt_STOP_OF_STREAMING_MODE,
    EVENT_CONN_WEIGHTS_TRANSMISSION = mt_WEIGHTS,
    EVENT_CONN_DROPPED = 99
}; 


void events_init ( void );
void events_audio_handler ( void* handler_arg, esp_event_base_t base, int32_t id, void* event_data );
void events_conn_handler ( void* handler_arg, esp_event_base_t base, int32_t id, void* event_data );


#ifdef __cplusplus
}
#endif