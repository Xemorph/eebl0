#pragma once

#ifndef __flash_eebl0_h_
#define __flash_eebl0_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

struct EEBL0;

struct EEBL0 * EEBL0_create(void);
void EEBL0_free(struct EEBL0 ** self);

void EEBL0_setPort(struct EEBL0 * self, const char * const port);
const char * const EEBL0_getPort(struct EEBL0 * self);

const char * const EEBL0_welcome(void);
const char * const EEBL0_getStatus(struct EEBL0 * self);

bool EEBL0_connect(struct EEBL0 * self);
void EEBL0_disconnect(struct EEBL0 * self);
bool EEBL0_erase(struct EEBL0 * self);

#ifdef __cplusplus
}
#endif

#endif
