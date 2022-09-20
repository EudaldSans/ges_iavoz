/********************************************************************************************
* Historico de Revisiones
*
* Versión	Fecha		    Autor	Comentario
------------------------------------------------------------------------------------------------
* V0.0 		xx-xx-2022      XXX	    Creacion
***********************************************************************************************/

/* INCLUDES */
/* -------- */
#include "sdkconfig.h"

#if CONFIG_IAVOZ_ENABLE

#include "ges_iavoz.h"
#include "esp_log.h"

#include "model_settings.h"
#include "ges_iavoz_main.h"

/* TYPES */
/* ----- */

/* DEFINES */
/* ------- */
#define GES_IAVOZ_TAG           "IAVOZ"

/* INTERNAL VARIABLES */
/* ------------------ */
static const char * TAG = "IAVOZMAIN";

const char* kCategoryLabels[kCategoryCount] = {
    "silence",
    "unknown",
    "ayuda",
    "apaga",
    "enciende",
    "heylola",
};

const int kCategoryEnum[kCategoryCount] = {
    (int) IAVOZ_KEY_NULL,
    (int) IAVOZ_KEY_NULL,
    (int) IAVOZ_KEY_SOCORRO,
    (int) IAVOZ_KEY_ENCIENDE,
    (int) IAVOZ_KEY_APAGA,
    (int) IAVOZ_KEY_NULL,
};

static IAVoz_ModelSettings_t IAVoz_ModelSettings = {
    .kMaxAudioSampleSize = kMaxAudioSampleSize,
    .kAudioSampleFrequency = kAudioSampleFrequency,
    .kFeatureSliceSize = kFeatureSliceSize,
    .kFeatureSliceCount = kFeatureSliceCount,
    .kFeatureElementCount = kFeatureElementCount,
    .kFeatureSliceStrideMs = kFeatureSliceStrideMs,
    .kFeatureSliceDurationMs = kFeatureSliceDurationMs,
    .kSilenceIndex = kSilenceIndex,
    .kUnknownIndex = kUnknownIndex,
    .kCategoryCount = kCategoryCount,
    .kCategoryLabels = kCategoryLabels,
    .kCategoryEnum = kCategoryEnum
};

static IAVoz_System_t * IAVoz_System;

/* EXTERNAL VARIABLES */
/* ------------------ */


/* INTERNAL FUNCTIONS */
/* ------------------ */


/* EXTERNAL FUNCTIONS */
/* ------------------ */

/* PUBLIC FUNCTIONS */
/* ---------------- */
bool IAVOZ_Init ( int iCore, pIAVOZCallback_t pCallback )
{
    bool ok = IAVoz_System_Init ( &IAVoz_System , &IAVoz_ModelSettings, pCallback );
    ESP_LOGI(TAG, "System initalized");
    if (ok)
    {
        ESP_LOGI(TAG, "%p %p ptr", IAVoz_System, &IAVoz_System);
        ESP_LOGI(TAG, "Starting System");
        IAVoz_System_Start(IAVoz_System);
    }
    return ok;
}

bool IAVOZ_Deinit ( void )
{
    IAVoz_System_Stop(IAVoz_System);
    bool ok = IAVoz_System_DeInit ( IAVoz_System );
    return ok;
}


/* CODE */
/* ---- */


#endif // CONFIG_IAVOZ_ENABLE

