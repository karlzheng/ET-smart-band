/* Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

/** @file
 *
 * @defgroup ble_sdk_uart_over_ble_main main.c
 * @{
 * @ingroup  ble_sdk_app_nus_eval
 * @brief    UART over BLE application main file.
 *
 * This file contains the source code for a sample application that uses the Nordic UART service.
 * This application uses the @ref srvlib_conn_params module.
 */

#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "app_error.h"
#include "nrf51_bitfields.h"
#include "ble.h"
#include "ble_hci.h"
#include "ble_srv_common.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "softdevice_handler.h"
#include "app_timer.h"
#include "app_button.h"
#include "ble_nus.h"
#ifdef BLE_DFU_APP_SUPPORT
#include "ble_dfu.h"
#include "dfu_app_handler.h"
#endif // BLE_DFU_APP_SUPPORT

#ifndef ENABLE_SIMPLE_UART //kevin add
//#include "app_uart.h"
#endif
#include "app_util_platform.h"
#include "bsp.h"
#include "bsp_btn_ble.h"

//#include "ble_conn_params.h"
#include "boards.h"
//#include "sensorsim.h"
//#include "softdevice_handler.h"
//#include "app_timer.h"
#include "device_manager.h"
#include "pstorage.h"
#include "app_trace.h"
#include "bsp.h"
#include "nrf_delay.h"
#include "bsp_btn_ble.h"
//#include "app_util.h"
//=========================
#include "ET1502B.h"
#include "et_debug.h"
#include "btprotocol.h"
#include "etSpi.h"
#include "etMainloop.h"
#include "etkeyboard.h"
#include "ke_timer.h"
#include "usr_task.h"
#include "uartProtocol.h"
#include "etLcdDisplayApp.h"
#include "etVersion.h"
#include "etSpi.h"
#include "gSensor_driver.h"
#include "et_flash_offset.h"
#include "etLcdDisplayApp.h"

#ifdef ENABLE_SIMPLE_UART //kevin add
#include "etSimple_uart.h"
//#include "etSimple_uart.h"
#endif
#define IS_SRVC_CHANGED_CHARACT_PRESENT 1//0                                           /**< Include the service_changed characteristic. If not enabled, the server's database cannot be changed for the lifetime of the device. */

//#define DEVICE_NAME                     "Nordic_UART"                               /**< Name of device. Will be included in the advertising data. */
#define NUS_SERVICE_UUID_TYPE           BLE_UUID_TYPE_BLE//BLE_UUID_TYPE_VENDOR_BEGIN                  /**< UUID type for the Nordic UART Service (vendor specific). */

#define APP_ADV_SLOW_INTERVAL                1280//                                          /**< The advertising interval (in units of 0.625 ms. This value corresponds to 40 ms). */
#define APP_ADV_SLOW_TIMEOUT_IN_SECONDS      300//60                                         /**< The advertising timeout (in units of seconds). */

#define APP_ADV_FAST_INTERVAL           320//64
#define APP_ADV_FAST_TIMEOUT_IN_SECONDS 10

#define APP_TIMER_PRESCALER             0                                           /**< Value of the RTC1 PRESCALER register. */
#define APP_1SECOND_TIMER 1
#define APP_TIMER_MAX_TIMERS            (2 + BSP_APP_TIMERS_NUMBER+APP_1SECOND_TIMER+USER_TIMER_COUNT)                 /**< Maximum number of simultaneously created timers. */
#define APP_TIMER_OP_QUEUE_SIZE         4                                           /**< Size of timer operation queues. */

#define MIN_CONN_INTERVAL               MSEC_TO_UNITS(7.5, UNIT_1_25_MS) //MSEC_TO_UNITS(20, UNIT_1_25_MS)             /**< Minimum acceptable connection interval (20 ms), Connection interval uses 1.25 ms units. */
#define MAX_CONN_INTERVAL               MSEC_TO_UNITS(40, UNIT_1_25_MS)//MSEC_TO_UNITS(75, UNIT_1_25_MS)             /**< Maximum acceptable connection interval (75 ms), Connection interval uses 1.25 ms units. */
#define SLAVE_LATENCY                   0                                           /**< Slave latency. */
#define CONN_SUP_TIMEOUT                MSEC_TO_UNITS(4000, UNIT_10_MS)             /**< Connection supervisory timeout (4 seconds), Supervision Timeout uses 10 ms units. */
#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(5000, APP_TIMER_PRESCALER)  /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(30000, APP_TIMER_PRESCALER) /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT    3                                           /**< Number of attempts before giving up the connection parameter negotiation. */

//#define START_STRING                    "Start...\n"                                /**< The string that will be sent over the UART when the application starts. */
#ifdef BLE_DFU_APP_SUPPORT
#define SEC_PARAM_BOND                   1                                          /**< Perform bonding. */
#define SEC_PARAM_MITM                   0                                          /**< Man In The Middle protection not required. */
#define SEC_PARAM_IO_CAPABILITIES        BLE_GAP_IO_CAPS_NONE                       /**< No I/O capabilities. */
#define SEC_PARAM_OOB                    0                                          /**< Out Of Band data not available. */
#define SEC_PARAM_MIN_KEY_SIZE           7                                          /**< Minimum encryption key size. */
#define SEC_PARAM_MAX_KEY_SIZE           16   
#endif

#define DEAD_BEEF                       0xDEADBEEF                                  /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

#ifdef BLE_DFU_APP_SUPPORT
#define DFU_REV_MAJOR                    0x00                                       /** DFU Major revision number to be exposed. */
#define DFU_REV_MINOR                    0x01                                       /** DFU Minor revision number to be exposed. */
#define DFU_REVISION                     ((DFU_REV_MAJOR << 8) | DFU_REV_MINOR)     /** DFU Revision number to be exposed. Combined of major and minor versions. */
#define APP_SERVICE_HANDLE_START         0x000C                                     /**< Handle of first application specific service when when service changed characteristic is present. */
#define BLE_HANDLE_MAX                   0xFFFF                                     /**< Max handle value in BLE. */

STATIC_ASSERT(IS_SRVC_CHANGED_CHARACT_PRESENT);                                     /** When having DFU Service support in application the Service Changed Characteristic should always be present. */
#endif // BLE_DFU_APP_SUPPORT

#define UART_TX_BUF_SIZE                256                                         /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE                256                                         /**< UART RX buffer size. */

static ble_nus_t                        m_nus;                                      /**< Structure to identify the Nordic UART Service. */
#if ENABLE_NEW_SERVECE
static new_server_t           new_m_nus;
#endif
static uint16_t                         m_conn_handle = BLE_CONN_HANDLE_INVALID;    /**< Handle of the current connection. */

//static ble_uuid_t                       m_adv_uuids[] = {{BLE_UUID_NUS_SERVICE, NUS_SERVICE_UUID_TYPE}};  /**< Universally unique service identifier. */
static ble_uuid_t                       m_adv_uuids[] = {{ADV_UUID, NUS_SERVICE_UUID_TYPE}};  /**< Universally unique service identifier. */
#ifdef BLE_DFU_APP_SUPPORT    
static ble_dfu_t                         m_dfus;                                    /**< Structure used to identify the DFU service. */

static dm_application_instance_t         m_app_handle;                              /**< Application identifier allocated by device manager */

#endif // BLE_DFU_APP_SUPPORT  
uint32_t bt_send_to_AG(uint8_t* buff,uint16_t temp_length,UINT8 ser_flag)
{
	//uint32_t	   err_code;
	//err_code=ble_nus_string_send(&m_nus, buff, temp_length);

   if(ser_flag==SERVICE_COS)
   {
   return ble_nus_string_send(&m_nus, buff, temp_length);
   }
	 else
	 {
	 return new_service_ble_nus_string_send(&new_m_nus, buff, temp_length);
	 }
}

void start_Rssi(void)
{
	//sd_ble_gap_rssi_get(m_conn_handle,p_rssi);
	sd_ble_gap_rssi_start(m_conn_handle,2,15);//kevin add
	disp_str_rssi.LCD_disp_Rssi_flag=1;
#if (DEBUG_UART_EN)
		DbgPrintf("start_Rssi\r\n");
#endif	

}
/*
uint32_t Get_Rssi(int8_t *p_rssi)
{
	return sd_ble_gap_rssi_get(m_conn_handle,p_rssi);
}
*/
void Get_Rssi_stop(void)
{
		  if(disp_str_rssi.LCD_disp_Rssi_flag==1)
		  {
       sd_ble_gap_rssi_stop(m_conn_handle);
			disp_str_rssi.LCD_disp_Rssi_flag=0;
#if (DEBUG_UART_EN)
			DbgPrintf("Get_Rssi_stop\r\n");
#endif						 
      }
}
/**@brief Function for assert macro callback.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyse 
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num    Line number of the failing ASSERT call.
 * @param[in] p_file_name File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}


/**@brief Function for the GAP initialization.
 *
 * @details This function will set up all the necessary GAP (Generic Access Profile) parameters of 
 *          the device. It also sets the permissions and appearance.
 */
static void gap_params_init(void)
{
    uint32_t                err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;
#if ENABLE_GET_FLASH_ID
    unsigned char keyid[40];
#endif
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);
  //=============================
#if ENABLE_GET_FLASH_ID
    etSpim1ReadNorFlashStdMode(KEYID_START_ADDR,keyid,40);
		if((keyid[TAG1_ADDR_OFFSET]==MAC_VALID_TAG1) && (keyid[TAG2_ADDR_OFFSET]==MAC_VALID_TAG2))
		{
		 err_code = sd_ble_gap_device_name_set(&sec_mode,
																						(const uint8_t *) &keyid[KEYID_DATA_ADDR_OFFSET],
																						keyid[KEYID_LENGHT_ADDR_OFFSET]);
       
		}
		else    
#endif
		{      
    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *) DEVICE_NAME,
                                          strlen(DEVICE_NAME));
		}
#if ENABLE_COS  
    SetToGetKEYID();
#endif
  //=======================  
    APP_ERROR_CHECK(err_code);
    
    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}
#ifdef BLE_DFU_APP_SUPPORT
/**@brief Function for stopping advertising.
 */
static void advertising_stop(void)
{
    uint32_t err_code;

    err_code = sd_ble_gap_adv_stop();
    APP_ERROR_CHECK(err_code);

    err_code = bsp_indication_set(BSP_INDICATE_IDLE);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for loading application-specific context after establishing a secure connection.
 *
 * @details This function will load the application context and check if the ATT table is marked as 
 *          changed. If the ATT table is marked as changed, a Service Changed Indication
 *          is sent to the peer if the Service Changed CCCD is set to indicate.
 *
 * @param[in] p_handle The Device Manager handle that identifies the connection for which the context 
 *                     should be loaded.
 */
static void app_context_load(dm_handle_t const * p_handle)
{
    uint32_t                 err_code;
    static uint32_t          context_data;
    dm_application_context_t context;

    context.len    = sizeof(context_data);
    context.p_data = (uint8_t *)&context_data;

    err_code = dm_application_context_get(p_handle, &context);
    if (err_code == NRF_SUCCESS)
    {
        // Send Service Changed Indication if ATT table has changed.
        if ((context_data & (DFU_APP_ATT_TABLE_CHANGED << DFU_APP_ATT_TABLE_POS)) != 0)
        {
            err_code = sd_ble_gatts_service_changed(m_conn_handle, APP_SERVICE_HANDLE_START, BLE_HANDLE_MAX);
            if ((err_code != NRF_SUCCESS) &&
                (err_code != BLE_ERROR_INVALID_CONN_HANDLE) &&
                (err_code != NRF_ERROR_INVALID_STATE) &&
                (err_code != BLE_ERROR_NO_TX_BUFFERS) &&
                (err_code != NRF_ERROR_BUSY) &&
                (err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING))
            {
                APP_ERROR_HANDLER(err_code);
            }
        }

        err_code = dm_application_context_delete(p_handle);
        APP_ERROR_CHECK(err_code);
    }
    else if (err_code == DM_NO_APP_CONTEXT)
    {
        // No context available. Ignore.
    }
    else
    {
        APP_ERROR_HANDLER(err_code);
    }
}


/** @snippet [DFU BLE Reset prepare] */
/**@brief Function for preparing for system reset.
 *
 * @details This function implements @ref dfu_app_reset_prepare_t. It will be called by 
 *          @ref dfu_app_handler.c before entering the bootloader/DFU.
 *          This allows the current running application to shut down gracefully.
 */
static void reset_prepare(void)
{
    uint32_t err_code;

    if (m_conn_handle != BLE_CONN_HANDLE_INVALID)
    {
        // Disconnect from peer.
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
        APP_ERROR_CHECK(err_code);
        err_code = bsp_indication_set(BSP_INDICATE_IDLE);
        APP_ERROR_CHECK(err_code);
    }
    else
    {
        // If not connected, the device will be advertising. Hence stop the advertising.
        advertising_stop();
    }

    err_code = ble_conn_params_stop();
    APP_ERROR_CHECK(err_code);

    nrf_delay_ms(500);
}
/** @snippet [DFU BLE Reset prepare] */
#endif // BLE_DFU_APP_SUPPORT


/**@brief Function for handling the data from the Nordic UART Service.
 *
 * @details This function will process the data received from the Nordic UART BLE Service and send
 *          it to the UART module.
 *
 * @param[in] p_nus    Nordic UART Service structure.
 * @param[in] p_data   Data to be send to UART module.
 * @param[in] length   Length of the data.
 */
/**@snippet [Handling the data received over BLE] */
static void nus_data_handler(ble_nus_t * p_nus, uint8_t * p_data, uint16_t length)
{
  Protocol_data_manage(p_data,length);
}

static void new_serve_nus_data_handler(new_server_t * p_nus, uint8_t * p_data, uint16_t length)
{
  newService_Protocol_data_manage(p_data,length);
}
/**@snippet [Handling the data received over BLE] */


/**@brief Function for initializing services that will be used by the application.
 */
static void services_init(void)
{
    uint32_t       err_code;
    ble_nus_init_t nus_init;
#if ENABLE_NEW_SERVECE
    new_service_nus_init_t nus_init_2;
    memset(&nus_init_2, 0, sizeof(nus_init_2));
#endif
    memset(&nus_init, 0, sizeof(nus_init));

    nus_init.data_handler = nus_data_handler;  
    err_code = ble_nus_init(&m_nus, &nus_init);
    APP_ERROR_CHECK(err_code);
#if ENABLE_NEW_SERVECE
    nus_init_2.data_handler = new_serve_nus_data_handler;
    err_code = new_server_nus_init(&new_m_nus, &nus_init_2);
    APP_ERROR_CHECK(err_code);
#endif

#ifdef BLE_DFU_APP_SUPPORT
    /** @snippet [DFU BLE Service initialization] */
    ble_dfu_init_t   dfus_init;

    // Initialize the Device Firmware Update Service.
    memset(&dfus_init, 0, sizeof(dfus_init));

    dfus_init.evt_handler   = dfu_app_on_dfu_evt;
    dfus_init.error_handler = NULL;
    dfus_init.evt_handler   = dfu_app_on_dfu_evt;
    dfus_init.revision      = DFU_REVISION;

    err_code = ble_dfu_init(&m_dfus, &dfus_init);
    APP_ERROR_CHECK(err_code);

    dfu_app_reset_prepare_set(reset_prepare);
    dfu_app_dm_appl_instance_set(m_app_handle);
    /** @snippet [DFU BLE Service initialization] */
#endif // BLE_DFU_APP_SUPPORT

}


/**@brief Function for handling an event from the Connection Parameters Module.
 *
 * @details This function will be called for all events in the Connection Parameters Module
 *          which are passed to the application.
 *
 * @note All this function does is to disconnect. This could have been done by simply setting
 *       the disconnect_on_fail config parameter, but instead we use the event handler
 *       mechanism to demonstrate its use.
 *
 * @param[in] p_evt  Event received from the Connection Parameters Module.
 */
static void on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
    uint32_t err_code;
    
    if(p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
}


/**@brief Function for handling errors from the Connection Parameters module.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for initializing the Connection Parameters module.
 */
static void conn_params_init(void)
{
    uint32_t               err_code;
    ble_conn_params_init_t cp_init;
    
    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail             = false;
    cp_init.evt_handler                    = on_conn_params_evt;
    cp_init.error_handler                  = conn_params_error_handler;
    
    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for putting the chip into sleep mode.
 *
 * @note This function will not return.
 */
/*
static void sleep_mode_enter(void)
{
    uint32_t err_code = bsp_indication_set(BSP_INDICATE_IDLE);
    APP_ERROR_CHECK(err_code);

    // Prepare wakeup buttons.
    err_code = bsp_btn_ble_sleep_mode_prepare();
    APP_ERROR_CHECK(err_code);

    // Go to system-off mode (this function will not return; wakeup will cause a reset).
    err_code = sd_power_system_off();
    APP_ERROR_CHECK(err_code);
}
*/

/**@brief Function for handling advertising events.
 *
 * @details This function will be called for advertising events which are passed to the application.
 *
 * @param[in] ble_adv_evt  Advertising event.
 */
static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
    uint32_t err_code;
    switch (ble_adv_evt)
    {
        case BLE_ADV_EVT_FAST:
            //err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
            //APP_ERROR_CHECK(err_code);
            break;
        case BLE_ADV_EVT_IDLE:
         //sleep_mode_enter();//kevin delete
#if DEBUG_UART_EN    
//    DbgPrintf("go to advertising8\r\n");
#endif         
				err_code = ble_advertising_start(BLE_ADV_MODE_SLOW);//KEVIN 
        APP_ERROR_CHECK(err_code);
            break;
        default:
            break;
    }
}


/**@brief Function for the Application's S110 SoftDevice event handler.
 *
 * @param[in] p_ble_evt S110 SoftDevice event.
 */
static void on_ble_evt(ble_evt_t * p_ble_evt)
{
    uint32_t                         err_code;
  
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED://0x10   
            //Protocol_set_MTU(GATT_MTU_SIZE_DEFAULT);
            Protocol_set_BT_connected();//kevin
            err_code = bsp_indication_set(BSP_INDICATE_CONNECTED);
            APP_ERROR_CHECK(err_code);
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;				    
            break;
            
        case BLE_GAP_EVT_DISCONNECTED://0x11
            Protocol_set_BT_Disconnected();//kevin
            err_code = bsp_indication_set(BSP_INDICATE_IDLE);
            APP_ERROR_CHECK(err_code);
            m_conn_handle = BLE_CONN_HANDLE_INVALID;
            break;

        case BLE_GAP_EVT_SEC_PARAMS_REQUEST://0x13
            // Pairing not supported
            err_code = sd_ble_gap_sec_params_reply(m_conn_handle, BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP, NULL, NULL);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_SYS_ATTR_MISSING:
            // No system attributes have been stored.
            err_code = sd_ble_gatts_sys_attr_set(m_conn_handle, NULL, 0, 0);
            APP_ERROR_CHECK(err_code);
            break;
        case BLE_EVT_TX_COMPLETE://0x01 kevin add
            Protocol_set_TX_complete();
 #if (DEBUG_UART_EN)
			//DbgPrintf("TX_COMPLETE\r\n");
#endif          
          //Protocol_data_to_AG_continue();
				break;
				case BLE_GATTS_EVT_WRITE://0x50
				break;
				case BLE_GAP_EVT_TIMEOUT://0x19
				break;
				case BLE_GAP_EVT_RSSI_CHANGED://0x1a
 #if (DEBUG_UART_EN)
					//	DbgPrintf("CHANGED RSSI =%d,distag=%x\r\n",p_ble_evt->evt.gap_evt.params.rssi_changed.rssi,Get_lastDisplayTag());
#endif				
					if((check_lastDisplayTag(DISP_TAG_FIND_AG)==0)&&(disp_str_rssi.LCD_disp_Rssi_flag==1))
					{
				   /*// display_Rssi_distance(p_ble_evt->evt.gap_evt.params.rssi_changed.rssi,LCD_RSSI_GET_SUCCESS);
						//if((Protocol_check_BT_connected()==1))
						//start_Rssi();
						*/
						disp_str_rssi.LCD_disp_rssi_value=p_ble_evt->evt.gap_evt.params.rssi_changed.rssi;
					 disp_str_rssi.LCD_disp_Rssi_data_ready=1;
					}
					else
					{
             //sd_ble_gap_rssi_stop(m_conn_handle);
						 Get_Rssi_stop();
          }
				break;
        default:
            // No implementation needed.
            break;
    }
}


/**@brief Function for dispatching a S110 SoftDevice event to all modules with a S110 SoftDevice 
 *        event handler.
 *
 * @details This function is called from the S110 SoftDevice event interrupt handler after a S110 
 *          SoftDevice event has been received.
 *
 * @param[in] p_ble_evt  S110 SoftDevice event.
 */
static void ble_evt_dispatch(ble_evt_t * p_ble_evt)
{ 
#ifdef BLE_DFU_APP_SUPPORT
    dm_ble_evt_handler(p_ble_evt);
#endif
    ble_conn_params_on_ble_evt(p_ble_evt);
    bsp_btn_ble_on_ble_evt(p_ble_evt);
    ble_nus_on_ble_evt(&m_nus, p_ble_evt);
#if ENABLE_NEW_SERVECE
    new_service_nus_on_ble_evt(&new_m_nus, p_ble_evt);
#endif
#ifdef BLE_DFU_APP_SUPPORT
    /** @snippet [Propagating BLE Stack events to DFU Service] */
    ble_dfu_on_ble_evt(&m_dfus, p_ble_evt);
    /** @snippet [Propagating BLE Stack events to DFU Service] */
#endif // BLE_DFU_APP_SUPPORT
    on_ble_evt(p_ble_evt);
    ble_advertising_on_ble_evt(p_ble_evt);
    //bsp_btn_ble_on_ble_evt(p_ble_evt);
    
}

/**@brief Function for dispatching a system event to interested modules.
 *
 * @details This function is called from the System event interrupt handler after a system
 *          event has been received.
 *
 * @param[in] sys_evt  System stack event.
 */
#ifdef BLE_DFU_APP_SUPPORT
static void sys_evt_dispatch(uint32_t sys_evt)
{
    pstorage_sys_event_handler(sys_evt);
    ble_advertising_on_sys_evt(sys_evt);
}
#endif
/**@brief Function for the S110 SoftDevice initialization.
 *
 * @details This function initializes the S110 SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void)
{
    uint32_t err_code;
    
    // Initialize SoftDevice.
    SOFTDEVICE_HANDLER_INIT(NRF_CLOCK_LFCLKSRC_XTAL_20_PPM, NULL);

    // Enable BLE stack.
    ble_enable_params_t ble_enable_params;
    memset(&ble_enable_params, 0, sizeof(ble_enable_params));
#ifdef S130
    ble_enable_params.gatts_enable_params.attr_tab_size   = BLE_GATTS_ATTR_TAB_SIZE_DEFAULT;
#endif
    ble_enable_params.gatts_enable_params.service_changed = IS_SRVC_CHANGED_CHARACT_PRESENT;
    err_code = sd_ble_enable(&ble_enable_params);
    APP_ERROR_CHECK(err_code);
    
    // Subscribe for BLE events.
    err_code = softdevice_ble_evt_handler_set(ble_evt_dispatch);
    APP_ERROR_CHECK(err_code);
  
#ifdef BLE_DFU_APP_SUPPORT  
    // Register with the SoftDevice handler module for BLE events.
    err_code = softdevice_sys_evt_handler_set(sys_evt_dispatch);
    APP_ERROR_CHECK(err_code);
#endif
}


/**@brief Function for handling events from the BSP module.
 *
 * @param[in]   event   Event generated by button press.
 */
void bsp_event_handler(bsp_event_t event)
{
    uint32_t err_code;
    switch (event)
    {
        case BSP_EVENT_SLEEP:
            //sleep_mode_enter();//kevin delete
            break;

        case BSP_EVENT_DISCONNECT:
            err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            if (err_code != NRF_ERROR_INVALID_STATE)
            {
                APP_ERROR_CHECK(err_code);
            }
            break;

        case BSP_EVENT_WHITELIST_OFF:
            err_code = ble_advertising_restart_without_whitelist();
            if (err_code != NRF_ERROR_INVALID_STATE)
            {
                APP_ERROR_CHECK(err_code);
            }
            break;
        case BSP_EVENT_KEY_0:
						write_switch_val(CANCEL_KEY);//OK_KEY
						break;
				case BSP_EVENT_KEY_1:
						write_switch_val(OK_KEY);//CANCEL_KEY
						break;
				case BSP_EVENT_KEY_2:
						write_switch_val(DOWN_KEY);//UP_KEY
						break;
				case BSP_EVENT_KEY_3:
						write_switch_val(UP_KEY);//DOWN_KEY
						break;		
        default:
            break;
    }
}


/**@brief   Function for handling app_uart events.
 *
 * @details This function will receive a single character from the app_uart module and append it to 
 *          a string. The string will be be sent over BLE when the last character received was a 
 *          'new line' i.e '\n' (hex 0x0D) or if the string has reached a length of 
 *          @ref NUS_MAX_DATA_LENGTH.
 */
/**@snippet [Handling the data received over UART] */
#ifndef ENABLE_SIMPLE_UART //kevin add
void uart_event_handle(app_uart_evt_t * p_event)
{
    static uint8_t data_array[BLE_NUS_MAX_DATA_LEN];
    static uint8_t index = 0;
//    uint32_t       err_code;

    switch (p_event->evt_type)
    {
        case APP_UART_DATA_READY:
            UNUSED_VARIABLE(app_uart_get(&data_array[index]));
				    //DbgTOZ8(data_array,1);
				    while(app_uart_put(data_array[index]) != NRF_SUCCESS);
            /*index++;

            if ((data_array[index - 1] == '\n') || (index >= (BLE_NUS_MAX_DATA_LEN)))
            {
                err_code = ble_nus_string_send(&m_nus, data_array, index);
                if (err_code != NRF_ERROR_INVALID_STATE)
                {
                    APP_ERROR_CHECK(err_code);
                }
								//while(app_uart_put(p_data[i]) != NRF_SUCCESS);
                
                index = 0;
            }*/
            break;

        case APP_UART_COMMUNICATION_ERROR:
            APP_ERROR_HANDLER(p_event->data.error_communication);
            break;

        case APP_UART_FIFO_ERROR:
            APP_ERROR_HANDLER(p_event->data.error_code);
            break;

        default:
            break;
    }
}
#endif
/**@snippet [Handling the data received over UART] */

/**@brief  Function for initializing the UART module.
 */

/**@snippet [UART Initialization] */
#ifndef ENABLE_SIMPLE_UART //kevin add
static void uart_init(void)
{

    uint32_t                     err_code;
    const app_uart_comm_params_t comm_params =
    {
          RX_PIN_NUMBER,
          TX_PIN_NUMBER,
          RTS_PIN_NUMBER,
          CTS_PIN_NUMBER,
          APP_UART_FLOW_CONTROL_DISABLED,//APP_UART_FLOW_CONTROL_ENABLED,
          false,
          UART_BAUDRATE_BAUDRATE_Baud115200
    };
    APP_UART_FIFO_INIT( &comm_params,
                       UART_RX_BUF_SIZE,
                       UART_TX_BUF_SIZE,
                       uart_event_handle,
                       APP_IRQ_PRIORITY_LOW,
                       err_code);
    APP_ERROR_CHECK(err_code);
}
#endif


/**@brief Function for handling the Device Manager events.
 *
 * @param[in] p_evt  Data associated to the device manager event.
 */
#ifdef BLE_DFU_APP_SUPPORT
static uint32_t device_manager_evt_handler(dm_handle_t const * p_handle,
                                           dm_event_t const  * p_event,
                                           ret_code_t        event_result)
{
    APP_ERROR_CHECK(event_result);

#ifdef BLE_DFU_APP_SUPPORT
    if (p_event->event_id == DM_EVT_LINK_SECURED)
    {
        app_context_load(p_handle);
    }
#endif // BLE_DFU_APP_SUPPORT

    return NRF_SUCCESS;
}
#endif

/**@brief Function for the Device Manager initialization.
 *
 * @param[in] erase_bonds  Indicates whether bonding information should be cleared from
 *                         persistent storage during initialization of the Device Manager.
 */
#ifdef BLE_DFU_APP_SUPPORT
static void device_manager_init(bool erase_bonds)
{
    uint32_t               err_code;
    dm_init_param_t        init_param = {.clear_persistent_data = erase_bonds};
    dm_application_param_t register_param;

    // Initialize persistent storage module.
    err_code = pstorage_init();
    APP_ERROR_CHECK(err_code);

    err_code = dm_init(&init_param);
    APP_ERROR_CHECK(err_code);

    memset(&register_param.sec_param, 0, sizeof(ble_gap_sec_params_t));

    register_param.sec_param.bond         = SEC_PARAM_BOND;//kevin delete 151227
    //register_param.sec_param.bond         = 0;//kevin add 151227
    register_param.sec_param.mitm         = SEC_PARAM_MITM;
    register_param.sec_param.io_caps      = SEC_PARAM_IO_CAPABILITIES;
    register_param.sec_param.oob          = SEC_PARAM_OOB;
    register_param.sec_param.min_key_size = SEC_PARAM_MIN_KEY_SIZE;
    register_param.sec_param.max_key_size = SEC_PARAM_MAX_KEY_SIZE;
    register_param.evt_handler            = device_manager_evt_handler;
    register_param.service_type           = DM_PROTOCOL_CNTXT_GATT_SRVR_ID;

    err_code = dm_register(&m_app_handle, &register_param);
    APP_ERROR_CHECK(err_code);
}
#endif

/**@brief Function for initializing the Advertising functionality.
 */
static void advertising_init(void)
{
    uint32_t      err_code;
    ble_advdata_t advdata;
    //ble_advdata_t scanrsp;//kevin delete

    // Build advertising data struct to pass into @ref ble_advertising_init.
    memset(&advdata, 0, sizeof(advdata));
    advdata.name_type          = BLE_ADVDATA_FULL_NAME;
    advdata.include_appearance = true;//false;
    advdata.flags              = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;//BLE_GAP_ADV_FLAGS_LE_ONLY_LIMITED_DISC_MODE;

    //memset(&scanrsp, 0, sizeof(scanrsp));
    advdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);//scanrsp.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    advdata.uuids_complete.p_uuids  = m_adv_uuids;//scanrsp.uuids_complete.p_uuids  = m_adv_uuids;

    ble_adv_modes_config_t options = {0};
    options.ble_adv_fast_enabled  = BLE_ADV_FAST_ENABLED;
    options.ble_adv_fast_interval = APP_ADV_FAST_INTERVAL;
    options.ble_adv_fast_timeout  = APP_ADV_FAST_TIMEOUT_IN_SECONDS;
    
    options.ble_adv_slow_enabled  = BLE_ADV_SLOW_ENABLED;
    options.ble_adv_slow_interval = APP_ADV_SLOW_INTERVAL;
    options.ble_adv_slow_timeout  = APP_ADV_SLOW_TIMEOUT_IN_SECONDS;

    err_code = ble_advertising_init(&advdata, NULL, &options, on_adv_evt, NULL);//err_code = ble_advertising_init(&advdata, &scanrsp, &options, on_adv_evt, NULL);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing buttons and leds.
 *
 * @param[out] p_erase_bonds  Will be true if the clear bonding button was pressed to wake the application up.
 */
static void buttons_leds_init(bool * p_erase_bonds)
{
    bsp_event_t startup_event;

    uint32_t err_code = bsp_init(BSP_INIT_LED | BSP_INIT_BUTTONS,
                                 APP_TIMER_TICKS(100, APP_TIMER_PRESCALER), 
                                 bsp_event_handler);
    APP_ERROR_CHECK(err_code);

    err_code = bsp_btn_ble_init(NULL, &startup_event);
    APP_ERROR_CHECK(err_code);

    *p_erase_bonds = (startup_event == BSP_EVENT_CLEAR_BONDING_DATA);
}


/**@brief Function for placing the application in low power state while waiting for events.
 */
static void power_manage(void)
{
    uint32_t err_code = sd_app_evt_wait();
    APP_ERROR_CHECK(err_code);
}


/**@brief Application main function.
 */
/**@brief Function for initializing bsp module.
 */
/*
void bsp_configuration()
{
   // uint32_t err_code = NRF_SUCCESS;

   // NRF_CLOCK->LFCLKSRC            = (CLOCK_LFCLKSRC_SRC_Xtal << CLOCK_LFCLKSRC_SRC_Pos);
    NRF_CLOCK->LFCLKSRC            = (CLOCK_LFCLKSRC_SRC_RC << CLOCK_LFCLKSRC_SRC_Pos);
    NRF_CLOCK->EVENTS_LFCLKSTARTED = 0;
    NRF_CLOCK->TASKS_LFCLKSTART    = 1;

    while (NRF_CLOCK->EVENTS_LFCLKSTARTED == 0)
    {
        // Do nothing.
    }
}*/
//static void BT_set_address(UINT8 *BD_ADDR)
static void BT_set_address(void)
{
    uint32_t err_code;

 	  ble_gap_addr_t  t_addr={0,{0x02,0x00,0x00,0x02,0xff,0xaa}};

#if ENABLE_GET_FLASH_ID
    unsigned char keyid[32];
    etSpim1ReadNorFlashStdMode(KEYID_START_ADDR,keyid,31);
#if DEBUG_UART_EN    
   /* DbgPrintf("flash address data\r\n");
    DbgPrintf("%x %x %x %x %x %x %x %x\r\n",keyid[TAG1_ADDR_OFFSET],
    keyid[TAG2_ADDR_OFFSET],
    keyid[MAC_ADDRESS_ADDR_OFFSET],
    keyid[MAC_ADDRESS_ADDR_OFFSET+1],
    keyid[MAC_ADDRESS_ADDR_OFFSET+2],
    keyid[MAC_ADDRESS_ADDR_OFFSET+3],
    keyid[MAC_ADDRESS_ADDR_OFFSET+4],
    keyid[MAC_ADDRESS_ADDR_OFFSET+5]
    );*/
#endif
		if((keyid[TAG1_ADDR_OFFSET]==MAC_VALID_TAG1) && (keyid[TAG2_ADDR_OFFSET]==MAC_VALID_TAG2))
		{
      if((keyid[MAC_ADDRESS_ADDR_OFFSET]==0x00) 
      && (keyid[MAC_ADDRESS_ADDR_OFFSET+1]==0x00)
      && (keyid[MAC_ADDRESS_ADDR_OFFSET+2]==0x00)
      && (keyid[MAC_ADDRESS_ADDR_OFFSET+3]==0x00)
      && (keyid[MAC_ADDRESS_ADDR_OFFSET+4]==0x00)
      && (keyid[MAC_ADDRESS_ADDR_OFFSET+5]==0x00))
      {
        //return;
        //memcpy(t_addr.addr,BD_ADDR,6);
      }
      else if((keyid[MAC_ADDRESS_ADDR_OFFSET]==0xff) 
      && (keyid[MAC_ADDRESS_ADDR_OFFSET+1]==0xff)
      && (keyid[MAC_ADDRESS_ADDR_OFFSET+2]==0xff)
      && (keyid[MAC_ADDRESS_ADDR_OFFSET+3]==0xff)
      && (keyid[MAC_ADDRESS_ADDR_OFFSET+4]==0xff)
      && (keyid[MAC_ADDRESS_ADDR_OFFSET+5]==0xff))
      {
        //return;
       // memcpy(t_addr.addr,BD_ADDR,6);
      }
      else 
      {        
        memcpy(t_addr.addr,keyid+MAC_ADDRESS_ADDR_OFFSET,MAC_ADDRESS_LENGTH);
      }
		}
    else
#endif
    {
      //memcpy(t_addr.addr,BD_ADDR,6);
    }
   
    err_code = sd_ble_gap_address_set(BLE_GAP_ADDR_CYCLE_MODE_NONE, &t_addr);
		APP_ERROR_CHECK(err_code);

}
#if TAP_DOUBLE_CLCIK_ENABLE
unsigned int test_click_count =0;
extern void test_display_step_num(UINT32 temp_step_num);
#endif
int main(void)
{
    uint32_t err_code;
    bool erase_bonds;
   // uint8_t  start_string[] = START_STRING;
	  //const ble_gap_addr_t  t_addr={0,{0xaa,0x55,0x33,0x44,0x55,0x66}};
//    const char  temp_str[]={0xaa,0x5c,0x00,0x02,0x00,0x00,0xa1,0x55};
    // Initialize.
    //bsp_configuration();
    APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_MAX_TIMERS, APP_TIMER_OP_QUEUE_SIZE, false);    
    buttons_leds_init(&erase_bonds);
#if ENABLE_COS
    simple_uart_init(UART_POWEROFF_BT_DELAY);
#else 
    uart_init_lowPower();
#endif

		etSpim1NorFlashInit();
#if DEBUG_UART_EN    
   DbgPrintf("\r\n===flash_init===\r\n");
#endif
    ble_stack_init();
    
#ifdef BLE_DFU_APP_SUPPORT 
    device_manager_init(erase_bonds);
#endif
    
    //BT_set_address((UINT8 *)DEVICE_MAC_ADDR);
    BT_set_address();
    gap_params_init();   
    advertising_init();    
    services_init();
    //advertising_init();
    conn_params_init();  
#if ENABLE_COS   
    SetToGetKEYID();
#endif
    dev_init();		
	  
    //err_code = ble_advertising_start(BLE_ADV_MODE_SLOW);
#if DEBUG_UART_EN    
//    DbgPrintf("go to advertising7\r\n");
#endif     
	 err_code = ble_advertising_start(BLE_ADV_MODE_FAST);
   APP_ERROR_CHECK(err_code);
    
    // Enter main loop.
    for (;;)
    {
		  main_loop();		  
		  power_manage();
		  #if TAP_DOUBLE_CLCIK_ENABLE
		  	test_display_step_num(test_click_count);
		  #endif
    }
}


/** 
 * @}
 */
