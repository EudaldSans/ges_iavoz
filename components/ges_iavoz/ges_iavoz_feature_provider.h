#ifndef GES_IAVOZ_FEATURE_PROVIDER
#define GES_IAVOZ_FEATURE_PROVIDER

#include "tensorflow/lite/c/common.h"

#include "esp_log.h"

#include "ges_iavoz_audio_provider.h"
#include "ges_iavoz_model_settings.h"

#include "tensorflow/lite/c/c_api_types.h"
#include "tensorflow/lite/experimental/microfrontend/lib/frontend.h"
#include "tensorflow/lite/experimental/microfrontend/lib/frontend_util.h"

typedef enum {
    STATE_INIT = 0,
    STATE_NOISE,
    STATE_TO_SPEECH,
    STATE_SPEECH,
    STATE_TO_NOISE
} IAVoz_FeatureProvider_State_t;

typedef struct {
    int8_t * feature_data;
    IAVoz_ModelSettings_t * ms;
    struct FrontendState frontend_state;
    float NSTP;
    bool voice_detected;
    IAVoz_FeatureProvider_State_t current_state;
    uint8_t n_trans;
} IAVoz_FeatureProvider_t;

// GES API
bool IAVoz_FeatureProvider_Init ( IAVoz_FeatureProvider_t ** fpptr, IAVoz_ModelSettings_t * ms );
bool IAVoz_FeatureProvider_DeInit ( IAVoz_FeatureProvider_t * fp );


// TF API
TfLiteStatus IAVoz_FeatureProvider_PopulateFeatureData ( IAVoz_FeatureProvider_t * fp, IAVoz_AudioProvider_t * ap, int32_t last_time_in_ms, int32_t time_in_ms, int* how_many_new_slices, int32_t* STP);
TfLiteStatus GenerateMicroFeatures ( IAVoz_FeatureProvider_t * fp, const int16_t* input, int input_size, int output_size, int8_t* output, size_t* num_samples_read, int32_t* STP);



#endif
