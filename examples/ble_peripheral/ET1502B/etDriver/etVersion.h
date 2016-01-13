//#include "ble_gap.h"
#define DEVICE_NAME                     "BSB15020200011" 
#define DEVICE_MAC_ADDR                 "\x02\x00\x00\x02\x02\x15"

//#define DEVICE_MAC_ADDR                 (0x00,("\x05\x05\x05\x05\x01\x18"))
//const ble_gap_addr_t  addr={0,{0x55,0x22,0x33,0x44,0x55,0x66}};
//const ble_gap_addr_t   DEVICE_MAC_ADDR={0,{0x11,0x22,0x33,0x44,0x55,0x66}};

extern void Get_KeyVersion(unsigned char * prt,unsigned char *str_length);
extern void Get_HWVersion(unsigned char * prt,unsigned char *str_length);
extern void Get_COSVersion(unsigned char * prt,unsigned char *str_length);
