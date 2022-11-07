#include "ges_iavoz_main.h"

#include <sys/_stdint.h>
#include "ges_iavoz_audio_provider.h"

#include "ges_iavoz_command_responder.h"
#include "model.h"

#define MAX_STP_SAMPLES 3

const char * TAG = "IAVOZ_SYS";

// constexpr int kTensorArenaSize = g_model_len;

void IAVoz_System_Task ( void * vParam );


bool IAVoz_System_Init ( IAVoz_System_t ** sysptr, IAVoz_ModelSettings_t * ms, pIAVOZCallback_t cb ) {
    IAVoz_System_t *sys = (IAVoz_System_t * ) malloc(sizeof(IAVoz_System_t));
    (*sysptr) = sys;
    if ( !sys ) {
        ESP_LOGE(TAG, "Error Allocating IAVoz System");
        return false;
    }

    sys->ms = ms;

    sys->tensor_arena = (uint8_t *) malloc(g_model_len * sizeof(uint8_t));

    // TF API
    sys->model = tflite::GetModel(g_model);
    if (sys->model->version() != TFLITE_SCHEMA_VERSION){
        ESP_LOGE(TAG, "Model provided is schema version %d not equal to supported version %d.", sys->model->version(), TFLITE_SCHEMA_VERSION);
        return false;
    }

    ESP_LOGI(TAG, "Creating error reporter");
    sys->error_reporter = new tflite::MicroErrorReporter();
    if (!sys->error_reporter) {ESP_LOGE(TAG, "Could not create error reporter");}

    ESP_LOGI(TAG, "Adding operations to op resolver");
    sys->micro_op_resolver = new tflite::MicroMutableOpResolver<6>(sys->error_reporter);
    if (sys->micro_op_resolver->AddDepthwiseConv2D() != kTfLiteOk) {
        ESP_LOGE(TAG, "Could not add deppth wise conv 2D layer");
        return false;
    }
    if (sys->micro_op_resolver->AddFullyConnected() != kTfLiteOk) {
        ESP_LOGE(TAG, "Could not add fully connected layer");
        return false;
    }
    if (sys->micro_op_resolver->AddSoftmax() != kTfLiteOk) {
        ESP_LOGE(TAG, "Could not add softmax layer");
        return false;
    }
    if (sys->micro_op_resolver->AddReshape() != kTfLiteOk) {
        ESP_LOGE(TAG, "Could not add reshape layer");
        return false;
    }
    if (sys->micro_op_resolver->AddMaxPool2D() != kTfLiteOk) {
        ESP_LOGE(TAG, "Could not add max pool 2D layer");
        return false;
    }
    if (sys->micro_op_resolver->AddConv2D() != kTfLiteOk) {
        ESP_LOGE(TAG, "Could not add conv 2D layer");
        return false;
    }

    ESP_LOGI(TAG, "Creating micro interpreter");
    sys->interpreter = new tflite::MicroInterpreter(sys->model, *(sys->micro_op_resolver), sys->tensor_arena, g_model_len, sys->error_reporter);

    ESP_LOGI(TAG, "Allocating tensors");
    TfLiteStatus allocate_status = sys->interpreter->AllocateTensors();
    if (allocate_status != kTfLiteOk) {
        ESP_LOGE(TAG, "AllocateTensors() failed");
        return false;
    }

    // Get information about the memory area to use for the model's input.
    sys->model_input = sys->interpreter->input(0);
    if ((sys->model_input->dims->size != 2) || (sys->model_input->dims->data[0] != 1) || (sys->model_input->dims->data[1] != (sys->ms->kFeatureSliceCount * sys->ms->kFeatureSliceSize)) || (sys->model_input->type != kTfLiteInt8)) 
    {
        ESP_LOGE(TAG, "Bad input tensor parameters in model");
        return false;
    }
    
    sys->model_input_buffer = sys->model_input->data.int8;

    // GES API
    ESP_LOGI(TAG, "Initializing AudioProvider");
    if ( !IAVoz_AudioProvider_Init(&sys->ap, sys->ms) )
    {
        ESP_LOGE(TAG, "AudioProvider Init Failed");
        return false;
    }

    ESP_LOGI(TAG, "Initializing FeatureProvider");
    if ( !IAVoz_FeatureProvider_Init(&sys->fp, sys->ms) )
    {
        ESP_LOGE(TAG, "FeatureProvider Init Failed");
        return false;
    }

    if ( !cb )
    {
        ESP_LOGE(TAG, "Null callback provided");
        return false;
    }

    sys->cb = cb;

    ESP_LOGI(TAG, "Initializing RecognizeCommands");
    sys->recognizer = new RecognizeCommands(sys->error_reporter);

    sys->previous_time = 0;

    sys->is_sys_started = false;

    initCommandResponder();

    return true;
}

void IAVoz_System_Start ( IAVoz_System_t * sys ) {
    //ESP_LOGI(TAG, "%p ptr", sys);
    if ( sys->is_sys_started ) {
        ESP_LOGW(TAG, "System Task already started");
        return;
    }

    xTaskCreate(IAVoz_System_Task, "System_Task", CONFIG_IAVOZ_SYS_TASK_STACK_SIZE, (void *) sys, CONFIG_IAVOZ_SYS_TASK_PRIORITY, &(sys->th));
    sys->is_sys_started = true;

    IAVoz_AudioProvider_Start(sys->ap);

    ESP_LOGI(TAG, "System Task started");
}

void IAVoz_System_Stop ( IAVoz_System_t * sys ) {
    if ( !sys->is_sys_started ) {
        ESP_LOGW(TAG, "System Task already stopped");
        return;
    }

    if ( !sys->th ) {
        ESP_LOGE(TAG, "System Task has NULL handler");
        return;
    }

    vTaskDelete(sys->th);
    sys->th = NULL;
    sys->is_sys_started = false;

    IAVoz_AudioProvider_Stop(sys->ap);

    ESP_LOGI(TAG, "System Task stopped");
}

bool IAVoz_System_DeInit ( IAVoz_System_t * sys ) {
    delete sys->error_reporter;
    delete sys->micro_op_resolver;
    delete sys->interpreter;

    if(!sys->tensor_arena) {free(sys->tensor_arena);}  

    // safely delete model settings
    bool ok = IAVoz_FeatureProvider_DeInit(sys->fp);
    ok = ok && IAVoz_AudioProvider_DeInit(sys->ap);

    ESP_LOGI(TAG, "IAVoz System DeInit");

    return ok;
}

void IAVoz_System_Task ( void * vParam ) {
    IAVoz_System_t * sys = (IAVoz_System_t *) vParam;
    IAVoz_ModelSettings_t * ms = sys->ms;

    int32_t previous_time = 0;
    int32_t STP_buffer[MAX_STP_SAMPLES];
    uint8_t STP_position = 0;

    for (;;) {
        const int32_t current_time = LatestAudioTimestamp(sys->ap);
        int how_many_new_slices = 0;

        TfLiteStatus feature_status = IAVoz_FeatureProvider_PopulateFeatureData(sys->fp, sys->ap, previous_time, current_time, &how_many_new_slices);
        STP_position = (STP_position + 1) % MAX_STP_SAMPLES;
        if (feature_status != kTfLiteOk) {continue;}
        previous_time = current_time;

        if (how_many_new_slices == 0 ) {continue;}
        
        int32_t STP = 0;
        for (int i = 0; i < MAX_STP_SAMPLES; i++) {
            STP += STP_buffer[i];
        }
        STP /= MAX_STP_SAMPLES;
        
        uint8_t voice_in_frame = 0;
        uint8_t voice_in_bof = 0;
        uint8_t voice_in_eof = 0;
        for (uint16_t sample = 0; sample < sys->fp->ms->kFeatureSliceCount; sample++) {
            uint16_t position = (sample + sys->fp->voices_write_pointer) % sys->fp->ms->kFeatureSliceCount;
            if (sample < sys->fp->ms->kFeatureSliceCount/4) {voice_in_bof += sys->fp->voices_in_frame[position];}
            if (sample > 3*sys->fp->ms->kFeatureSliceCount/4) {voice_in_eof += sys->fp->voices_in_frame[position];}

            voice_in_frame += sys->fp->voices_in_frame[position];
        }

        ESP_LOGD(TAG, "vif: %3d\t vib: %3d\t vie: %3d\t STP: %d", voice_in_frame, voice_in_bof, voice_in_eof, STP);
        
        if (voice_in_frame < sys->fp->ms->kFeatureSliceCount/3){continue;}
        if (voice_in_eof > voice_in_frame/2) {continue;}
        if (voice_in_bof > voice_in_frame/2) {continue;}
        // if (STP < 50) {continue;}

        for (int i = 0; i < ms->kFeatureElementCount; i++) {
            sys->model_input_buffer[i] = sys->fp->feature_data[i];
        }

        TfLiteStatus invoke_status = sys->interpreter->Invoke();
        if (invoke_status != kTfLiteOk ) { ESP_LOGE(TAG, "Interpeter failed");}
        
        TfLiteTensor * output = sys->interpreter->output(0);
        IAVOZ_KEY_t found_command;
        uint8_t found_index;
        uint8_t score = 0;
        bool is_new_command = false;

        TfLiteStatus process_status = sys->recognizer->ProcessLatestResults(
            output, current_time, &found_command, &score, &is_new_command, &found_index);
        if (process_status != kTfLiteOk) {
            ESP_LOGE(TAG, "RecognizeCommands::ProcessLatestResults() failed");
            return;
        }

        if (is_new_command) {
            sys->cb(found_command, STP);
            RespondToCommand(found_command);
        }

    }

    vTaskDelete(NULL);
}