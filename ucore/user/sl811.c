#include <ulib.h>
#include <stdio.h>
#include <string.h>
#include <file.h>
#include "sl811.h"


#define printf(...)                     fprintf(1, __VA_ARGS__)

int hex2i(const char *hex) {
    int len = strlen(hex);
    int i;
    int sum = 0;
    char c;
    int base = 1;
    for (i = 0; i < len; ++i) {
        c = hex[len - i - 1];
        if ('0' <= c && c <= '9') {
            sum += (c - '0') * base;
        }
        else if ('a' <= c && c <= 'f') {
            sum += (c - 'a' + 10) * base;
        }
        else if ('A' <= c && c <= 'F') {
            sum += (c - 'A' + 10) * base;
        }
        base <<= 4;
    }
    return sum;
}

void setup_sl811() {
    sl811_write(SL11H_IRQ_ENABLE, 0);
    sl811_write(SL11H_IRQ_STATUS, ~0);
    sl811_write(SL11H_CTLREG1, SL11H_CTL1MASK_SE0);
    mdelay(20);

    sl811_write(SL11H_IRQ_ENABLE, 0);
    /* sl811_write(SL11H_CTLREG1, sl811->ctrl1); */
    /* sl811_write(SL811HS_CTLREG2, SL811HS_CTL2_INIT); */
}

void print_sl811(int start, int count) {
    int x = 0;
    int i = 0, j;
    for (; i < 8; ++i) {
        printf("0x%02x:  ", start + i * 16);
        for (j = 0; j < 16; ++j) {
            printf("%02x ", sl811_read(start + i * 16 + j));
            x += 1;
            if (x == count) {
                printf("\n");
                return;
            }
        }
        printf("\n");
    }
}
int
main(int argc, char **argv) {
    const char *cmd;
    int reg, data;
    int start = 0, count = 16;
    if (argc >= 2) {
        cmd = argv[1];
    }

    if (strcmp(cmd, "setup") == 0) {
        setup_sl811();
    }
    else if (strcmp(cmd, "p") == 0) {
        if (argc >= 3)
            start = hex2i(argv[2]);
        if (argc >= 4)
            start = hex2i(argv[3]);
        print_sl811(start, count);
    }
    else if (strcmp(cmd, "r") == 0 && argc == 3) {
        reg = hex2i(argv[2]);
        printf("%2x\n", sl811_read(reg));
    }
    else if (strcmp(cmd, "w") == 0 && argc == 4) {
        reg = hex2i(argv[2]);
        data = hex2i(argv[3]);
        sl811_write(reg, data);
    }
    return 0;
}

