#pragma once

#ifndef __flash_eebl0_h_
#define __flash_eebl0_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

enum
{
    EEBL0_ERROR_CBLOG_SUCCESS             = 0  ,
    EEBL0_ERROR_CBLOG_STDLIB_MALLOC_FAIL  = 1  ,
    EEBL0_ERROR_CBLOG_SYSSTAT_STAT_FAIL   = 2  ,
    EEBL0_ERROR_CBLOG_FOPEN_FAIL          = 3  ,
    EEBL0_ERROR_CBLOG_EF_NULL_INPUT       = 4  ,
    EEBL0_ERROR_CBLOG_ESP_NULL_INPUT      = 5  ,
    EEBL0_ERROR_CBLOG_EGP_NULL_INPUT      = 6  ,
    EEBL0_ERROR_CBLOG_ECTD_NULL_INPUT     = 7  ,
    EEBL0_ERROR_CBLOG_ECTD_RS232OPEN_FAIL = 7  ,
    EEBL0_ERROR_CBLOG_EDC_NULL_INPUT      = 8  ,
    EEBL0_ERROR_CBLOG_EEFW_NULL_INPUT     = 9  ,
    EEBL0_ERROR_CBLOG_EEFW_RS232SEND_FAIL = 10 ,
    EEBL0_ERROR_CBLOG_EEFW_RS232READ_FAIL = 11 ,
    EEBL0_ERROR_CBLOG_EVFW_NULL_INPUT     = 12 ,
    EEBL0_ERROR_CBLOG_EVFW_FMW_NF         = 13 ,
    EEBL0_ERROR_CBLOG_EVFW_RS232SEND_FAIL = 14 ,
    EEBL0_ERROR_CBLOG_EVFW_RS232READ_FAIL = 15 ,
    EEBL0_ERROR_CBLOG_ESALFW_NULL_INPUT   = 16 ,
};

struct EEBL0;

/**
 * Creates a new EEBL0 structure which can be used to communicate with
 * specific MCUs, e.g. the MCU on the EFC-X9 by ElmorLabs.
 *
 * @param  cb_log  A callback function which gets called by the internal library
 *                 if critical errors appear. A specific error code as int type
 *                 will be passed to the given callback function.
 * @return         Returns a pointer to the malloced & pre-initialized
 *                 EEBL0 structure.
 */
struct EEBL0 * EEBL0_create(void (*cb_log)(int code));
void EEBL0_free(struct EEBL0 ** self);

/**
 * Sets the port of the given structure EEBL0 to give the possibility to connect
 * with the structure to a connected device under the given port. For example:
 *  -> Windows - COM1, COM2, COM3, ..., etc.
 *  -> Unix    -
 * @param self  {struct EEBL0 *}      EEBL0 object to the set the port for
 * @param port  {const char * const}  Port name
 */
void EEBL0_setPort(struct EEBL0 * self, const char * const port);

const char * const EEBL0_getPort(struct EEBL0 * self);
const char * const EEBL0_welcome(void);
const char * const EEBL0_getStatus(struct EEBL0 * self);

/**
 * Attempts to connect to the device under the previously set port with the passed structure EEBL0.
 * @param  self  {struct EEBL0 *}  EEBL0 object which should connect to device
 * @return       {bool}            If the connection is successful, the value 'true' is returned, otherwise the value 'false' is returned.
 */
bool EEBL0_connectToDevice(struct EEBL0 * self);
void EEBL0_disconnect(struct EEBL0 * self);

bool EEBL0_setAndLoadFirmware(struct EEBL0 * self, const char * const fw_location);
bool EEBL0_readFirmwareFromDevice(struct EEBL0 * self);
bool EEBL0_eraseFirmware(struct EEBL0 * self);
bool EEBL0_flashFirmware(struct EEBL0 * self);
bool EEBL0_verifyFirmware(struct EEBL0 * self);

#ifdef __cplusplus
}
#endif

#endif
