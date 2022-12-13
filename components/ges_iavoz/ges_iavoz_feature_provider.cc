/* Copyright 2018 The TensorFlow Authors. All Rights Reserved.

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

#include "ges_iavoz_feature_provider.h"

#include <cstddef>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define NOISE_T     4
#define SPEECH_T    8
#define TRANS_T     10

static const char *TAG = "IAVOZ_FP";

TfLiteStatus InitializeMicroFeatures( IAVoz_FeatureProvider_t * fp );
TfLiteStatus GenerateMicroFeatures ( IAVoz_FeatureProvider_t * fp, const int16_t* input, int input_size, int output_size, int8_t* output, size_t* num_samples_read, int32_t* STP);

bool IAVoz_FeatureProvider_Init ( IAVoz_FeatureProvider_t ** fpptr, IAVoz_ModelSettings_t * ms ) {
    IAVoz_FeatureProvider_t * fp = (IAVoz_FeatureProvider_t *) malloc (sizeof(IAVoz_FeatureProvider_t));
    (*fpptr) = fp;
    if ( !fp ) {
        ESP_LOGE(TAG, "Error Allocating Feature Provider struct");
        return false;
    }

    if ( !ms ) {
        ESP_LOGE(TAG, "Error Reading Model Settings");
        return false;
    }

    fp->ms = ms;

    fp->feature_data = (int8_t *) malloc(fp->ms->kFeatureElementCount);
    if ( !fp->feature_data ) {
        ESP_LOGE(TAG, "Error Allocating Feature Provider buffer");
        return false;
    }

    fp->vad = fvad_new();
    if (!fp->vad) {
        ESP_LOGE(TAG, "Error allocating Fvad");
        return false;
    }

    fvad_set_sample_rate(fp->vad, fp->ms->kAudioSampleFrequency);
    fvad_set_mode(fp->vad, 2);

    fp->voices_in_frame = (bool*) malloc(sizeof(bool)*fp->ms->kFeatureSliceCount);
    if (!fp->voices_in_frame) {
        ESP_LOGE(TAG, "Error allocating space for voices in frame array");
        return false;
    }

    fp->voices_write_pointer = 0;

    memset(fp->feature_data, 0, fp->ms->kFeatureElementCount);
    memset(fp->voices_in_frame, 0, sizeof(bool)*fp->ms->kFeatureSliceCount);

    fp->number_of_frames = kAudioSampleFrequency/kMaxAudioSampleSize + 1;
    ESP_LOGI(TAG, "Allocating space for %d frames of %d samples", fp->number_of_frames, fp->ms->kMaxAudioSampleSize);
    for (int i = 0; i < fp->number_of_frames; i++) {
        fp->audio_samples[i] = (int16_t*) malloc(fp->ms->kMaxAudioSampleSize * sizeof(int16_t));
        memset(fp->audio_samples[i], 0, fp->ms->kMaxAudioSampleSize * sizeof(int16_t));
        if (!fp->audio_samples[i]) {
            ESP_LOGE(TAG, "Error allocating space for samples array");
            return false;
        }
    }
    
    fp->current_frame_start = 0;

    InitializeMicroFeatures( fp );

    return true;
}

bool IAVoz_FeatureProvider_DeInit ( IAVoz_FeatureProvider_t * fp ) {
    if (!fp ) {
        ESP_LOGE(TAG, "Failed to de-init Feature Provider");
        return false;
    }

    if ( !fp->feature_data ) {ESP_LOGW(TAG, "Null feature data");} 
    else {free(fp->feature_data);}

    if (!fp->vad) {ESP_LOGW(TAG, "Null Fvad");}
    else {fvad_free(fp->vad);}

    if (!fp->voices_in_frame) {ESP_LOGW(TAG, "Null voices in frame");}
    else {free(fp->voices_in_frame);}

    for (int i = 0; i < fp->number_of_frames; i++) {
        if (!fp->audio_samples[i]) {
            ESP_LOGW(TAG, "Null samples array");
            continue;
        }

        free(fp->audio_samples[i]);
    }

    free(fp);
    ESP_LOGI(TAG, "Feature Provider de-initialized");

    return true;
}

TfLiteStatus IAVoz_FeatureProvider_PopulateFeatureData (IAVoz_FeatureProvider_t * fp, IAVoz_AudioProvider_t * ap, 
        int32_t last_time_in_ms, int32_t time_in_ms, int* how_many_new_slices, float* STP) {

    static bool is_first_run_ = true;
    // Quantize the time into steps as long as each window stride, so we can
    // figure out which audio data we need to fetch.
    const int last_step = (last_time_in_ms / fp->ms->kFeatureSliceStrideMs);
    const int current_step = (time_in_ms / fp->ms->kFeatureSliceStrideMs);

    int slices_needed = current_step - last_step;
    // If this is the first call, make sure we don't use any cached information.

    if (is_first_run_) {
        is_first_run_ = false;
        slices_needed = fp->ms->kFeatureSliceCount;
    }

    if (slices_needed > fp->ms->kFeatureSliceCount) {slices_needed = fp->ms->kFeatureSliceCount;}

    *how_many_new_slices = slices_needed;

    const int slices_to_keep = fp->ms->kFeatureSliceCount - slices_needed;
    const int slices_to_drop = fp->ms->kFeatureSliceCount - slices_to_keep;

    // If we can avoid recalculating some slices, just move the existing data
    // up in the spectrogram, to perform something like this:
    // last time = 80ms          current time = 120ms
    // +-----------+             +-----------+
    // | data@20ms |         --> | data@60ms |
    // +-----------+       --    +-----------+
    // | data@40ms |     --  --> | data@80ms |
    // +-----------+   --  --    +-----------+
    // | data@60ms | --  --      |  <empty>  |
    // +-----------+   --        +-----------+
    // | data@80ms | --          |  <empty>  |
    // +-----------+             +-----------+

    if (slices_to_keep > 0) {
        for (int dest_slice = 0; dest_slice < slices_to_keep; ++dest_slice) {
            int8_t* dest_slice_data = fp->feature_data + (dest_slice * fp->ms->kFeatureSliceSize);
            const int src_slice = dest_slice + slices_to_drop;
            const int8_t* src_slice_data = fp->feature_data + (src_slice * fp->ms->kFeatureSliceSize);
            
            for (int i = 0; i < fp->ms->kFeatureSliceSize; ++i) {
                dest_slice_data[i] = src_slice_data[i];
            }
        }
    }

    // Any slices that need to be filled in with feature data have their
    // appropriate audio data pulled, and features calculated for that slice.
    if (slices_needed > 0) {
        for (int new_slice = slices_to_keep; new_slice < fp->ms->kFeatureSliceCount; ++new_slice) {
            const int new_step = (current_step - fp->ms->kFeatureSliceCount + 1) + new_slice;
            const int32_t slice_start_ms = (new_step * fp->ms->kFeatureSliceStrideMs);
            int16_t* audio_samples = nullptr;
            int audio_samples_size = 0;
            int vadres;

            // printf("Getting samples\n");

            // TODO(petewarden): Fix bug that leads to non-zero slice_start_ms
            GetAudioSamples(ap, (slice_start_ms > 0 ? slice_start_ms : 0),
                            fp->ms->kFeatureSliceDurationMs, &audio_samples_size,
                            &audio_samples);

            if (audio_samples_size < fp->ms->kMaxAudioSampleSize) {
                ESP_LOGE(TAG, "Audio data size %d too small, want %d", audio_samples_size, fp->ms->kMaxAudioSampleSize);
                return kTfLiteError;
            }

            // fvad only accepts frames of 30ms (480 samples @ 16kHz)
            vadres = fvad_process(fp->vad, audio_samples, fp->ms->kFeatureSliceDurationMs*fp->ms->kAudioSampleFrequency/1000);

            if (vadres < 0) {
                ESP_LOGE(TAG, "fvad process faied with error: %d", vadres);
                return kTfLiteApplicationError;
            }

            vadres = !!vadres; // Make sure it is 0 or 1

            fp->voices_in_frame[fp->voices_write_pointer] = vadres;
            fp->voices_write_pointer = (fp->voices_write_pointer + 1) % fp->ms->kFeatureSliceCount;

            int8_t* new_slice_data = fp->feature_data + (new_slice * fp->ms->kFeatureSliceSize);
            size_t num_samples_read;
            TfLiteStatus generate_status = GenerateMicroFeatures(
                fp, audio_samples, audio_samples_size, fp->ms->kFeatureSliceSize,
                new_slice_data, &num_samples_read, STP);

            uint16_t samples_to_keep = fp->ms->kAudioSampleFrequency - audio_samples_size;

            memcpy(fp->audio_samples[fp->current_frame_start], audio_samples, audio_samples_size * sizeof(int16_t));
            
            // for (int sample = 0; sample < audio_samples_size; sample++) {
            //     printf("[%d](%d/%d) ", sample, fp->audio_samples[fp->current_frame_start][sample], audio_samples[sample]);
            // }

            // printf("/n");
            
            fp->current_frame_start = (fp->current_frame_start + 1) % fp->number_of_frames;

            int8_t max_bank = 0;
            int16_t max_value = new_slice_data[0];
            int32_t low_band_power = 0;
            int32_t mid_band_power = 0;
            for (uint8_t sample = 1; sample < fp->ms->kFeatureSliceSize; sample++) {
                if (new_slice_data[sample] > max_value) {
                    max_bank = sample;
                    max_value = new_slice_data[sample];
                }

                if (6 < sample && sample < 13) {low_band_power += new_slice_data[sample];}
                if (12 < sample && sample < 19) {mid_band_power += new_slice_data[sample];}
            }
    
            // UpdateState(fp, STP, ZCR, max_bank, low_band_power, mid_band_power);
            
            if (generate_status != kTfLiteOk) {return generate_status;}         
        }
    }

    return kTfLiteOk;
}

TfLiteStatus InitializeMicroFeatures( IAVoz_FeatureProvider_t * fp ) 
{
    FrontendConfig config;
    config.window.size_ms = fp->ms->kFeatureSliceDurationMs;
    config.window.step_size_ms = fp->ms->kFeatureSliceStrideMs;
    config.noise_reduction.smoothing_bits = 10;
    config.filterbank.num_channels = fp->ms->kFeatureSliceSize;
    config.filterbank.lower_band_limit = 125.0;
    config.filterbank.upper_band_limit = 7500.0;
    config.noise_reduction.smoothing_bits = 10;
    config.noise_reduction.even_smoothing = 0.025;
    config.noise_reduction.odd_smoothing = 0.06;
    config.noise_reduction.min_signal_remaining = 0.05;
    config.pcan_gain_control.enable_pcan = 1;
    config.pcan_gain_control.strength = 0.95;
    config.pcan_gain_control.offset = 80.0;
    config.pcan_gain_control.gain_bits = 21;
    config.log_scale.enable_log = 1;
    config.log_scale.scale_shift = 6;

    if (!FrontendPopulateState(&config, &(fp->frontend_state), fp->ms->kAudioSampleFrequency)) 
    {
        ESP_LOGE(TAG, "FrontendPopulateState() failed");
        return kTfLiteError;
    }

    return kTfLiteOk;
}

TfLiteStatus GenerateMicroFeatures ( IAVoz_FeatureProvider_t * fp, const int16_t* input, int input_size, int output_size, int8_t* output, size_t* num_samples_read, float* STP) {
    const int16_t* frontend_input;
    static bool g_is_first_time = true;
    if (g_is_first_time) {
        frontend_input = input;
        g_is_first_time = false;
    } else {
        frontend_input = input + 160;
    }
    
    FrontendOutput frontend_output = FrontendProcessSamples(&(fp->frontend_state), frontend_input, input_size, num_samples_read);


    *STP = 0;
    for (size_t i = 0; i < frontend_output.size; ++i) {
    // These scaling values are derived from those used in input_data.py in the
    // training pipeline.
    // The feature pipeline outputs 16-bit signed integers in roughly a 0 to 670
    // range. In training, these are then arbitrarily divided by 25.6 to get
    // float values in the rough range of 0.0 to 26.0. This scaling is performed
    // for historical reasons, to match up with the output of other feature
    // generators.
    // The process is then further complicated when we quantize the model. This
    // means we have to scale the 0.0 to 26.0 real values to the -128 to 127
    // signed integer numbers.
    // All this means that to get matching values from our integer feature
    // output into the tensor input, we have to perform:
    // input = (((feature / 25.6) / 26.0) * 256) - 128
    // To simplify this and perform it in 32-bit integer math, we rearrange to:
    // input = (feature * 256) / (25.6 * 26.0) - 128
        constexpr int32_t value_scale = 256;
        constexpr int32_t value_div = static_cast<int32_t>((25.6f * 26.0f) + 0.5f);
        int32_t value = ((frontend_output.values[i] * value_scale) + (value_div / 2)) / value_div;
        value -= 128;
        
        if (value < -128) {value = -128;}
        if (value > 127) {value = 127;}
        
        output[i] = value;
        *STP += frontend_output.values[i];
    }

    *STP /= frontend_output.size;

    return kTfLiteOk;
}
