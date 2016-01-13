#include <stdio.h>
#include <stdint.h>
#include "ET1502B.h"

#define QPRINTF	DbgPrintf
#define MY_QPRINTF(fmt, ...)//		DbgPrintf
// ce-link log debug 
/* module log enable switchs */
#define BOSCH_MODULE_LOG_EN		0
#define DATA_MANAGER_LOG_EN		0
#define DATA_TRANSFER_LOG_EN	0
#define ARI_MODULE_LOG_EN		0
#define SLEEP_RCD_LOG_EN    0
//#define DEBUG_PROTOCOL_EN   	0
//#define DEBUG_RTC_EN 			0
//#define DEBUG_SPI_EN 			0
//#define DEBUG_BUTTON_EN 		0
//#define MENU_MODULE_LOG_EN		0
// BATTERY_MODULE_LOG_EN	0
//#define DEBUG_SLEEP_EN  0
//#define DEBUG_DISP_EN 0


/* system debug level define */
#define CE_LINK_LOG_NULL		0
#define CE_LINK_LOG_ERR	     	1
#define CE_LINK_LOG_WARING     	2
#define CE_LINK_LOG_INFO     	3
#define CE_LINK_LOG_DUG	     	4

#define CE_LINK_LOG_LEVEL		CE_LINK_LOG_DUG


#if CE_LINK_LOG_LEVEL == CE_LINK_LOG_DUG
    #define CE_DEBUG	QPRINTF
	#define CE_INFO		QPRINTF
	#define CE_WARING	QPRINTF
	#define CE_ERROR	QPRINTF
#elif CE_LINK_LOG_LEVEL == CE_LINK_LOG_INFO
	#define CE_DEBUG(fmt, ...)
	#define CE_INFO		QPRINTF
	#define CE_WARING	QPRINTF
	#define CE_ERROR	QPRINTF
#elif CE_LINK_LOG_LEVEL == CE_LINK_LOG_WARING
	#define CE_DEBUG(fmt, ...)
	#define CE_INFO(fmt, ...)
	#define CE_WARING	QPRINTF
	#define CE_ERROR	QPRINTF
#elif CE_LINK_LOG_LEVEL == CE_LINK_LOG_ERR
	#define CE_DEBUG(fmt, ...)
	#define CE_INFO(fmt, ...)
	#define CE_WARING(fmt, ...)
	#define CE_ERROR	QPRINTF
#else
	#define CE_DEBUG(fmt, ...)
	#define CE_INFO(fmt, ...)
	#define CE_WARING(fmt, ...)
	#define CE_ERROR(fmt, ...)
#endif
//==================================
#if BOSCH_MODULE_LOG_EN
	#define BOSCH_DUG		CE_DEBUG
	#define BOSCH_INFO		CE_INFO
	#define BOSCH_WAR		CE_WARING
	#define BOSCH_ERR		CE_ERROR
#else
	#define BOSCH_DUG(fmt, ...)
	#define BOSCH_INFO(fmt, ...)
	#define BOSCH_WAR(fmt, ...)
	#define BOSCH_ERR(fmt, ...)
#endif

#if DATA_MANAGER_LOG_EN
	#define DATA_MANA_DUG		CE_DEBUG
	#define DATA_MANA_INFO	    CE_INFO
	#define DATA_MANA_WAR		CE_WARING
	#define DATA_MANA_ERR		CE_ERROR
#else
	#define DATA_MANA_DUG(fmt, ...)
	#define DATA_MANA_INFO(fmt, ...)
	#define DATA_MANA_WAR(fmt, ...)
	#define DATA_MANA_ERR(fmt, ...)
#endif

#if DATA_TRANSFER_LOG_EN
	#define DATA_TRAN_DUG		CE_DEBUG
	#define DATA_TRAN_INFO		CE_INFO
	#define DATA_TRAN_WAR		CE_WARING
	#define DATA_TRAN_ERR		CE_ERROR
#else
	#define DATA_TRAN_DUG(fmt, ...)
	#define DATA_TRAN_INFO(fmt, ...)
	#define DATA_TRAN_WAR(fmt, ...)
	#define DATA_TRAN_ERR(fmt, ...)
#endif

#if ARI_MODULE_LOG_EN
	#define ARITH_DUG		CE_DEBUG
	#define ARITH_INFO		CE_INFO
	#define ARITH_WAR		CE_WARING
	#define ARITH_ERR		CE_ERROR
#else
	#define ARITH_DUG(fmt, ...)
	#define ARITH_INFO(fmt, ...)
	#define ARITH_WAR(fmt, ...)
	#define ARITH_ERR(fmt, ...)
#endif

#if SLEEP_RCD_LOG_EN
  #define SLP_RCD_DUG		CE_DEBUG
	#define SLP_RCD_INFO    CE_INFO
	#define SLP_RCD_WAR		CE_WARING
	#define SLP_RCD_ERR		CE_ERROR
#else
    #define SLP_RCD_DUG(fmt, ...)
	#define SLP_RCD_INFO(fmt, ...)
	#define SLP_RCD_WAR(fmt, ...)
	#define SLP_RCD_ERR(fmt, ...)
#endif




//=================================================

extern void DbgPrintf( const char * format, ... );
extern void DbgPrintfbuff(uint8_t * tx_buf, uint16_t len );
extern void DbgPrintf_COS_log( const char * format, ... );
extern void DbgPrintf_nop( const char * format, ... );



