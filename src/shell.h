#pragma once
#include <stdint.h>
#include "ringbuf.h"

/* ===== Protokół ===== */
#define STX  0x02
#define ACK  0x06
#define NACK 0x15

typedef enum {
    CMD_SET_SPEED = 0x01,
    CMD_STOP      = 0x02,
    CMD_GET_STAT  = 0x03
} cmd_t;

/* ===== Stan urządzenia / driver ===== */
typedef struct {
    rb_t rx, tx;

    /* stan urządzenia */
    uint8_t speed;

    /* telemetria */
    unsigned ticks;
    unsigned broken_frames;
    unsigned crc_errors;
} shell_t;

/* ===== API ===== */
void shell_init(shell_t* sh);
void shell_rx_bytes(shell_t* sh, const char* s);
void shell_tick(shell_t* sh);
