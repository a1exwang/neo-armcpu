#include <ulib.h>
#include <stdio.h>
#include <string.h>
#include <file.h>
#include "sl811.h"
#include "usb.h"


#define printf(...)                     fprintf(1, __VA_ARGS__)

inline void msleep(int ms) {
    int i;
    for (i = 0; i < ms * (1000); ++i) {
        __nop;
    }
}


void usage() {
    printf("sl811 [args]\n");
    printf("  p [addr] [size], default, addr=0,size=0x10, in hex, print regs\n");
    printf("  i, print info\n");
    printf("  w reg val, write reg\n");
    printf("  r reg, read reg\n");
    printf("  e <ops_str>, send packets, ops can be s,i,t\n");
    printf("  get_desc, get descriptor\n");
}


void sl811_write_buf(int base, const char *buf, int n) {
    int i;
    for (i = 0; i < n; ++i) 
      sl811_write(base + i, buf[i]);
}
void sl811_read_buf(int base, char *buf, int n) {
    int i;
    for (i = 0; i < n; ++i) 
      buf[i] = sl811_read(base + i);
}

void print_mem(const char *start, int count) {
    int x = 0;
    int i = 0, j;
    for (; i < 8; ++i) {
        printf("0x%08x:  ", (unsigned int)(start + i * 16));
        for (j = 0; j < 16; ++j) {
            printf("%02x ", sl811_read(start[i * 16 + j]));
            x += 1;
            if (x == count) {
                printf("\n");
                return;
            }
        }
        printf("\n");
    }
}

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
    sl811_write(0, 0);
    sl811_write(3, 0);
    sl811_write(4, 0);
    sl811_write(SL11H_IRQ_ENABLE, 0);
    sl811_write(SL11H_IRQ_STATUS, ~1);
    sl811_write(SL11H_CTLREG1, SL11H_CTL1MASK_SE0);
    mdelay(20);

    sl811_write(SL11H_IRQ_ENABLE, 0);
    sl811_write(SL11H_CTLREG1, 0);
    sl811_write(SL811HS_CTLREG2, SL811HS_CTL2_INIT);
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

int isset(unsigned x, int bit) {
    return (x & (1 << bit)) == 0 ? 0 : 1;
}

void print_sl811_info() {
    int cr1 = sl811_read(SL11H_CTLREG1);
    int ien = sl811_read(SL11H_IRQ_ENABLE);
    int ist = sl811_read(SL11H_IRQ_STATUS);
    int rev = sl811_read(SL11H_HWREVREG) >> 4;

    printf("Ctrl Regs:\n");
    printf("  CR1: (Sus,Speed,JK,RST,SOF)\n");
    printf("       (%d,%d,%d,%d,%d)\n", 
        isset(cr1, 6), isset(cr1, 5), isset(cr1,4), isset(cr1,3),isset(cr1,0));

    printf("  IRQ_EN: (Detect,I/R,SOFTimer,B-Done,A-Done)\n");
    printf("       (%d,%d,%d,%d,%d)\n", 
        isset(ien, 6), isset(ien, 5), isset(ien,4), isset(ien,1),isset(ien,0));

    printf("  IRQ_ST: (Detect,I/R,SOFTimer,B-Done,A-Done)\n");
    printf("       (%d,%d,%d,%d,%d)\n", 
        isset(ist, 6), isset(ist, 5), isset(ist,4), isset(ist,1),isset(ist,0));
    printf("  REV: %x\n", rev);

    int hcr = sl811_read(SL11H_HOSTCTLREG);
    int addr = sl811_read(SL11H_BUFADDRREG);
    int len = sl811_read(SL11H_BUFLNTHREG);
    int st = sl811_read(SL11H_PKTSTATREG);
    int n_tx = sl811_read(SL11H_XFERCNTREG);
    printf("USB Regs:\n");
    printf("  HCR: (Pre,D0/1,SyncSOF,ISO,Direction,En,Arm)\n");
    printf("       (%d,%d,%d,%d,%d,%d,%d)\n", 
        isset(hcr, 7), isset(hcr,6),isset(hcr,5),isset(hcr,4), isset(hcr,2),isset(hcr,1),isset(hcr,0));

    printf("  Addr/Len: (%x, %x)\n", addr, len);
    printf("  Status: (Stall,NAK,Overflow,Setup,Seq,Timeout,Err,ACK)\n");
    printf("          (%d,%d,%d,%d,%d,%d,%d,%d)\n",
        isset(st,7),isset(st,6),isset(st,5),isset(st,4),isset(st,3),isset(st,2),isset(st,1),isset(st,0));
    printf("  N Transmitted %x\n", n_tx);
}

#define sleep_us(x) 

int transmit_cnt = 0;

int wait_transfer() {
    volatile unsigned int st, ctl;
    int i;
    // 20ms
    for (i = 0; i < 100000; ++i) {
        ctl = sl811_read(SL11H_HOSTCTLREG);
        if ((ctl & 1) == 0) {
            st = sl811_read(SL11H_PKTSTATREG);
            if ((st & 0xE7) == 1) {
                return i;
            }
            else if ((st & 0xE7) != 0) {
                printf("st: %02x\n", st);
                return -1;
            }
        }
    }
    return -2;
}

const char get_desc[8] = {0x80,6,0,1,0,0,0x12,0};
int setup_packet(const struct usb_setup_pkt* ppkt,
                 int ep, 
                 int addr, 
                 int is_in) {
    printf("1\n");
    int buf_addr = 0x10;
    sl811_write_buf(buf_addr, (unsigned char*)ppkt, sizeof(struct usb_setup_pkt));
    sl811_write(SL11H_BUFADDRREG, buf_addr);
    sl811_write(SL11H_BUFLNTHREG, sizeof(struct usb_setup_pkt));
    sl811_write(SL11H_PIDEPREG, SL_SETUP | ep);
    sl811_write(SL11H_DEVADDRREG, addr);
    sl811_write(SL11H_HOSTCTLREG, SL11H_HCTLMASK_ARM | SL11H_HCTLMASK_ENABLE | (is_in ? SL11H_HCTLMASK_IN : SL11H_HCTLMASK_OUT));
    printf("1\n");
    int p = wait_transfer();
    if (p != 0) {
        print_sl811_info();
        if (p == -2)
            printf("setup_packet timeout\n");
        else
            printf("setup_packet error\n");
        return -1;
    }
    printf("1\n");
    sl811_write(SL11H_IRQ_STATUS, 0);
    return 0;
}

// IN DATA1
int in_packet(char *buf,
              int len, 
              int ep, 
              int addr) {
    int buf_addr = 0x20;
    sl811_write(SL11H_BUFADDRREG, buf_addr);
    sl811_write(SL11H_BUFLNTHREG, len);
    sl811_write(SL11H_PIDEPREG, SL_IN | ep);
    sl811_write(SL11H_DEVADDRREG, addr);
    sl811_write(SL11H_HOSTCTLREG, SL11H_HCTLMASK_ARM | SL11H_HCTLMASK_ENABLE | SL11H_HCTLMASK_IN);
    int p = wait_transfer();
    if (p != 0) {
        print_sl811_info();
        if (p == -2)
            printf("in_packet timeout\n");
        else
            printf("in packet error\n");
        return -1;
    }
    sl811_read_buf(buf_addr, buf, len);
    sl811_write(SL11H_IRQ_STATUS, 0);
    return 0;
}

// DATA1
int status_packet(int ep, 
                  int addr) {
    int buf_addr = 0x20;
    sl811_write(SL11H_BUFADDRREG, buf_addr);
    sl811_write(SL11H_BUFLNTHREG, 0);
    sl811_write(SL11H_PIDEPREG, SL_OUT | ep);
    sl811_write(SL11H_DEVADDRREG, addr);
    sl811_write(SL11H_HOSTCTLREG, SL11H_HCTLMASK_ARM | SL11H_HCTLMASK_ENABLE | SL11H_HCTLMASK_OUT | SL11H_HCTLMASK_TOGGLE);

    int p = wait_transfer();
    if (p != 0) {
        if (p == -2)
            printf("status_packet timeout\n");
        else
            printf("status_packet error\n");
        print_sl811_info();
        return -1;
    }
    return 0;
}

static const char *cmd;
static int reg, data;
static int start = 0, count = 16;
static char buf[0x100];
static int a,b,c;
static int ep = 0, addr = 0;
static struct usb_setup_pkt pkt = {
  USB_REQ_TYPE_IN,
  GET_DESCRIPTOR,
  ((USB_DESC_TYPE_DEVICE) << 8) | 0,
  0,
  sizeof(struct usb_dev_desc)
};
static struct usb_dev_desc desc;

#define SET(ptr, member, val) \
      struct_set((ptr), \
                (unsigned int)(&(ptr)->member) - (unsigned int)(ptr), \
                sizeof((ptr)->member), \
                (val))

void struct_set(void *ptr, int offset, int len, unsigned int val) {
    unsigned int addr = (unsigned int)ptr + offset;
    unsigned int aligned_addr = (addr & 0xFFFFFFFC);
    unsigned int off = addr - aligned_addr;
    unsigned int cell_data = *((unsigned int*)aligned_addr);
    unsigned int cell_zeroed = cell_data & ~(((1<<(len*8))-1) << (off*8));
    unsigned int data_shifted = (val & ((1<<(len*8))-1)) << (off*8);
    *((unsigned int*)aligned_addr) = cell_zeroed | data_shifted;
}

int
main(int argc, char **argv) {

    if (argc >= 2) {
        cmd = argv[1];
    }

    if (strcmp(cmd, "setup") == 0) {
        setup_sl811();
    }
    else if (strcmp(cmd, "i") == 0) {
        print_sl811_info();
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
    else if (strcmp(cmd, "getdesc") == 0 && argc == 2) {
        printf("1\n");
        SET(&pkt, idx, 0);
        printf("1\n");
        a = setup_packet(&pkt, 0, ep, addr);
        printf("1\n");
        b = in_packet((char*)&desc, sizeof(struct usb_dev_desc), ep, addr); 
        c = status_packet(ep, addr);
        printf("In packet: ");
        print_mem((const char*)&desc, sizeof(desc));
        printf("Cycles: %d, %d, %d\n", a, b, c);
    }
    else if (strcmp(cmd, "msleep") == 0 && argc == 3) {
        msleep(hex2i(argv[2]));
    }
    return 0;
}

