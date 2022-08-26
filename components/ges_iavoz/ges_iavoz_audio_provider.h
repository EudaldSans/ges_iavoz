#ifndef GES_IAVOZ_MICROPHONE
#define GES_IAVOZ_MICROPHONE

#include "sdkconfig.h"

#include "driver/i2s.h"
#include "esp_log.h"

#include "freertos/task.h"

#include "ringbuf.h"
#include "ges_iavoz_model_settings.h"

#include "tensorflow/lite/c/common.h"

#define GES_IAVOZ_I2S_NUM   1
#define GES_IAVOZ_PIN_BCK   26
#define GES_IAVOZ_PIN_WS    32
#define GES_IAVOZ_PIN_DIN   33

typedef struct {
    ringbuf_t * audio_capture_buffer;
    uint32_t audio_capture_buffer_size;
    volatile int32_t latest_audio_timestamp;
    int32_t history_samples_to_keep;
    int32_t new_samples_to_get;

    int16_t * audio_output_buffer;
    bool is_audio_started;
    int16_t * history_buffer;

    TaskHandle_t audio_task_handle;

    IAVoz_ModelSettings_t * ms;
} IAVoz_AudioProvider_t;

const int32_t kAudioCaptureBufferSize = 80000;
const int32_t i2s_bytes_to_read = 3200;




// GES API
bool IAVoz_AudioProvider_Init ( IAVoz_AudioProvider_t ** apptr, IAVoz_ModelSettings_t * ms );
bool IAVoz_AudioProvider_DeInit ( IAVoz_AudioProvider_t * ap );
void IAVoz_AudioProvider_Start ( IAVoz_AudioProvider_t * ap );
void IAVoz_AudioProvider_Stop ( IAVoz_AudioProvider_t * ap );


// TF API
TfLiteStatus GetAudioSamples( IAVoz_AudioProvider_t * ap , int start_ms, int duration_ms, int *audio_samples_size, int16_t **audio_samples );

int32_t LatestAudioTimestamp( IAVoz_AudioProvider_t * ap );

#endif // IAVOZ_ENABLE