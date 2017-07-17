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
    int x = 0;
    int i = 0, j;
    printf("sl811_read_buf\n");
    for (; i < 16; ++i) {
        printf("0x%02x:  ", base + i * 16);
        for (j = 0; j < 16; ++j) {
            int val = sl811_read(base + i * 16 + j);
            printf("%02x ", x);
            buf[x] = val;
            x += 1;
            if (x == n) {
                printf("\n");
                return;
            }
        }
        printf("\n");
    }
}
void sl811_read_buf_fast(int base, char *buf, int n) {
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
                return i + 1;
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
                 int addr){
    printf("1\n");
    int buf_addr = 0x10;
    sl811_write_buf(buf_addr, (unsigned char*)ppkt, sizeof(struct usb_setup_pkt));
    sl811_write(SL11H_BUFADDRREG, buf_addr);
    sl811_write(SL11H_BUFLNTHREG, sizeof(struct usb_setup_pkt));
    sl811_write(SL11H_PIDEPREG, SL_SETUP | ep);
    sl811_write(SL11H_DEVADDRREG, addr);
    sl811_write(SL11H_HOSTCTLREG, SL11H_HCTLMASK_ARM | SL11H_HCTLMASK_ENABLE | SL11H_HCTLMASK_OUT);
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

static char g_buf[0x10000];

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

void usb_get_dev_desc(struct usb_dev_desc *desc, int ep, int addr) {
    int a, b, c;
    struct usb_setup_pkt pkt;
    SET(&pkt, req_type, USB_REQ_TYPE_IN | USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_DEVICE);
    SET(&pkt, req, GET_DESCRIPTOR);
    SET(&pkt, val, (USB_DESC_TYPE_DEVICE) << 8 | 0);
    SET(&pkt, idx, 0);
    SET(&pkt, cnt, sizeof(struct usb_dev_desc));
    a = setup_packet(&pkt, ep, addr);
    b = in_packet((char*)desc, sizeof(struct usb_dev_desc), ep, addr); 
    c = status_packet(ep, addr);
    printf("Cycles: %d, %d, %d\n", a, b, c);
    printf("DeviceDescriptor returned:\n");
    print_mem((const char*)desc, sizeof(*desc));
}

void usb_set_address(int ep, int addr, int new_addr) {
    int a, b; char buf[10];
    struct usb_setup_pkt pkt;
    SET(&pkt, req_type, USB_REQ_TYPE_OUT | USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_DEVICE);
    SET(&pkt, req, SET_ADDRESS);
    SET(&pkt, val, new_addr);
    SET(&pkt, idx, 0);
    SET(&pkt, cnt, 0);
    a = setup_packet(&pkt, ep, addr);
    b = in_packet(buf, 0, ep, addr);
    /* c = status_packet(ep, addr); */
    printf("Cycles: %d, %d\n", a, b);
    printf("Address set: %x\n", new_addr);
}

void usb_get_conf_desc(char *buf, int len, int ep, int addr) {
    int a, b, c, n_read = 0, n_max = 120, rest = len, offset = 0, pkt_size = 0;
    struct usb_setup_pkt pkt;
    SET(&pkt, req_type, USB_REQ_TYPE_IN | USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_DEVICE);
    SET(&pkt, req, GET_DESCRIPTOR);
    SET(&pkt, val, (USB_DESC_TYPE_CONF) << 8 | 0);
    SET(&pkt, idx, 0);
    SET(&pkt, cnt, len);
    a = setup_packet(&pkt, ep, addr);
    while (1) {
        offset = len - rest;
        if (rest > n_max)
            pkt_size = n_max;
        else
            pkt_size = rest;
        rest -= pkt_size;
        b = in_packet(((char*)g_buf + offset), pkt_size, ep, addr); 
        if (rest <= 0)
            break;
    }
    c = status_packet(ep, addr);
    printf("Cycles: %d, %d, %d\n", a, b, c);
    printf("ConfigurationDescriptor returned:\n");
    print_mem((const char*)buf, len);
}
void usb_get_str_desc(char *buf, int *plen, int ep, int addr, int idx) {
    int a, b, c, max_packet_size = 0x3C; // 60Bytes
    struct usb_setup_pkt pkt;
    SET(&pkt, req_type, USB_REQ_TYPE_IN | USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_DEVICE);
    SET(&pkt, req, GET_DESCRIPTOR);
    SET(&pkt, val, (USB_DESC_TYPE_STR) << 8 | idx);
    SET(&pkt, idx, 0);
    SET(&pkt, cnt, max_packet_size);
    a = setup_packet(&pkt, ep, addr);
    b = in_packet((char*)buf, max_packet_size, ep, addr); 
    c = status_packet(ep, addr);
    *plen = sl811_read(0x20);
    printf("Cycles: %d, %d, %d\n", a, b, c);
    printf("StringDescriptor returned, len %d:\n", *plen);
    print_mem((const char*)buf, *plen);
}

int main(int argc, char **argv) {
    const char *cmd;
    int reg, data;

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
        int start = 0, count = 0x10;
        if (argc >= 3)
            start = hex2i(argv[2]);
        if (argc >= 4)
            count = hex2i(argv[3]);
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
    else if (strcmp(cmd, "getdesc") == 0 && argc >= 2) {
        int addr = 0;
        struct usb_dev_desc desc;
        if (argc >= 3)
            addr = hex2i(argv[2]);
        usb_get_dev_desc(&desc, 0, addr);
    }
    else if (strcmp(cmd, "setaddr") == 0 && argc >= 2) {
        int addr = 0, new_addr = 1;
        if (argc >= 3)
            addr = hex2i(argv[2]);
        if (argc >= 4)
            new_addr = hex2i(argv[3]);
        usb_set_address(0, addr, new_addr);
    }
    else if (strcmp(cmd, "getconf") == 0 && argc >= 2) {
        int addr = 1, len = sizeof(struct usb_conf_desc);
        if (argc >= 3)
            addr = hex2i(argv[2]);
        if (argc >= 4)
            len = hex2i(argv[3]);
        usb_get_conf_desc(g_buf, len, 0, addr);
    }
    else if (strcmp(cmd, "getstr") == 0 && argc >= 4) {
        int addr, idx, len;
        addr = hex2i(argv[2]);
        idx = hex2i(argv[3]);
        usb_get_str_desc(g_buf, &len, 0, addr, idx);
        g_buf[len] = 0;
        printf("StringDescriptor: '%s'", g_buf);
    }
    else if (strcmp(cmd, "msleep") == 0 && argc == 3) {
        msleep(hex2i(argv[2]));
    }
    return 0;
}

