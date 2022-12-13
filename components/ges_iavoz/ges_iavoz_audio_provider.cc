
#include "ges_iavoz_audio_provider.h"
#include "sdkconfig.h"

#include <cstddef>
#include <cstdlib>
#include <cstring>

#include "ges_events.h"

static const char * TAG = "IAVOZ_AP";

void IAVoz_AudioProvider_I2STask ( void * vParam );

bool IAVoz_I2SInit ( void ) {
    // Init I2S

    // Start listening for audio: MONO @ 16KHz
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_TX),
        .sample_rate = 16000,
        .bits_per_sample = (i2s_bits_per_sample_t)16,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = 0,
        .dma_buf_count = 3,
        .dma_buf_len = 300,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = -1,
    };

    i2s_pin_config_t pin_config = {
        .mck_io_num = I2S_PIN_NO_CHANGE,
        .bck_io_num = CONFIG_IAVOZ_MIC_I2S_PIN_BCK,    // IIS_SCLK
        .ws_io_num = CONFIG_IAVOZ_MIC_I2S_PIN_WS,     // IIS_LCLK
        .data_out_num = I2S_PIN_NO_CHANGE,  // IIS_DSIN
        .data_in_num = CONFIG_IAVOZ_MIC_I2S_PIN_DIN,   // IIS_DOUT
    };

    bool success = true;
    esp_err_t ret = 0;

    ret = i2s_driver_install((i2s_port_t) CONFIG_IAVOZ_MIC_I2S_NUM, &i2s_config, 0, NULL);
    if (ret != ESP_OK) {
        success = false;
        ESP_LOGE(TAG, "Error in i2s_driver_install");
    }

    ret = i2s_set_pin((i2s_port_t) CONFIG_IAVOZ_MIC_I2S_NUM, &pin_config);
    if (ret != ESP_OK) {
        success = false;
        ESP_LOGE(TAG, "Error in i2s_set_pin");
    }

    ret = i2s_zero_dma_buffer((i2s_port_t) CONFIG_IAVOZ_MIC_I2S_NUM);
    if (ret != ESP_OK) {
        success = false;
        ESP_LOGE(TAG, "Error in initializing dma buffer with 0");
    }

    return success;
}

bool IAVoz_AudioProvider_Init ( IAVoz_AudioProvider_t ** apptr, IAVoz_ModelSettings_t * ms ) {
    IAVoz_AudioProvider_t * ap = (IAVoz_AudioProvider_t *) malloc(sizeof(IAVoz_AudioProvider_t));
    (*apptr) = ap;
    if (!ap) {
        ESP_LOGE(TAG, "Error allocating Audio Provider struct");
        return false;
    }

    if (!ms) {
        ESP_LOGE(TAG, "Error with provided Model Settings");
        return false;
    }
    ap->ms = ms;

    ESP_LOGI(TAG, "Initializing Ring Buffer");
    ap->audio_capture_buffer_size = 80000;
    ap->audio_capture_buffer = rb_init("tf_ringbuffer", ap->audio_capture_buffer_size);
    if (!ap->audio_capture_buffer) {
        ESP_LOGE(TAG, "Error creating ring buffer");
        return false;
    }

    ap->latest_audio_timestamp = 0;
    ap->history_samples_to_keep = ((ap->ms->kFeatureSliceDurationMs - ap->ms->kFeatureSliceStrideMs) * (ap->ms->kAudioSampleFrequency / 1000));
    ap->new_samples_to_get = (ap->ms->kFeatureSliceStrideMs * (ap->ms->kAudioSampleFrequency / 1000));

    ap->audio_output_buffer = (int16_t *) malloc(sizeof(int16_t) * ap->ms->kMaxAudioSampleSize);
    if (!ap->audio_output_buffer) {
        ESP_LOGE(TAG, "Error creating Audio Output Buffer");
        return false;
    }

    ap->is_audio_started = false;

    ap->history_buffer = (int16_t *) malloc(sizeof(int16_t) * ap->history_samples_to_keep);
    if (!ap->history_buffer) {
        ESP_LOGE(TAG, "Error creating History Buffer");
        return false;
    }

    bool success = IAVoz_I2SInit();
    if ( !success ) {return false;}

    ap->audio_task_handle = NULL;

    return true;
}

void IAVoz_AudioProvider_Start ( IAVoz_AudioProvider_t * ap ) {
    if ( ap->is_audio_started ) {
        ESP_LOGW(TAG, "AudioProvider Task already started");
        return;
    }

    ap->is_audio_started = true;
    xTaskCreate(IAVoz_AudioProvider_I2STask, "AudioProvider_I2STask", CONFIG_IAVOZ_MIC_TASK_STACK_SIZE, (void *) ap, CONFIG_IAVOZ_MIC_TASK_PRIORITY, &(ap->audio_task_handle));

    ESP_LOGI(TAG, "AudioProvider Task started");
}

void IAVoz_AudioProvider_Stop ( IAVoz_AudioProvider_t * ap ) {
    if ( !ap->is_audio_started ) {
        ESP_LOGW(TAG, "AudioProvider Task already stopped");
        return;
    }

    if ( !ap->audio_task_handle ) {
        ESP_LOGE(TAG, "AudioProvider Task has NULL handler");
        return;
    }

    vTaskDelete(ap->audio_task_handle);
    ap->audio_task_handle = NULL;
    ap->is_audio_started = false;

    ESP_LOGI(TAG, "AudioProvider Task stopped");
}

bool IAVoz_AudioProvider_DeInit ( IAVoz_AudioProvider_t * ap ) {
    if ( !ap ) {
        ESP_LOGE(TAG, "Failed to de-init Audio Provider");
        return false;
    }

    if (ap->is_audio_started)       {IAVoz_AudioProvider_Stop(ap);}
    if (ap->audio_capture_buffer)   {free(ap->audio_capture_buffer);}
    if (ap->audio_output_buffer)    {free(ap->audio_output_buffer);}
    if (ap->history_buffer)         {free(ap->history_buffer);}

    free(ap);

    ESP_LOGI(TAG, "Audio Provider de-initialized");
    return true;
}

void IAVoz_AudioProvider_I2STask ( void * vParam ) {
    IAVoz_AudioProvider_t * ap = (IAVoz_AudioProvider_t *) vParam;

    size_t bytes_read = i2s_bytes_to_read;
    uint16_t i2s_read_buffer[i2s_bytes_to_read / 2] = {};

    for ( ;; ) {
        i2s_read((i2s_port_t) GES_IAVOZ_I2S_NUM, (void*)i2s_read_buffer, i2s_bytes_to_read, &bytes_read, 10);

        if (bytes_read <= 0) {
            ESP_LOGE(TAG, "Error in I2S read : %d", bytes_read);
        } else {
            if (bytes_read < i2s_bytes_to_read) {ESP_LOGE(TAG, "Partial I2S read");}

            for (int sample = 2; sample < bytes_read / sizeof(uint16_t); sample += 2) {
                i2s_read_buffer[sample/2] = i2s_read_buffer[sample];
            }

            /* write bytes read by i2s into ring buffer */
            int bytes_written = rb_write(ap->audio_capture_buffer, (uint8_t*)i2s_read_buffer, bytes_read / 2, 10);

            /* update the timestamp (in ms) to let the model know that new data has
            * arrived */
            ap->latest_audio_timestamp += ((1000 * (bytes_written / 2)) / ap->ms->kAudioSampleFrequency);
            ESP_LOGD(TAG, "%d-%d-%d-%d", i2s_read_buffer[0], i2s_read_buffer[1], i2s_read_buffer[2], i2s_read_buffer[3]);

            if (bytes_written <= 0) {ESP_LOGE(TAG, "Could Not Write in Ring Buffer: %d ", bytes_written);} 
            else if (bytes_written < bytes_read / 2) {ESP_LOGW(TAG, "Partial Write");}
        }
    }
}


TfLiteStatus GetAudioSamples(IAVoz_AudioProvider_t * ap, int start_ms, int duration_ms, int *audio_samples_size, int16_t **audio_samples)
{
    if (!ap->is_audio_started) 
    {
        IAVoz_AudioProvider_Start(ap);
        ap->is_audio_started = true;
    }

    /* copy 160 samples (320 bytes) into output_buff from history */
    memcpy((void*)(ap->audio_output_buffer), (void*)(ap->history_buffer),
    ap->history_samples_to_keep * sizeof(int16_t));

    /* copy 320 samples (640 bytes) from rb at ( int16_t*(g_audio_output_buffer) +
    * 160 ), first 160 samples (320 bytes) will be from history */
    int32_t bytes_read = rb_read(ap->audio_capture_buffer,
        ((uint8_t*)(ap->audio_output_buffer + ap->history_samples_to_keep)),
        ap->new_samples_to_get * sizeof(int16_t), 10);

    if (bytes_read < 0) 
    {
        ESP_LOGE(TAG, " Model Could not read data from Ring Buffer");
    } 
    else if (bytes_read < ap->new_samples_to_get * sizeof(int16_t)) 
    {
        ESP_LOGD(TAG, "RB FILLED RIGHT NOW IS %d",
        rb_filled(ap->audio_capture_buffer));
        ESP_LOGD(TAG, " Partial Read of Data by Model ");
        ESP_LOGV(TAG, " Could only read %d bytes when required %d bytes ", bytes_read, ap->new_samples_to_get * sizeof(int16_t));
    }

    /* copy 320 bytes from output_buff into history */
    memcpy((void*)(ap->history_buffer),
        (void*)(ap->audio_output_buffer + ap->new_samples_to_get),
        ap->history_samples_to_keep * sizeof(int16_t));

    *audio_samples_size = ap->ms->kMaxAudioSampleSize;
    *audio_samples = ap->audio_output_buffer;
    return kTfLiteOk;
}

int32_t LatestAudioTimestamp ( IAVoz_AudioProvider_t * ap ) 
{ 
    return ap->latest_audio_timestamp; 
}