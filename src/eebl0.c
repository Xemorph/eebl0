#include "eebl0/eebl0.h"
#include <stdlib.h>
#include <string.h>
#include <rs232/rs_232.h>


#define LOG_ERROR_IF_FOUND(x) \
    do \
    { \
        if (eebl0->cb_log != NULL) eebl0->cb_log((x)); \
    } while (0)

static const char * const _WELCOME = "EEBL0";
static const char * const _READABLE_STATUS[] = {
    "CONNECTING", "CONNECTED", "DISCONNECTING", "DISCONNECTED", "FAILURE"
};

struct EEBL0 {
    char * port;
    int    port_id;
    int    status;
    void (*cb_log)(int code);
};

enum EEBL0_UART {
    EEBL0_UART_CMD_WELCOME = 0x00,
    EEBL0_UART_CMD_SIZE = (uint8_t)'#',
    EEBL0_UART_CMD_RUN_APP = (uint8_t)'a',
    EEBL0_UART_CMD_ERASE = (uint8_t)'e',
    EEBL0_UART_CMD_PROGRAM_PAGE = (uint8_t)'p',
    EEBL0_UART_CMD_VERIFY_PAGE = (uint8_t)'v'
};

enum EEBL0_STATUS {
    EEBL0_STATUS_CONNECTING = 0,
    EEBL0_STATUS_CONNECTED = 1,
    EEBL0_STATUS_DISCONNECTING = 2,
    EEBL0_STATUS_DISCONNECTED = 3,
    EEBL0_STATUS_FAILURE = 4
};

struct EEBL0 * EEBL0_create(void (*cb_log)(int code)) {
    struct EEBL0 * eebl0;

    eebl0 = malloc(sizeof(*eebl0));
    if (!eebl0) {
        if (cb_log != NULL) cb_log(EEBL0_ERROR_CBLOG_STDLIB_MALLOC_FAIL);
        return (NULL);
    }

    memset(eebl0, 0, sizeof(*eebl0));
    eebl0->port_id = INT_MIN;
    eebl0->status = EEBL0_STATUS_DISCONNECTED;
    // Hook the logging mechanism if available
    if (cb_log != NULL) eebl0->cb_log = cb_log;
    else eebl0->cb_log = NULL;

    return (eebl0);
}

void EEBL0_free(struct EEBL0 ** self) {
    if (self == NULL || *self == NULL) {
        LOG_ERROR_IF_FOUND(EEBL0_ERROR_CBLOG_EF_NULL_INPUT);
        return;
    }

    EEBL0_disconnect(*self); // Always call disconnect
    if ((*self)->port) {
        free((*self)->port);
        (*self)->port = NULL;
    }

    free(*self);
    *self = NULL;
}

const char * const EEBL0_welcome(void) {
    return (_WELCOME);
}

const char * const EEBL0_getStatus(struct EEBL0 * self) {
    return (_READABLE_STATUS[self->status]);
}

void EEBL0_setPort(struct EEBL0 * self, const char * const port) {
    if (self == NULL || port == NULL) {
        LOG_ERROR_IF_FOUND(EEBL0_ERROR_CBLOG_ESP_NULL_INPUT);
        return;
    }

    size_t sz = (sizeof(*port) * strlen(port)) + 1;
    self->port = malloc(sz);
    if (self->port) {
        memcpy(self->port, port, sizeof(*port) * strlen(port));
        self->port[sz-1] = '\0';
    }
}
const char * const EEBL0_getPort(struct EEBL0 * self) {
    if (self == NULL) {
        LOG_ERROR_IF_FOUND(EEBL0_ERROR_CBLOG_EGP_NULL_INPUT);
        return;
    }

    return((const char * const)self->port);
}

bool EEBL0_connect(struct EEBL0 * self) {
    if (self == NULL) {
        LOG_ERROR_IF_FOUND(EEBL0_ERROR_CBLOG_EC_NULL_INPUT);
        return;
    }

        self->status  = EEBL0_STATUS_CONNECTING;
        self->port_id = RS232_GetPortnr(self->port);
           int bdrate = 115200; // Default
    const char mode[] = {
        '8', // DATA_BITS
        'N', // PARITY
        '1', // N_STOP_BITS
        0    // Null terminator, required for underlying strlen function
    };

    if (RS232_OpenComport(self->port_id, bdrate, &mode[0], 0) != 0) {
        self->status = EEBL0_STATUS_FAILURE;
        LOG_ERROR_IF_FOUND(EEBL0_ERROR_CBLOG_EC_RS232OPEN_FAIL);
        return (false);
    }

    self->status = EEBL0_STATUS_CONNECTED;
    return (true);
}

void EEBL0_disconnect(struct EEBL0 * self) {
    if (self == NULL) {
        LOG_ERROR_IF_FOUND(EEBL0_ERROR_CBLOG_EDC_NULL_INPUT);
        return;
    }

    if (self->port_id != INT_MIN) {
        self->status = EEBL0_STATUS_DISCONNECTING;
        RS232_CloseComport(self->port_id);
        self->port_id = INT_MIN;
        self->status = EEBL0_STATUS_DISCONNECTED;
    }
}

bool EEBL0_erase(struct EEBL0 * self) {
    if (self == NULL) {
        LOG_ERROR_IF_FOUND(EEBL0_ERROR_CBLOG_EE_NULL_INPUT);
        return;
    }

    if (self->status != EEBL0_STATUS_CONNECTED) {
        return (false);
    }

    uint8_t rx;
    int bWritten = RS232_SendByte(self->port_id, EEBL0_UART_CMD_ERASE);
    if (bWritten == -1 || bWritten == 0) {
        LOG_ERROR_IF_FOUND(EEBL0_ERROR_CBLOG_EE_RS232SEND_FAIL);
        return (false);
    }

    int bRead = RS232_PollComport(self->port_id, &rx, 1);
    if (bRead == -1) {
        LOG_ERROR_IF_FOUND(EEBL0_ERROR_CBLOG_EE_RS232READ_FAIL);
        return (false);
    }

    return (rx == (uint8_t)'s');
}

bool EEBL0_verfiy(struct EEBL0 * self, const uint8_t * const data, size_t data_sz) {
    if (self == NULL || data == NULL || data_sz <= 0) {
        LOG_ERROR_IF_FOUND(EEBL0_ERROR_CBLOG_EV_NULL_INPUT);
        return (false);
    }

    // Require 16K-1K-256B sized firmware
    uint8_t bfr[15104];
    uint8_t rx[65];

    int j;
    for (j = data_sz; j < data_sz; j++)
        bfr[j] = 0xFF;

    memcpy(&bfr[0], data, data_sz > 15104 ? 15104 : data_sz);

    // Page firmware
    int page;
    for (page = 0; page < 15104 / 64; page++) {
        // Verify
        int bWritten = RS232_SendByte(self->port_id, EEBL0_UART_CMD_VERIFY_PAGE);
        if (bWritten == -1 || bWritten == 0) {
            LOG_ERROR_IF_FOUND(EEBL0_ERROR_CBLOG_EV_RS232SEND_FAIL);
            return (false);
        }

        int bRead = RS232_PollComport(self->port_id, &rx, 65);
        if (bRead == -1) {
            LOG_ERROR_IF_FOUND(EEBL0_ERROR_CBLOG_EV_RS232READ_FAIL);
            return (false);
        }
        if (rx[0] != (uint8_t)'s') {
            return (false);
        }

        // Compare page
        int y;
        for(int y = 0; y < 64; y++) {
            if(bfr[page * 64 + y] != rx[y + 1]) {
                return (false);
            }
        }
    }

    return (true);
}
