/********************************************************************************************
* Historico de Revisiones
*
* Versión	Fecha		    Autor	Comentario
------------------------------------------------------------------------------------------------
* V0.0 		12-07-2022      LMP	    Creacion
***********************************************************************************************/

#ifndef _GES_IAVOZ
#define _GES_IAVOZ

#ifdef __cplusplus
extern "C" {
#endif

/* INCLUDES */
/* -------- */
#include "sdkconfig.h"

#ifdef CONFIG_IAVOZ_ENABLE

#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>

/* DEFINES */
/* ------- */

/* TYPES */
/* ----- */
typedef enum {
    IAVOZ_KEY_NULL = 0,
    IAVOZ_KEY_HEYLOLA,
    IAVOZ_KEY_ENCIENDE,
    IAVOZ_KEY_APAGA,
    IAVOZ_KEY_SUBE,
    IAVOZ_KEY_BAJA,
    IAVOZ_KEY_PARA,
    IAVOZ_KEY_SOCORRO,
    IAVOZ_KEY_ACTIVA,
    IAVOZ_KEY_TODO,
    IAVOZ_NUM_KEYS
} IAVOZ_KEY_t;

typedef void (*pIAVOZCallback_t)(IAVOZ_KEY_t xKeyWord, uint64_t uiPower);

/* EXTERNAL FUNCTIONS */
/* ------------------ */

/* PUBLIC FUNCTIONS */
/* ---------------- */

/**
 * @brief Configure a IAVOZ service.
 *
 * @param uiCore          The CPU core where the underlying task must be executed.
 *
 * @param pCallback       A callback function, which is called when the right keyword is detected.
 *
 * @return
 *     - true if all is ok
 *     - false if an error occurred. In this case an error log message is written to the console.
 */
bool IAVOZ_Init(int iCore, pIAVOZCallback_t pCallback);

/**
 * @brief Deinit IAVOZ service.
 *
 * @return
 *     - true if all is ok
 *     - false if an error occurred. In this case an error log message is written to the console.
 */
bool IAVOZ_Deinit(void);



#endif // CONFIG_IAVOZ_ENABLE

#ifdef __cplusplus
}
#endif

#endif