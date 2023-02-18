#pragma once

#ifndef __flash_eebl0_h_
#define __flash_eebl0_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

enum
{
    EEBL0_ERROR_CBLOG_SUCCESS             = 0  ,
    EEBL0_ERROR_CBLOG_STDLIB_MALLOC_FAIL  = 1  ,
    EEBL0_ERROR_CBLOG_EF_NULL_INPUT       = 2  ,
    EEBL0_ERROR_CBLOG_ESP_NULL_INPUT      = 3  ,
    EEBL0_ERROR_CBLOG_ESP_NULL_INPUT      = 4  ,
    EEBL0_ERROR_CBLOG_EC_NULL_INPUT       = 5  ,
    EEBL0_ERROR_CBLOG_EC_RS232OPEN_FAIL   = 6  ,
    EEBL0_ERROR_CBLOG_EDC_NULL_INPUT      = 7  ,
    EEBL0_ERROR_CBLOG_EE_NULL_INPUT       = 8  ,
    EEBL0_ERROR_CBLOG_EE_RS232SEND_FAIL   = 9  ,
    EEBL0_ERROR_CBLOG_EE_RS232READ_FAIL   = 10 ,
    EEBL0_ERROR_CBLOG_EV_NULL_INPUT       = 11 ,
    EEBL0_ERROR_CBLOG_EV_RS232SEND_FAIL   = 12 ,
    EEBL0_ERROR_CBLOG_EV_RS232READ_FAIL   = 13 ,
};

struct EEBL0;

struct EEBL0 * EEBL0_create(void (*cb_log)(int code));
void EEBL0_free(struct EEBL0 ** self);

void EEBL0_setPort(struct EEBL0 * self, const char * const port);
const char * const EEBL0_getPort(struct EEBL0 * self);

const char * const EEBL0_welcome(void);
const char * const EEBL0_getStatus(struct EEBL0 * self);

bool EEBL0_connect(struct EEBL0 * self);
void EEBL0_disconnect(struct EEBL0 * self);
bool EEBL0_erase(struct EEBL0 * self);
bool EEBL0_verfiy(struct EEBL0 * self, uint8_t * data, size_t data_sz);

#ifdef __cplusplus
}
#endif

#endif
