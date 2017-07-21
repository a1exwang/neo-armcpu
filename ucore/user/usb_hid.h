#pragma once


struct usb_hid_report {
    unsigned char modifier;
    unsigned char reserved;
    unsigned char usage_indexes[6];

} __attribute__ ((packed));

char usb_hid_usage_table[] = {
  0,0,0,0,'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z',
// 30
  '1','2','3','4','5','6','7','8','9','0',
// 40
  '\r',0,'\b','\t',' ',0,0,0,0,0,0,0,0,0,0,0,0,0,'f','f','f','f','f','f','f','f','f','f','f','f',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'f','f','f','f','f','f','f','f','f','f','f','f',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0

};
