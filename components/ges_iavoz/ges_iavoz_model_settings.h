#ifndef GES_IAVOZ_MODEL_SETTINGS
#define GES_IAVOZ_MODEL_SETTINGS

#include "model_settings.h"

typedef struct {
    int kMaxAudioSampleSize;
    int kAudioSampleFrequency;
    int kFeatureSliceSize;
    int kFeatureSliceCount;
    int kFeatureElementCount;
    int kFeatureSliceStrideMs;
    int kFeatureSliceDurationMs;
    int kSilenceIndex;
    int kUnknownIndex;
    int kCategoryCount;
    const char ** kCategoryLabels;
    const int * kCategoryEnum;
} IAVoz_ModelSettings_t;

#endif