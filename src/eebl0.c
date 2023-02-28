#include "eebl0/eebl0.h"

#include <sys/types.h>
#include <sys/stat.h>

#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <rs232/rs_232.h>


#define LOG_ERROR_IF_FOUND(x) \
    do \
    { \
        if (self->cb_log != NULL) self->cb_log((x)); \
    } while (0)

static const char * const _WELCOME = "EEBL0";
static const char * const _READABLE_STATUS[] = {
    "CONNECTING", "CONNECTED", "DISCONNECTING", "DISCONNECTED", "FAILURE"
};

bool __load_and_pad_firmware(struct EEBL0 * self, const char * const fw);

struct EEBL0 {
    // Port name
    char * port;
    // Underlying port id of 'port'
    int    port_id;
    // Current status of the connection
    int    status;
    // Firmware
    uint8_t * fw;
    // Size of the raw (binary) firmware data
    size_t fw_sz;
    // Callback log function
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
    eebl0->fw_sz = INT_MIN;
    eebl0->status = EEBL0_STATUS_DISCONNECTED;
    // Hook the logging mechanism if available
    if (cb_log != NULL) eebl0->cb_log = cb_log;
    else eebl0->cb_log = NULL;

    return (eebl0);
}

void EEBL0_free(struct EEBL0 ** self) {
    if (self == NULL || *self == NULL) {
        //LOG_ERROR_IF_FOUND(EEBL0_ERROR_CBLOG_EF_NULL_INPUT);
        return;
    }

    EEBL0_disconnect(*self); // Always call disconnect
    if ((*self)->port) {
        free((*self)->port);
        (*self)->port = NULL;
    }

    if ((*self)->fw) {
        free((*self)->fw);
        (*self)->fw = NULL;
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
        return NULL;
    }

    return((const char * const)self->port);
}

bool EEBL0_connectToDevice(struct EEBL0 * self) {
    if (self == NULL) {
        LOG_ERROR_IF_FOUND(EEBL0_ERROR_CBLOG_ECTD_NULL_INPUT);
        return (false);
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
        LOG_ERROR_IF_FOUND(EEBL0_ERROR_CBLOG_ECTD_RS232OPEN_FAIL);
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

bool EEBL0_setAndLoadFirmware(struct EEBL0 * self, const char * const fw_location) {
    if (self == NULL || fw_location == NULL) {
        LOG_ERROR_IF_FOUND(EEBL0_ERROR_CBLOG_ESALFW_NULL_INPUT);
        return (false);
    }

    return (__load_and_pad_firmware(self, fw_location));
}

bool EEBL0_eraseFirmware(struct EEBL0 * self) {
    if (self == NULL) {
        LOG_ERROR_IF_FOUND(EEBL0_ERROR_CBLOG_EEFW_NULL_INPUT);
        return (false);
    }

    if (self->status != EEBL0_STATUS_CONNECTED) {
        return (false);
    }

    uint8_t rx;
    int bWritten = RS232_SendByte(self->port_id, EEBL0_UART_CMD_ERASE);
    if (bWritten == -1 || bWritten == 0) {
        LOG_ERROR_IF_FOUND(EEBL0_ERROR_CBLOG_EEFW_RS232SEND_FAIL);
        return (false);
    }

    int bRead = RS232_PollComport(self->port_id, &rx, 1);
    if (bRead == -1) {
        LOG_ERROR_IF_FOUND(EEBL0_ERROR_CBLOG_EEFW_RS232READ_FAIL);
        return (false);
    }

    return (rx == (uint8_t)'s');
}

bool EEBL0_verfiyFirmware(struct EEBL0 * self) {
    if (self == NULL) {
        LOG_ERROR_IF_FOUND(EEBL0_ERROR_CBLOG_EVFW_NULL_INPUT);
        return (false);
    }

    if (self->fw == NULL || self->fw_sz <= 0) {
        LOG_ERROR_IF_FOUND(EEBL0_ERROR_CBLOG_EVFW_FMW_NF);
        return (false);
    }

    // Read buffer
    uint8_t rx[65];
    // Page firmware
    int page;
    for (page = 0; page < 15104 / 64; page++) {
        // Verify
        int bWritten = RS232_SendByte(self->port_id, EEBL0_UART_CMD_VERIFY_PAGE);
        if (bWritten == -1 || bWritten == 0) {
            LOG_ERROR_IF_FOUND(EEBL0_ERROR_CBLOG_EVFW_RS232SEND_FAIL);
            return (false);
        }

        int bRead = RS232_PollComport(self->port_id, &rx[0], 65);
        if (bRead == -1) {
            LOG_ERROR_IF_FOUND(EEBL0_ERROR_CBLOG_EVFW_RS232READ_FAIL);
            return (false);
        }
        if (rx[0] != (uint8_t)'s') {
            return (false);
        }

        // Compare page
        int y;
        for(int y = 0; y < 64; y++) {
            if(self->fw[page * 64 + y] != rx[y + 1]) {
                return (false);
            }
        }
    }

    return (true);
}

//#if defined(__linux__) || defined(__FreeBSD__)
//bool __load_and_pad_firmware(struct EEBL0 * self, const char * const fw) {
//    struct stat _fw_stats;
//    fstat(fd, &_fw_stats);
//    long int _fw_sz = _fw_stats.st_size;
//
//    return (true);
//}
//#else
bool __load_and_pad_firmware(struct EEBL0 * self, const char * const fw) {
    // Read file statistics
    struct _stat _fw_stats;
    int rc = _stat(fw, &_fw_stats);
    if (rc) {
        LOG_ERROR_IF_FOUND(EEBL0_ERROR_CBLOG_SYSSTAT_STAT_FAIL);
        return (false);
    }
    // Firmware size through file statistics
    long int _fw_sz = _fw_stats.st_size;

    // TODO: C-Standard functions are fine but OS-specific functions are faster
    //       and better. So ... Use OS-specific functions!
    // Open file
    FILE * _fw = fopen(fw, "rb");
    if (_fw == NULL) {
        LOG_ERROR_IF_FOUND(EEBL0_ERROR_CBLOG_FOPEN_FAIL);
        return (false);
    }

    // Require 16K-1K-256B sized firmware
    self->fw = (uint8_t *)malloc( (self->fw_sz = sizeof(*(self->fw)) * 15104) );
    if (fw == NULL) {
        LOG_ERROR_IF_FOUND(EEBL0_ERROR_CBLOG_STDLIB_MALLOC_FAIL);
        return (false);
    }
    // Pad firmware if required
    int j;
    for (j = _fw_sz; j < self->fw_sz; j++)
        self->fw[j] = 0xFF;
    // Load firmware from file to allocated memory
    fread(self->fw, _fw_sz > self->fw_sz ? self->fw_sz : _fw_sz, 1, _fw);
    // Close file
    fclose(_fw);

    return (true);
}
//#endif
