//#include "macro.h"
#include "ET1502B.h"
#include <stdio.h>
#include <stdarg.h>  
#include <string.h>
#include "et_debug.h"
//#include "app_uart.h"


#include <stdint.h>
#ifdef ENABLE_SIMPLE_UART
#include "etSimple_uart.h"
#endif
/*
void uart_debug(const char *dbgmsg)
{
	//unsigned char  buff[100] = {0};
	unsigned int len = 0,len2=strlen(dbgmsg);

	for(len=0;len<len2;len++)
	{
		 //app_uart_put(dbgmsg[len]);
			simple_uart_put(dbgmsg[len]);
	}
}

void Dbg_Print_Str(const char* dbgmsg)
{
	//DbgPrintf( "%s",dbgmsg );
	uart_debug(dbgmsg);
}
*/

void DbgPrintf( const char * format, ... )
{
//	int ret=0;
#if DEBUG_UART_EN 

#ifndef ENABLE_SIMPLE_UART //kevin add	
	  /*    char tx_buf[256];
        int msg_len,len;
        va_list args;
        va_start(args, format);
        msg_len = vsprintf(tx_buf, format, args);
        va_end(args);

			
				for(len=0;len<msg_len;len++)
				{
					simple_uart_put(tx_buf[len]);
					//while(app_uart_put(tx_buf[len]) != NRF_SUCCESS);

					//simple_uart_put(tx_buf[len]);
				}*/
#else	
        #if ENABLE_COS
	      char tx_buf[256];
				//int msg_len,len;
        va_list args;
        va_start(args, format);
        vsprintf(tx_buf, format, args);
        va_end(args);				
				simple_uart_putstring((const uint8_t *)tx_buf);
				//simple_uart_putstringbuff(tx_buf,msg_len);
        #endif
#endif
#endif			
	//	return ret;
}

void DbgPrintfbuff(uint8_t * tx_buf, uint16_t len )
{
#if DEBUG_UART_EN 
#ifndef ENABLE_SIMPLE_UART //kevin add	
	   /*   char tx_buf[256];
        int msg_len,len;
        va_list args;
        va_start(args, format);
        msg_len = vsprintf(tx_buf, format, args);
        va_end(args);

			
				for(len=0;len<msg_len;len++)
				{
					simple_uart_put(tx_buf[len]);
					//while(app_uart_put(tx_buf[len]) != NRF_SUCCESS);

					//simple_uart_put(tx_buf[len]);
				}*/
#else	
        //uint16_t msg_len;			
      #if ENABLE_COS
				simple_uart_putstringbuff(tx_buf,len);
      #endif
#endif
#endif
}

void DbgPrintf_COS_log( const char * format, ... )
{
//	int ret=0;
#if DEBUG_UART_EN
     #if ENABLE_COS
	      char tx_buf[256];
				int msg_len,i;
        unsigned char check=0;
        //int msg_len;
        va_list args;
        va_start(args, format);
        msg_len=vsprintf(tx_buf+4, format, args);        
        tx_buf[0]=0xaa;
        tx_buf[1]=0xdd;
        tx_buf[2]=0;
        tx_buf[3]=msg_len;
        for (i = 1; i < msg_len+1; i++)
        {
            check += tx_buf[i++];
        }
        tx_buf[msg_len+4]=(~check)&0xff;
        tx_buf[msg_len+5]=0x55;
        
        va_end(args);				
				//simple_uart_putstring((const uint8_t *)tx_buf);
				simple_uart_putstringbuff((uint8_t *)tx_buf,msg_len+6);
     #endif

#endif			
	//	return ret;
}
void DbgPrintf_ble_log( unsigned char *str,unsigned int len)
{
//	int ret=0;
#if DEBUG_BLE_EN
     #if ENABLE_COS
	      char tx_buf[400];
				int i=0,offset=0;
        unsigned char check=0;
     
        tx_buf[0]=0xaa;
        tx_buf[1]=0xdd;
        check +=0xdd;
        tx_buf[2]=len/256;
        tx_buf[3]=len%256;
        check +=tx_buf[2];
        check +=tx_buf[3];
        offset=4;
        for (i = 0; i < len; i++)
        {
            check += str[i];
            tx_buf[offset]=str[i];
            offset++;
        }
        tx_buf[offset++]=(~check)&0xff;
        tx_buf[offset++]=0x55;
        
        //va_end(args);				
				//simple_uart_putstring((const uint8_t *)tx_buf);
				simple_uart_putstringbuff((uint8_t *)tx_buf,offset);
     #endif

#endif			
	//	return ret;
}

void DbgPrintf_nop( const char * format, ... )
{
}

