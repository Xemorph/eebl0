#include "eebl0/eebl0.h"
#include <stdlib.h>
#include <string.h>
#include <rs232/rs_232.h>

static const char * const _WELCOME = "EEBL0";
static const char * const _READABLE_STATUS[] = {
    "CONNECTING", "CONNECTED", "DISCONNECTING", "DISCONNECTED", "FAILURE"
};

struct EEBL0 {
    char * port;
    int    port_id;
    int    status;
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

struct EEBL0 * EEBL0_create(void) {
    struct EEBL0 * eebl0;

    eebl0 = malloc(sizeof(*eebl0));
    if (!eebl0) {
        return (NULL);
    }

    memset(eebl0, 0, sizeof(*eebl0));
    eebl0->port_id = INT_MIN;
    eebl0->status = EEBL0_STATUS_DISCONNECTED;

    return (eebl0);
}

void EEBL0_free(struct EEBL0 ** self) {
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
    size_t sz = (sizeof(*port) * strlen(port)) + 1;
    self->port = malloc(sz);
    if (self->port) {
        memcpy(self->port, port, sizeof(*port) * strlen(port));
        self->port[sz-1] = '\0';
    }
}
const char * const EEBL0_getPort(struct EEBL0 * self) {
    return((const char * const)self->port);
}

bool EEBL0_connect(struct EEBL0 * self) {
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
        return (false);
    }

    self->status = EEBL0_STATUS_CONNECTED;
    return (true);
}

void EEBL0_disconnect(struct EEBL0 * self) {
    if (self->port_id != INT_MIN) {
        self->status = EEBL0_STATUS_DISCONNECTING;
        RS232_CloseComport(self->port_id);
        self->port_id = INT_MIN;
        self->status = EEBL0_STATUS_DISCONNECTED;
    }
}

bool EEBL0_erase(struct EEBL0 * self) {
    if (self->status != EEBL0_STATUS_CONNECTED) {
        return (false);
    }

    uint8_t rx;
    int bWritten = RS232_SendByte(self->port_id, EEBL0_UART_CMD_ERASE);
    if (bWritten == -1 || bWritten == 0) {
        return (false);
    }

    RS232_PollComport(self->port_id, &rx, 1);
    return (rx == (uint8_t)'s');
}
