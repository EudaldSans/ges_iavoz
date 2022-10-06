#ifndef GES_IAVOZ_VAD
#define GES_IAVOZ_VAD

#include "esp_system.h"

esp_err_t ProcessNewFrame (uint16_t* data, size_t data_len);

#endif