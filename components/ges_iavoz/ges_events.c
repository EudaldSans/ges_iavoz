#include "ges_events.h"

#include <string.h>
#include "ges_iavoz.h"

static const char* TAG = "[events]";

enum assistantState {
    IDLE = 0,
    LISTENING,
    EXPECTING,
    THINKING,
    SPEAKING,
    FINISHED
};

// Loop elements
esp_event_loop_args_t events_audio_loop_args = {
        .queue_size = 5,
        .task_name = "events_audio_loop",
        .task_priority = 5,
        .task_stack_size = 4096,
        .task_core_id = tskNO_AFFINITY
};
ESP_EVENT_DEFINE_BASE(EVENTS_AUDIO);

esp_event_loop_args_t events_conn_loop_args = {
        .queue_size = 10,
        .task_name = "events_conn_loop",
        .task_priority = 5,
        .task_stack_size = 4096,
        .task_core_id = tskNO_AFFINITY
};
ESP_EVENT_DEFINE_BASE(EVENTS_CONN);

// Sender variables
uint8_t sequence_num = 0;
int32_t frames_to_send = -1;
atomic_bool cs_hub_available, cs_permission_to_send;


void events_init ( void ) {
    cs_hub_available = false;
    cs_permission_to_send = false;

    esp_event_loop_create(&events_audio_loop_args, &events_audio_loop_h);
    esp_event_loop_create(&events_conn_loop_args, &events_conn_loop_h);

    esp_event_handler_register_with(events_audio_loop_h, EVENTS_AUDIO, ESP_EVENT_ANY_ID, events_audio_handler, NULL);
    esp_event_handler_register_with(events_conn_loop_h, EVENTS_CONN, ESP_EVENT_ANY_ID, events_conn_handler, NULL);
    ESP_LOGI(TAG, "Event loops created");
}

// Handler for the audio events
void events_audio_handler(void* handler_arg, esp_event_base_t base, int32_t id, void* event_data) {
    switch(id){

        case VAD_START: {
            IAVOZ_KEY_t found_index = *(IAVOZ_KEY_t*) event_data;
            ESP_LOGI(TAG, "Voice detection STARTED with event: %d, %p", found_index, event_data);
            if (!cs_hub_available) {return;}
            
            
            uint8_t payload[5];
            payload[0] = mt_AUDIO_DETECTED;
            payload[1] = 0;
            payload[2] = 5;
            payload[3] = sequence_num;
            payload[4] = found_index,
            
            send_tcp(payload, 5);
            sequence_num++;

        } break;

        // New sound frame to add to buffer or send
        case EVENT_AUDIO_FRAME: {
            ESP_LOGV(TAG, "Sending frame %d", sequence_num);
            if ( cs_permission_to_send && frames_to_send != 0) {
                uint8_t payload[4 + AUDIO_FRAME_SAMPLES * MIC_CH_NUM * sizeof(int16_t)];
                payload[0] = mt_AUDIO_DATA;
                payload[1]=((4 + AUDIO_FRAME_SAMPLES * MIC_CH_NUM * sizeof(int16_t)) >> 8);
                payload[2]=(4 + AUDIO_FRAME_SAMPLES * MIC_CH_NUM * sizeof(int16_t)) & 0xff;
                payload[3] = sequence_num++;
                memcpy(&payload[4], event_data, AUDIO_FRAME_SAMPLES * MIC_CH_NUM * sizeof(int16_t));

                int16_t* samples = (int16_t*) event_data;

                for (int sample= 0; sample < AUDIO_FRAME_SAMPLES; sample++) {
                    printf("%d ", samples[sample]);
                }

                printf("\n");

                send_tcp(payload, sizeof(payload));
                if ( frames_to_send > 0 ) {
                    frames_to_send--;
                }
            }
        } break;

        case EVENT_AUDIO_FINISHED: {
            
            if (!cs_hub_available) {return;}
            
            uint8_t payload[3];
            payload[0] = mt_AUDIO_FINISHED;
            payload[1] = 0;
            payload[2] = 3;

            send_tcp(payload, 3);
        }

        default: {
        }
    }
}

// Handler for the general connection events
void events_conn_handler (void* handler_arg, esp_event_base_t base, int32_t id, void* event_data) {
    ESP_LOGV(TAG, "New communication %d", id);
    uint8_t MAC[6];
    switch(id){
        case EVENT_CONN_SYNC:{
            if ( cs_hub_available ){return;}
            
            ESP_LOGI(TAG, "Syncronization message received");

            uint16_t port = *((uint16_t*)event_data);
            const char *ip = ((char *) event_data) + 2;

            // Start TCP conn
            in_addr_t new_addr = inet_addr(ip);
            start_tcp(new_addr, port);

            // Login message
            uint8_t payload[12];
            // Message Type
            payload[0] = mt_LOGIN_MESSAGE;
            payload[1] = 0;
            payload[2] = 12;
            // Microphone ID
            payload[3] = MIC_ID;
            // Microphone Mono/Stereo
            payload[4] = MIC_CH_NUM;
            // Frame Size as power of two
            payload[5] = AUDIO_FRAME_SAMPLES_P2;
            esp_efuse_mac_get_default(&payload[6]);

            send_tcp(&payload, sizeof(payload));

            cs_hub_available = true;
        } break;

        case EVENT_CONN_AUDIO_REQ:{
            ESP_LOGI(TAG, "Audio request message received");
            cs_permission_to_send = true;

            int n_sec_req = *((int *) event_data);

            if ( n_sec_req < 0 ){frames_to_send = -1;}
            else {frames_to_send = n_sec_req * MIC_SAMPLE_RATE / AUDIO_FRAME_SAMPLES;}

        } break;

        case CONN_STOP_OF_STREAMING_MODE:{
            ESP_LOGI(TAG, "Stoping streaming mode");
            cs_permission_to_send = false;
            esp_event_post_to(events_audio_loop_h, EVENTS_AUDIO, EVENT_AUDIO_FINISHED, NULL, 0, portMAX_DELAY);

        } break;
        case EVENT_CONN_AUDIO_STOP:{
            ESP_LOGI(TAG, "Audio stop message received");
            cs_permission_to_send = false;
            esp_event_post_to(events_audio_loop_h, EVENTS_AUDIO, EVENT_AUDIO_FINISHED, NULL, 0, portMAX_DELAY);

        } break;

        case EVENT_CONN_DROPPED: {
            ESP_LOGI(TAG, "CONNECTION DROPPED");
            cs_hub_available = false;
            cs_permission_to_send = false;
            frames_to_send = -1;
        } break;

        case CONN_START_OF_STREAMING_MODE: {
            float STP = 0;
            esp_event_post_to(events_audio_loop_h, EVENTS_AUDIO, VAD_START, &STP, sizeof(STP), portMAX_DELAY);

            ESP_LOGI(TAG, "Streaming mode ON.");
        } break;

        default: {return;}
    }
}

/*EVENT_CONN_LOGIN = mt_LOGIN_MESSAGE,
    EVENT_CONN_SYNC = mt_SYNCHRONIZATION,
    EVENT_CONN_AUDIO_REQ = mt_AUDIO_REQUEST,
    EVENT_CONN_MASTER_AVAILABLE = mt_MASTER_AVAILABLE,
    EVENT_CONN_AUDIO_STOP = mt_AUDIO_STOP,
    EVENT_CONN_OUT_ON = mt_OUTPUT_ON,
    EVENT_CONN_OUT_OFF = mt_OUTPUT_OFF
    EVENT_CONN_ID = mt_ID,
    EVENT_CONN_REF_ANS = mt_REF_ANSWER*/