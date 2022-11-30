#pragma once

#include "driver/gpio.h"
#include "driver/i2s.h"

#define MIC_ID MIC_ID_PCM2

#define MIC_I2S_NUM 0
#define MIC_SAMPLE_RATE 16000

#define MIC_CH_LEFT     1
#define MIC_CH_RIGHT    1
#define MIC_CH_NUM      (MIC_CH_LEFT + MIC_CH_RIGHT)

#define MIC_MODULATION_IS_PCM  1
#define MIC_MODULATION_IS_PDM  (!MIC_MODULATION_IS_PCM)

#define MIC_MODEL_IS_SPH       0

#define MIC_PIN_BCK 13
#define MIC_PIN_WS  0
#define MIC_PIN_SD  22

// Audio configurations
#define AUDIO_FRAME_SAMPLES_P2  9
#define AUDIO_FRAME_SAMPLES     (1 << AUDIO_FRAME_SAMPLES_P2)

// Microphone configurations
enum {
    MIC_ID_PCM0 = 1,    // MSM261S4030H0
    MIC_ID_PCM1,        // SPH064LM4H-B
    MIC_ID_PDM0,        // IMP34DT05
    MIC_ID_PDM1,        // IM69D120
    MIC_ID_CASELESS,    // INMP 441
    MIC_ID_PCM2         // CMM-4030D-261-I2S
};

// Message Types
enum msgType {
            mt_AUDIO_DETECTED               = 0,  //s->m
            mt_AUDIO_FINISHED               = 1,  //s->m
            mt_AUDIO_DATA                   = 2,  //s->m
            mt_SYNCHRONIZATION              = 3,  //m->s
            mt_AUDIO_REQUEST                = 4,  //m->s
            mt_MASTER_AVAILABLE             = 5,  //m->s
            mt_MASTER_UNAVAILABLE           = 6,  //m->s
            mt_AUDIO_STOP                   = 7,  //m->s
            mt_OUTPUT_ON                    = 8,  //m->s
            mt_OUTPUT_OFF                   = 9,  //m->s
            mt_LOCATION_INFO                = 10, //s->m
            mt_LOGIN_MESSAGE                = 11, //s->m
            mt_ID                           = 12, //m->s
            mt_ID_ACK                       = 13, //s->m
            mt_REF_PETITION                 = 14, //s->m
            mt_REF_ANSWER                   = 15, //m->m
            mt_ASSISTANT_STATE              = 16, //m->s
            mt_START_OF_STREAMING_MODE      = 17, //m->s
            mt_STOP_OF_STREAMING_MODE       = 18 //m->s
};
