#include <stdbool.h>
#include <stdint.h>

#include "nrf.h"
#include "bsp.h"
#include "app_timer.h"
#include "app_error.h"
#include "et_nrf_drv_wdt.h"
//#include "nrf_drv_wdt.h"
//#include "nrf_drv_clock.h"
//#include "nrf_delay.h"
#include "app_util_platform.h"
//================
#include "ET1502B.h"
#include "et_debug.h"
#include "et_WDT.h"
#if ENABLE_WDT
nrf_drv_wdt_channel_id m_channel_id;

void wdt_event_handler(void)
{
//    DbgPrintf("WDT_event_handler\r\n");
    //NOTE: The max amount of time we can spend in WDT interrupt is two cycles of 32768[Hz] clock - after that, reset occurs
	
}
void et_WDT_init(void)
{
	  uint32_t err_code = NRF_SUCCESS;
    //Configure WDT.
    nrf_drv_wdt_config_t config = NRF_DRV_WDT_DEAFULT_CONFIG;
    err_code = nrf_drv_wdt_init(&config, wdt_event_handler);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_drv_wdt_channel_alloc(&m_channel_id);
    APP_ERROR_CHECK(err_code);
    nrf_drv_wdt_enable();
}
void et_WDT_feed(void)
{
  nrf_drv_wdt_channel_feed(m_channel_id);
}
#endif
