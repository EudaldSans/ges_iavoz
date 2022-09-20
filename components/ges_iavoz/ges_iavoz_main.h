#ifndef GES_IAVOZ_MAIN
#define GES_IAVOZ_MAIN

#include "esp_log.h"
#include "ges_iavoz.h"
#include "ges_iavoz_audio_provider.h"
#include "ges_iavoz_feature_provider.h"
#include "ges_iavoz_command_recognizer.h"
#include "ges_iavoz_model_settings.h"

#include "model.h"

#include "tensorflow/lite/c/c_api_types.h"
#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/core/api/error_reporter.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"

typedef struct {
    tflite::ErrorReporter * error_reporter;
    const tflite::Model * model;
    tflite::MicroMutableOpResolver<6> * micro_op_resolver;
    tflite::MicroInterpreter * interpreter;
    RecognizeCommands * recognizer;

    IAVoz_AudioProvider_t * ap;
    IAVoz_FeatureProvider_t * fp;
    IAVoz_ModelSettings_t * ms;

    uint8_t * tensor_arena;
    int8_t * feature_buffer;
    int8_t * model_input_buffer;
    TfLiteTensor * model_input;

    int32_t previous_time;
    pIAVOZCallback_t cb;
    TaskHandle_t th;
    bool is_sys_started;
} IAVoz_System_t;


bool IAVoz_System_Init ( IAVoz_System_t ** sysptr, IAVoz_ModelSettings_t * ms, pIAVOZCallback_t cb );
bool IAVoz_System_DeInit ( IAVoz_System_t * sys );
void IAVoz_System_Start ( IAVoz_System_t * sys );
void IAVoz_System_Stop ( IAVoz_System_t * sys );

#endif