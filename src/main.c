#include <stdint.h>
#include "shell.h"

static void send_frame(shell_t* sh,
                       uint8_t cmd,
                       uint8_t* payload,
                       uint8_t payload_len)
{
    uint8_t crc = 0;

    rb_put(&sh->rx, STX);

    rb_put(&sh->rx, payload_len + 1);   
    crc ^= (payload_len + 1);

    rb_put(&sh->rx, cmd);
    crc ^= cmd;

    for (uint8_t i = 0; i < payload_len; i++){
        rb_put(&sh->rx, payload[i]);
        crc ^= payload[i];
    }

    rb_put(&sh->rx, crc);
}

int main(void){
    shell_t sh;
    shell_init(&sh);

    uint8_t speed = 80;
    send_frame(&sh, CMD_SET_SPEED, &speed, 1);
    for (int i = 0; i < 10; i++) shell_tick(&sh);

    send_frame(&sh, CMD_GET_STAT, NULL, 0);
    for (int i = 0; i < 10; i++) shell_tick(&sh);

    rb_put(&sh.rx, STX);
    rb_put(&sh.rx, 1);
    rb_put(&sh.rx, CMD_STOP);
    rb_put(&sh.rx, 0xFF);   
    for (int i = 0; i < 10; i++) shell_tick(&sh);

    for (int i = 0; i < 200; i++){
        send_frame(&sh, CMD_STOP, NULL, 0);
    }
    for (int i = 0; i < 50; i++) shell_tick(&sh);

    return 0;
}
