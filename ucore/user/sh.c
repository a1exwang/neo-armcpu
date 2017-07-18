#include <ulib.h>
#include <stdio.h>
#include <string.h>
#include <dir.h>
#include <file.h>
#include <error.h>
#include <unistd.h>
#include <readline.h>

#define printf(...)                     fprintf(1, __VA_ARGS__)
#define putc(c)                         printf("%c", c)

#define BUFSIZE                         4096
#define WHITESPACE                      " \t\r\n"
#define SYMBOLS                         "<|>&;"

char shcwd[BUFSIZE];

int
gettoken(char **p1, char **p2) {
    char *s;
    if ((s = *p1) == NULL) {
        return 0;
    }
    while (strchr(WHITESPACE, *s) != NULL) {
        *s ++ = '\0';
    }
    if (*s == '\0') {
        return 0;
    }

    *p2 = s;
    int token = 'w';
    if (strchr(SYMBOLS, *s) != NULL) {
        token = *s, *s ++ = '\0';
    }
    else {
        bool flag = 0;
        while (*s != '\0' && (flag || strchr(WHITESPACE SYMBOLS, *s) == NULL)) {
            if (*s == '"') {
                *s = ' ', flag = !flag;
            }
            s ++;
        }
    }
    *p1 = (*s != '\0' ? s : NULL);
    return token;
}

void
usage(void) {
    printf("usage: sh [command-file]\n");
}

int
reopen(int fd2, const char *filename, uint32_t open_flags) {
    int ret, fd1;
    close(fd2);
    if ((ret = open(filename, open_flags)) >= 0 && ret != fd2) {
        close(fd2);
        fd1 = ret, ret = dup2(fd1, fd2);
        close(fd1);
    }
    return ret < 0 ? ret : 0;
}

int
testfile(const char *name) {
    int ret;
    if ((ret = open(name, O_RDONLY)) < 0) {
        return ret;
    }
    close(ret);
    return 0;
}

int
runcmd(char *cmd) {
    static char argv0[BUFSIZE];
    const char *argv[EXEC_MAX_ARG_NUM + 1];
    char *t;
    int argc, token, ret, p[2];
again:
    argc = 0;
    while (1) {
        switch (token = gettoken(&cmd, &t)) {
        case 'w':
            if (argc == EXEC_MAX_ARG_NUM) {
                printf("sh error: too many arguments\n");
                return -1;
            }
            argv[argc ++] = t;
            break;
        case '<':
            if (gettoken(&cmd, &t) != 'w') {
                printf("sh error: syntax error: < not followed by word\n");
                return -1;
            }
            if ((ret = reopen(0, t, O_RDONLY)) != 0) {
                return ret;
            }
            break;
        case '>':
            if (gettoken(&cmd, &t) != 'w') {
                printf("sh error: syntax error: > not followed by word\n");
                return -1;
            }
            if ((ret = reopen(1, t, O_RDWR | O_TRUNC | O_CREAT)) != 0) {
                return ret;
            }
            break;
        case '|':
          //  if ((ret = pipe(p)) != 0) {
          //      return ret;
          //  }
            if ((ret = fork()) == 0) {
                close(0);
                if ((ret = dup2(p[0], 0)) < 0) {
                    return ret;
                }
                close(p[0]), close(p[1]);
                goto again;
            }
            else {
                if (ret < 0) {
                    return ret;
                }
                close(1);
                if ((ret = dup2(p[1], 1)) < 0) {
                    return ret;
                }
                close(p[0]), close(p[1]);
                goto runit;
            }
            break;
        case 0:
            goto runit;
        case ';':
            if ((ret = fork()) == 0) {
                goto runit;
            }
            else {
                if (ret < 0) {
                    return ret;
                }
                waitpid(ret, NULL);
                goto again;
            }
            break;
        default:
            printf("sh error: bad return %d from gettoken\n", token);
            return -1;
        }
    }

runit:
    if (argc == 0) {
        return 0;
    }
    else if (strcmp(argv[0], "cd") == 0) {
        if (argc != 2) {
            return -1;
        }
        strcpy(shcwd, argv[1]);
        return 0;
    }
    if ((ret = testfile(argv[0])) != 0) {
        if (ret != -E_NOENT) {
            return ret;
        }
        snprintf(argv0, sizeof(argv0), "/%s", argv[0]);
        argv[0] = argv0;
    }
    argv[argc] = NULL;
    return __exec(NULL, argv);
}

int fork_run_command(char *buffer) {
  int ret;
  printf("\r\n");
  shcwd[0] = '\0';
  int pid;
  if ((pid = fork()) == 0) {
    ret = runcmd(buffer);
    exit(ret);
  }
  assert(pid >= 0);
  if (waitpid(pid, &ret) == 0) {
    if (ret == 0 && shcwd[0] != '\0') {
      ret = 0;
    }
    if (ret != 0) {
      printf("error: %d - %e\n", ret, ret);
    }
  }
  return ret;
}

/*
 * SL811HS register declarations and HCD data structures
 *
 * Copyright (C) 2004 Psion Teklogix
 * Copyright (C) 2004 David Brownell
 * Copyright (C) 2001 Cypress Semiconductor Inc. 
 */

/*
 * SL811HS has transfer registers, and control registers.  In host/master
 * mode one set of registers is used; in peripheral/slave mode, another.
 *  - SL11H only has some "A" transfer registers from 0x00-0x04
 *  - SL811HS also has "B" registers from 0x08-0x0c
 *  - SL811S (or HS in slave mode) has four A+B sets, at 00, 10, 20, 30
 */

#define SL811_EP_A(base)	((base) + 0)
#define SL811_EP_B(base)	((base) + 8)

#define SL811_HOST_BUF		0x00
#define SL811_PERIPH_EP0	0x00
#define SL811_PERIPH_EP1	0x10
#define SL811_PERIPH_EP2	0x20
#define SL811_PERIPH_EP3	0x30


/* TRANSFER REGISTERS:  host and peripheral sides are similar
 * except for the control models (master vs slave).
 */
#define SL11H_HOSTCTLREG	0
#	define SL11H_HCTLMASK_ARM	0x01
#	define SL11H_HCTLMASK_ENABLE	0x02
#	define SL11H_HCTLMASK_IN	0x00
#	define SL11H_HCTLMASK_OUT	0x04
#	define SL11H_HCTLMASK_ISOCH	0x10
#	define SL11H_HCTLMASK_AFTERSOF	0x20
#	define SL11H_HCTLMASK_TOGGLE	0x40
#	define SL11H_HCTLMASK_PREAMBLE	0x80
#define SL11H_BUFADDRREG	1
#define SL11H_BUFLNTHREG	2
#define SL11H_PKTSTATREG	3	/* read */
#	define SL11H_STATMASK_ACK	0x01
#	define SL11H_STATMASK_ERROR	0x02
#	define SL11H_STATMASK_TMOUT	0x04
#	define SL11H_STATMASK_SEQ	0x08
#	define SL11H_STATMASK_SETUP	0x10
#	define SL11H_STATMASK_OVF	0x20
#	define SL11H_STATMASK_NAK	0x40
#	define SL11H_STATMASK_STALL	0x80
#define SL11H_PIDEPREG		3	/* write */
#	define	SL_SETUP	0xd0
#	define	SL_IN		0x90
#	define	SL_OUT		0x10
#	define	SL_SOF		0x50
#	define	SL_PREAMBLE	0xc0
#	define	SL_NAK		0xa0
#	define	SL_STALL	0xe0
#	define	SL_DATA0	0x30
#	define	SL_DATA1	0xb0
#define SL11H_XFERCNTREG	4	/* read */
#define SL11H_DEVADDRREG	4	/* write */


/* CONTROL REGISTERS:  host and peripheral are very different.
 */
#define SL11H_CTLREG1		5
#	define SL11H_CTL1MASK_SOF_ENA	0x01
#	define SL11H_CTL1MASK_FORCE	0x18
#		define SL11H_CTL1MASK_NORMAL	0x00
#		define SL11H_CTL1MASK_SE0	0x08	/* reset */
#		define SL11H_CTL1MASK_J		0x10
#		define SL11H_CTL1MASK_K		0x18	/* resume */
#	define SL11H_CTL1MASK_LSPD	0x20
#	define SL11H_CTL1MASK_SUSPEND	0x40
#define SL11H_IRQ_ENABLE	6
#	define SL11H_INTMASK_DONE_A	0x01
#	define SL11H_INTMASK_DONE_B	0x02
#	define SL11H_INTMASK_SOFINTR	0x10
#	define SL11H_INTMASK_INSRMV	0x20	/* to/from SE0 */
#	define SL11H_INTMASK_RD		0x40
#	define SL11H_INTMASK_DP		0x80	/* only in INTSTATREG */
#define SL11S_ADDRESS		7

/* 0x08-0x0c are for the B buffer (not in SL11) */

#define SL11H_IRQ_STATUS	0x0D	/* write to ack */
#define SL11H_HWREVREG		0x0E	/* read */
#	define SL11H_HWRMASK_HWREV	0xF0
#define SL11H_SOFLOWREG		0x0E	/* write */
#define SL11H_SOFTMRREG		0x0F	/* read */

/* a write to this register enables SL811HS features.
 * HOST flag presumably overrides the chip input signal?
 */
#define SL811HS_CTLREG2		0x0F
#	define SL811HS_CTL2MASK_SOF_MASK	0x3F
#	define SL811HS_CTL2MASK_DSWAP		0x40
#	define SL811HS_CTL2MASK_HOST		0x80

#define SL811HS_CTL2_INIT	(SL811HS_CTL2MASK_HOST | 0x2e)


/* DATA BUFFERS: registers from 0x10..0xff are for data buffers;
 * that's 240 bytes, which we'll split evenly between A and B sides.
 * Only ISO can use more than 64 bytes per packet.
 * (The SL11S has 0x40..0xff for buffers.)
 */
#define H_MAXPACKET	120		/* bytes in A or B fifos */

#define SL11H_DATA_START	0x10
#define	SL811HS_PACKET_BUF(is_a)	((is_a) \
		? SL11H_DATA_START \
		: (SL11H_DATA_START + H_MAXPACKET))
#define __nop { __asm__ __volatile__("nop"); }

int mdelay(int ms) {
  volatile int i = 0;
  volatile int  sum = 0;
  for (; i < 50000; ++i) {
    *((volatile int *)&sum) += 1;
    __nop;
  }
  return sum;
}

void sl811_write(unsigned char reg, unsigned char data) {
    volatile int *sl811_ctrl = (int*)0xaf000000;
    *sl811_ctrl = reg;
    __nop;
    __nop;
    __nop;
    __nop;
    __nop;
    *sl811_ctrl = data;
    __nop;
    __nop;
    __nop;
    __nop;
    __nop;
}
unsigned char sl811_read(unsigned char reg) {
    volatile int *sl811_ctrl = (int*)0xaf000000;
    volatile const unsigned int *sl811_data = (const unsigned int*)0xaf000004;
    *sl811_ctrl = reg;
    __nop;
    __nop;
    __nop;
    __nop;
    __nop;
    return *sl811_data;
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
                return;
            }
        }
        printf("\n");
    }
}

int
main(int argc, char **argv) {
    printf("user sh is running!!!\n");
    int ret, interactive = 1;
    if (argc == 2) {
        if ((ret = reopen(0, argv[1], O_RDONLY)) != 0) {
            return ret;
        }
        interactive = 0;
    }
    else if (argc > 2) {
        usage();
        return -1;
    }
    //shcwd = malloc(BUFSIZE);
    assert(shcwd != NULL);

    /* char *cmd1 = "ls"; */
    /* int ret1 = fork_run_command(cmd1); */
    /* printf("%s (%d)\n", cmd1, ret1); */
    
    // sl811
    printf("sl811[HWREV] = %02x\n", sl811_read(SL11H_HWREVREG));

    /* printf("pre-init\n"); */
    /* print_sl811(0, 16); */

    /* printf("sl811 setup\n"); */
    /* setup_sl811(); */
    /* sl811_write(SL11H_IRQ_ENABLE, 1); */
    /* print_sl811(0, 16); */


    char *buffer;
    while ((buffer = readline((interactive) ? "$ " : NULL)) != NULL) {
      fork_run_command(buffer);
    }
    return 0;
}

