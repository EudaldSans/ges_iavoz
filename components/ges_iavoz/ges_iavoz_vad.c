#include "ges_iavoz_vad.h"
#include "es_dsp.h"
#include <math.h>



typedef enum {
    VAD_STATE_INIT = 0,
    VAD_STATE_TO_SPEECH,
    VAD_STATE_SPEECH,
    VAD_STATE_TO_NOISE,
    VAD_STATE_NOISE,
    VAD_STATES_LEN,
} VAD_state_t;

typedef struct {
    float NSTP;
    VAD_state_t previous_state;
    VAD_state_t current_state;
    uint8_t n_trans;
    uint8_t n_extend;
} VAD_features;

VAD_features* features;

/**
 * @brief Initializes the VAD component, by allocating zize for VAD features and initializing them.
 * 
 * @return @c ESP_ERR_NO_MEM if space for components is not found.
 * @c ESP_OK if initialization is successful 
 */
esp_err_t VAD_init(){
    features = maloc(sizeof(VAD_features));
    if (features == NULL) {
        return ESP_ERR_NO_MEM;
    }

    features->NSTP = 0;
    features->current_state = VAD_STATE_INIT;
    features->previous_state = VAD_STATE_INIT;

    return ESP_OK;
}

/**
 * @brief Frees space allocated when component is initialized
 */
void VAD_deinit() {
    free(features);
}

/**
 * @brief Performs analysis on an audio frame to determine if there is voice within.
 * 
 * @param data pointer to an @c int16_t array with the audio frame
 * @param data_len the length of the array
 * @param voice_present pointer to a boolean that will reflect whether voice was found within the frame
 * @return true if voice is detected in the frame
 * @return false if no voice is detected in the frame
 */
bool VAD_process_new_frame (int16_t* data, size_t data_len, bool* voice_present) {
    float STP, ZCR = 0;

    for (size_t sample = 1; sample < data_len; sample++) {

    }
}



