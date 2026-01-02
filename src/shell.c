#include <stdio.h>
#include <stdint.h>
#include "shell.h"

typedef enum {
    RX_IDLE,
    RX_LEN,
    RX_CMD,
    RX_PAYLOAD,
    RX_CRC
} rx_state_t;

static rx_state_t rx_state = RX_IDLE;
static uint8_t rx_len;
static uint8_t rx_cmd;
static uint8_t rx_payload[8];
static uint8_t rx_pos;
static uint8_t rx_crc;

static void tx_byte(shell_t* sh, uint8_t b){
    rb_put(&sh->tx, b);
}

static void send_ack(shell_t* sh){
    tx_byte(sh, ACK);
}

static void send_nack(shell_t* sh){
    tx_byte(sh, NACK);
}

static void handle_frame(shell_t* sh){
    switch ((cmd_t)rx_cmd){

    case CMD_SET_SPEED:
        if (rx_payload[0] <= 100){
            sh->speed = rx_payload[0];
            send_ack(sh);
        } else {
            send_nack(sh);
        }
        break;

    case CMD_STOP:
        sh->speed = 0;
        send_ack(sh);
        break;

    case CMD_GET_STAT:
        tx_byte(sh, sh->rx.dropped);
        tx_byte(sh, sh->broken_frames);
        tx_byte(sh, sh->crc_errors);
        send_ack(sh);
        break;

    default:
        send_nack(sh);
        break;
    }
}

void shell_init(shell_t* sh){
    rb_init(&sh->rx);
    rb_init(&sh->tx);

    sh->speed = 0;
    sh->ticks = 0;
    sh->broken_frames = 0;
    sh->crc_errors = 0;

    rx_state = RX_IDLE;
}

void shell_rx_bytes(shell_t* sh, const char* s){
    while (*s){
        rb_put(&sh->rx, (uint8_t)*s++);
    }
}

void shell_tick(shell_t* sh){
    sh->ticks++;

    uint8_t b;

    while (rb_get(&sh->rx, &b)){
        switch (rx_state){

        case RX_IDLE:
            if (b == STX){
                rx_state = RX_LEN;
                rx_crc = 0;
            }
            break;

        case RX_LEN:
            rx_len = b;
            rx_pos = 0;
            rx_crc ^= b;
            rx_state = RX_CMD;
            break;

        case RX_CMD:
            rx_cmd = b;
            rx_crc ^= b;
            rx_state = (rx_len > 1) ? RX_PAYLOAD : RX_CRC;
            break;

        case RX_PAYLOAD:
            rx_payload[rx_pos++] = b;
            rx_crc ^= b;
            if (rx_pos == rx_len - 1)
                rx_state = RX_CRC;
            break;

        case RX_CRC:
            if (rx_crc == b){
                handle_frame(sh);
            } else {
                sh->crc_errors++;
                send_nack(sh);
            }
            rx_state = RX_IDLE;
            break;
        }
    }

    /* TX */
    while (rb_get(&sh->tx, &b)){
       printf("0x%02X ", b);

    }
}
