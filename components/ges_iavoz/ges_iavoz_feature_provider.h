#ifndef GES_IAVOZ_FEATURE_PROVIDER
#define GES_IAVOZ_FEATURE_PROVIDER

#include "tensorflow/lite/c/common.h"

#include "esp_log.h"

#include "ges_iavoz_audio_provider.h"
#include "ges_iavoz_model_settings.h"

#include "tensorflow/lite/c/c_api_types.h"
#include "tensorflow/lite/experimental/microfrontend/lib/frontend.h"
#include "tensorflow/lite/experimental/microfrontend/lib/frontend_util.h"

typedef struct {
    int8_t * feature_data;
    IAVoz_ModelSettings_t * ms;
    struct FrontendState frontend_state;
} IAVoz_FeatureProvider_t;

// GES API
bool IAVoz_FeatureProvider_Init ( IAVoz_FeatureProvider_t ** fpptr, IAVoz_ModelSettings_t * ms );
bool IAVoz_FeatureProvider_DeInit ( IAVoz_FeatureProvider_t * fp );


// TF API
TfLiteStatus IAVoz_FeatureProvider_PopulateFeatureData ( IAVoz_FeatureProvider_t * fp, IAVoz_AudioProvider_t * ap, int32_t last_time_in_ms, int32_t time_in_ms, int* how_many_new_slices);
TfLiteStatus GenerateMicroFeatures ( IAVoz_FeatureProvider_t * fp, const int16_t* input, int input_size, int output_size, int8_t* output, size_t* num_samples_read );



#endif
