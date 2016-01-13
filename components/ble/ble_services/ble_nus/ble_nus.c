/* Copyright (c) 2012 Nordic Semiconductor. All Rights Reserved.
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

#include "ble_nus.h"
#include <string.h>
#include "nordic_common.h"
#include "ble_srv_common.h"
#include "btprotocol.h" //kevin add
#include "et_debug.h"

#define ENABLE_BLE_DEBUG 1

#if(ENABLE_OLD_UUID)
#define BLE_UUID_NUS_TX_CHARACTERISTIC 0x4243//                     /**< The UUID of the TX Characteristic. */
#define BLE_UUID_NUS_RX_CHARACTERISTIC 0x4243//                     /**< The UUID of the RX Characteristic. */
#define NUS_BASE_UUID                 {{0x73,0xa2,0x08,0x47,0x48,0x39,0xe0,0xff,0x3e,0x44,0x01,0xff,0x43,0x42,0x53,0x55}};// /**< Used vendor specific UUID. */
  #if(ENABLE_NEW_SERVECE)
  #define NEW_SERVECE_NUS_TX_CHARACTERISTIC 0xfe02//                     /**< The UUID of the TX Characteristic. */
  #define NEW_SERVECE_NUS_RX_CHARACTERISTIC 0xfe03//                     /**< The UUID of the RX Characteristic. */
  #define NEW_SERVECE_NUS_BASE_UUID                 {{0x73,0xa2,0x08,0x47,0x48,0x39,0xe0,0xff,0x3e,0x44,0x55,0xaa,0x00,0x00,0x53,0x55}};// /**< Used vendor specific UUID. */
  #endif
#else
#define BLE_UUID_NUS_TX_CHARACTERISTIC 0xff02//0x4243//                     /**< The UUID of the TX Characteristic. */
#define BLE_UUID_NUS_RX_CHARACTERISTIC 0xff03//0x4243//                     /**< The UUID of the RX Characteristic. */
//#define BLE_UUID_NUS_TX2_CHARACTERISTIC 0xff04//0x4243//                     /**< The UUID of the TX Characteristic. */
//#define BLE_UUID_NUS_RX2_CHARACTERISTIC 0xff05//0x4243//                     /**< The UUID of the RX Characteristic. */
#define NUS_BASE_UUID                 {{0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0, 0x93, 0xF3, 0xA3, 0xB5, 0x00, 0x00, 0x40, 0x6E}}//{{0x73,0xa2,0x08,0x47,0x48,0x39,0xe0,0xff,0x3e,0x44,0x01,0xff,0x43,0x42,0x53,0x55}};// /**< Used vendor specific UUID. */

#endif

#define BLE_NUS_MAX_RX_CHAR_LEN        BLE_NUS_MAX_DATA_LEN        /**< Maximum length of the RX Characteristic (in bytes). */
#define BLE_NUS_MAX_TX_CHAR_LEN        BLE_NUS_MAX_DATA_LEN        /**< Maximum length of the TX Characteristic (in bytes). */

//#define NUS_BASE_UUID                  {{0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0, 0x93, 0xF3, 0xA3, 0xB5, 0x00, 0x00, 0x40, 0x6E}} /**< Used vendor specific UUID. */
#if(ENABLE_OLD_UUID==0)
//uint32_t ble_nus_string_send2(ble_nus_t * p_nus, uint8_t * p_string, uint16_t length);
#endif
/**@brief Function for handling the @ref BLE_GAP_EVT_CONNECTED event from the S110 SoftDevice.
 *
 * @param[in] p_nus     Nordic UART Service structure.
 * @param[in] p_ble_evt Pointer to the event received from BLE stack.
 */
static void on_connect(ble_nus_t * p_nus, ble_evt_t * p_ble_evt)
{
    p_nus->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
}


/**@brief Function for handling the @ref BLE_GAP_EVT_DISCONNECTED event from the S110 SoftDevice.
 *
 * @param[in] p_nus     Nordic UART Service structure.
 * @param[in] p_ble_evt Pointer to the event received from BLE stack.
 */
static void on_disconnect(ble_nus_t * p_nus, ble_evt_t * p_ble_evt)
{
    UNUSED_PARAMETER(p_ble_evt);
    p_nus->conn_handle = BLE_CONN_HANDLE_INVALID;
}
/**@brief Function for handling the @ref BLE_GAP_EVT_CONNECTED event from the S110 SoftDevice.
 *
 * @param[in] p_nus     Nordic UART Service structure.
 * @param[in] p_ble_evt Pointer to the event received from BLE stack.
 */
static void on_connect2(new_server_t * p_nus, ble_evt_t * p_ble_evt)
{
    p_nus->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
}


/**@brief Function for handling the @ref BLE_GAP_EVT_DISCONNECTED event from the S110 SoftDevice.
 *
 * @param[in] p_nus     Nordic UART Service structure.
 * @param[in] p_ble_evt Pointer to the event received from BLE stack.
 */
static void on_disconnect2(new_server_t * p_nus, ble_evt_t * p_ble_evt)
{
    UNUSED_PARAMETER(p_ble_evt);
    p_nus->conn_handle = BLE_CONN_HANDLE_INVALID;
}

/**@brief Function for handling the @ref BLE_GATTS_EVT_WRITE event from the S110 SoftDevice.
 *
 * @param[in] p_nus     Nordic UART Service structure.
 * @param[in] p_ble_evt Pointer to the event received from BLE stack.
 */
static void on_write(ble_nus_t * p_nus, ble_evt_t * p_ble_evt)
{

    ble_gatts_evt_write_t * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;
#if ((DEBUG_UART_EN) && (ENABLE_BLE_DEBUG))
		//	DbgPrintf("write->handle=%x\r\n",p_evt_write->handle);
#endif
    if((p_evt_write->handle == p_nus->rx_handles.cccd_handle)&&(p_evt_write->len == 2))      
    {
        if (ble_srv_is_notification_enabled(p_evt_write->data))
        {
            p_nus->is_notification_enabled = true;
          //Protocol_set_notify(1);
#if ((DEBUG_UART_EN) && (ENABLE_BLE_DEBUG))
			DbgPrintf("set notification1\r\n");
#endif           
        }
        else
        {
#if ((DEBUG_UART_EN) && (ENABLE_BLE_DEBUG))
			DbgPrintf("clear notification1\r\n");
#endif           
            p_nus->is_notification_enabled = false;
          //Protocol_set_notify(0);
        }
    }
#if(ENABLE_OLD_UUID==0)
  /*  else if((p_evt_write->handle == p_nus->rx2_handles.cccd_handle)&&(p_evt_write->len == 2))
    {
        if (ble_srv_is_notification_enabled(p_evt_write->data))
        {
            p_nus->is_notification2_enabled = true;           
        }
        else
        {         
          p_nus->is_notification2_enabled = false;
        }
    } */ 
#endif    
    else if(
             (p_evt_write->handle == p_nus->tx_handles.value_handle)
             &&
             (p_nus->data_handler != NULL)
            )
    {
#if ((DEBUG_UART_EN) && (ENABLE_BLE_DEBUG))
     DbgPrintfbuff("_write1:",strlen("_write1:"));
      for(uint16_t i=0;i<p_evt_write->len;i++)
      DbgPrintf("%02x ",p_evt_write->data[i]);
     DbgPrintfbuff("\r\n",2);
#endif     
        p_nus->data_handler(p_nus, p_evt_write->data, p_evt_write->len);//kevin delete
			 //Protocol_data_manage(p_evt_write->data, p_evt_write->len);//kevin add
    }
#if(ENABLE_OLD_UUID==0)
   /* else if(
             (p_evt_write->handle == p_nus->tx2_handles.value_handle)
             &&
             (p_nus->data_handler != NULL)
            )
    {      
        //p_nus->data_handler(p_nus, p_evt_write->data, p_evt_write->len);//kevin delete
      //ble_nus_string_send2(p_nus, p_evt_write->data, p_evt_write->len);
      
    } */
#endif    
    else
    {
        // Do Nothing. This event is not relevant for this service.
    }
}
#if(ENABLE_NEW_SERVECE)
static void on_write2(new_server_t * p_nus, ble_evt_t * p_ble_evt)
{

    ble_gatts_evt_write_t * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;
#if ((DEBUG_UART_EN) && (ENABLE_BLE_DEBUG))
	//		DbgPrintf("write->handle=%x\r\n",p_evt_write->handle);
#endif
    if((p_evt_write->handle == p_nus->rx_handles.cccd_handle)&&(p_evt_write->len == 2))      
    {
        if (ble_srv_is_notification_enabled(p_evt_write->data))
        {
            p_nus->is_notification_enabled = true;
          Protocol_set_notify(1);
#if ((DEBUG_UART_EN) && (ENABLE_BLE_DEBUG))
			DbgPrintf("set notification3\r\n");
#endif           
        }
        else
        {
#if ((DEBUG_UART_EN) && (ENABLE_BLE_DEBUG))
			DbgPrintf("clear notification3\r\n");
#endif           
            p_nus->is_notification_enabled = false;
          Protocol_set_notify(0);
        }
    }    
    else if(
             (p_evt_write->handle == p_nus->tx_handles.value_handle)
             &&
             (p_nus->data_handler != NULL)
            )
    {
#if ((DEBUG_UART_EN) && (ENABLE_BLE_DEBUG))
     DbgPrintfbuff("_write3:",strlen("_write3:"));
      for(uint16_t i=0;i<p_evt_write->len;i++)
      DbgPrintf("%02x ",p_evt_write->data[i]);
     DbgPrintfbuff("\r\n",2);
#endif     
        p_nus->data_handler(p_nus, p_evt_write->data, p_evt_write->len);//kevin delete
    }
    else
    {
        // Do Nothing. This event is not relevant for this service.
    }
}
#endif
/**@brief Function for adding RX characteristic.
 *
 * @param[in] p_nus       Nordic UART Service structure.
 * @param[in] p_nus_init  Information needed to initialize the service.
 *
 * @return NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t rx_char_add(ble_nus_t * p_nus, const ble_nus_init_t * p_nus_init)
{
#if(ENABLE_OLD_UUID)
    ble_gatts_char_md_t char_md;
	  ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;
  
    uint32_t            err_code;
	
    //25ccb714-151b-4d8e-b328-38f3fb42dc2e
    ble_uuid128_t   nus_base_uuid = {{0x73,0xa2,0x08,0x47,0x48,0x39,0xe0,0xff,0x3e,0x44,0x03,0xff,0x43,0x42,0x53,0x55}};//0xB714
																			
    memset(&cccd_md, 0, sizeof(cccd_md));
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);   
		cccd_md.vloc = BLE_GATTS_VLOC_STACK;
																			
    memset(&char_md, 0, sizeof(char_md));
    
    //char_md.char_props.write            = 1;
		char_md.char_props.notify            =1;
    //char_md.char_props.write_wo_resp    = 0;
    char_md.p_char_user_desc            = NULL;
    char_md.p_char_pf                   = NULL;
    char_md.p_user_desc_md              = NULL;
    char_md.p_cccd_md                   = &cccd_md;
    char_md.p_sccd_md                   = NULL;
    
    // Add custom base UUID.
    err_code = sd_ble_uuid_vs_add(&nus_base_uuid, &ble_uuid.type);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    } 
    
    //ble_uuid.type                       = p_nus->uuid_type;
    ble_uuid.uuid                       = BLE_UUID_NUS_RX_CHARACTERISTIC;
    
    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
    
    attr_md.vloc                        = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth                     = 0;
    attr_md.wr_auth                     = 0;
    attr_md.vlen                        = 1;
    
    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid              = &ble_uuid;
    attr_char_value.p_attr_md           = &attr_md;
    attr_char_value.init_len            = sizeof(uint8_t);//1;
    attr_char_value.init_offs           = 0;
    attr_char_value.max_len             = BLE_NUS_MAX_RX_CHAR_LEN;
    
    return sd_ble_gatts_characteristic_add(p_nus->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_nus->rx_handles);
#if ((DEBUG_UART_EN) && (ENABLE_BLE_DEBUG))
      //DbgPrintf("rx_handles=%x\r\n",(unsigned int)p_nus->rx_handles.value_handle);
#endif                                           
#else	
    /**@snippet [Adding proprietary characteristic to S110 SoftDevice] */
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&cccd_md, 0, sizeof(cccd_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);

    cccd_md.vloc = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.notify = 1;
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = &cccd_md;
    char_md.p_sccd_md         = NULL;

    ble_uuid.type = p_nus->uuid_type;
    ble_uuid.uuid = BLE_UUID_NUS_RX_CHARACTERISTIC;

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);

    attr_md.vloc    = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth = 0;
    attr_md.wr_auth = 0;
    attr_md.vlen    = 1;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = sizeof(uint8_t);
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = BLE_NUS_MAX_RX_CHAR_LEN;

    return sd_ble_gatts_characteristic_add(p_nus->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_nus->rx_handles);
    /**@snippet [Adding proprietary characteristic to S110 SoftDevice] */
#endif
}
#if(ENABLE_OLD_UUID==0)
/*
static uint32_t rx2_char_add(ble_nus_t * p_nus, const ble_nus_init_t * p_nus_init)
{
    //@snippet [Adding proprietary characteristic to S110 SoftDevice]
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&cccd_md, 0, sizeof(cccd_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);

    cccd_md.vloc = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.notify = 1;
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = &cccd_md;
    char_md.p_sccd_md         = NULL;

    ble_uuid.type = p_nus->uuid_type;
    ble_uuid.uuid = BLE_UUID_NUS_RX2_CHARACTERISTIC;

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);

    attr_md.vloc    = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth = 0;
    attr_md.wr_auth = 0;
    attr_md.vlen    = 1;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = sizeof(uint8_t);
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = BLE_NUS_MAX_RX_CHAR_LEN;

    return sd_ble_gatts_characteristic_add(p_nus->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_nus->rx2_handles);
   //@snippet [Adding proprietary characteristic to S110 SoftDevice]

}*/
#endif
/**@brief Function for adding TX characteristic.
 *
 * @param[in] p_nus       Nordic UART Service structure.
 * @param[in] p_nus_init  Information needed to initialize the service.
 *
 * @return NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t tx_char_add(ble_nus_t * p_nus, const ble_nus_init_t * p_nus_init)
{
#if(ENABLE_OLD_UUID)
	ble_gatts_char_md_t char_md;
	ble_gatts_attr_t	attr_char_value;
	ble_uuid_t			ble_uuid;
	ble_gatts_attr_md_t attr_md;
  
	uint32_t			err_code;
	//25ccb714-151b-4d8e-b328-38f3fb42dc2e
	ble_uuid128_t	nus_base_uuid = {{0x73,0xa2,0x08,0x47,0x48,0x39,0xe0,0xff,0x3e,0x44,0x02,0xff,0x43,0x42,0x53,0x55}};//0xB714
									 
	memset(&char_md, 0, sizeof(char_md));
	
	char_md.char_props.write			= 1;
	char_md.char_props.write_wo_resp	= 1;
	char_md.p_char_user_desc			= NULL;
	char_md.p_char_pf					= NULL;
	char_md.p_user_desc_md				= NULL;
	char_md.p_cccd_md					= NULL;
	char_md.p_sccd_md					= NULL;
	
	// Add custom base UUID.
	err_code = sd_ble_uuid_vs_add(&nus_base_uuid, &ble_uuid.type);
	if (err_code != NRF_SUCCESS)
	{
		return err_code;
	} 
	
	//ble_uuid.type 					  = p_nus->uuid_type;
	ble_uuid.uuid						= BLE_UUID_NUS_TX_CHARACTERISTIC;
	
	memset(&attr_md, 0, sizeof(attr_md));

	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
	
	attr_md.vloc						= BLE_GATTS_VLOC_STACK;
	attr_md.rd_auth 					= 0;
	attr_md.wr_auth 					= 0;
	attr_md.vlen						= 1;
	
	memset(&attr_char_value, 0, sizeof(attr_char_value));

	attr_char_value.p_uuid				= &ble_uuid;
	attr_char_value.p_attr_md			= &attr_md;
	attr_char_value.init_len			= 1;
	attr_char_value.init_offs			= 0;
	attr_char_value.max_len 			= BLE_NUS_MAX_TX_CHAR_LEN;
	
	return sd_ble_gatts_characteristic_add(p_nus->service_handle,
										   &char_md,
										   &attr_char_value,
										   &p_nus->tx_handles);
#else
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.write         = 1;
    char_md.char_props.write_wo_resp = 1;
    char_md.p_char_user_desc         = NULL;
    char_md.p_char_pf                = NULL;
    char_md.p_user_desc_md           = NULL;
    char_md.p_cccd_md                = NULL;
    char_md.p_sccd_md                = NULL;

    ble_uuid.type = p_nus->uuid_type;
    ble_uuid.uuid = BLE_UUID_NUS_TX_CHARACTERISTIC;

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);

    attr_md.vloc    = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth = 0;
    attr_md.wr_auth = 0;
    attr_md.vlen    = 1;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = 1;
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = BLE_NUS_MAX_TX_CHAR_LEN;

    return sd_ble_gatts_characteristic_add(p_nus->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_nus->tx_handles);
 #if ((DEBUG_UART_EN) && (ENABLE_BLE_DEBUG))
      DbgPrintf("tx_handles=%x\r\n",p_nus->service_handle);
#endif                                          
#endif
}
#if(ENABLE_OLD_UUID==0)
/*
static uint32_t tx2_char_add(ble_nus_t * p_nus, const ble_nus_init_t * p_nus_init)
{
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.write         = 1;
    char_md.char_props.write_wo_resp = 1;
    char_md.p_char_user_desc         = NULL;
    char_md.p_char_pf                = NULL;
    char_md.p_user_desc_md           = NULL;
    char_md.p_cccd_md                = NULL;
    char_md.p_sccd_md                = NULL;

    ble_uuid.type = p_nus->uuid_type;
    ble_uuid.uuid = BLE_UUID_NUS_TX2_CHARACTERISTIC;

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);

    attr_md.vloc    = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth = 0;
    attr_md.wr_auth = 0;
    attr_md.vlen    = 1;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = 1;
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = BLE_NUS_MAX_TX_CHAR_LEN;

    return sd_ble_gatts_characteristic_add(p_nus->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_nus->tx2_handles);
}*/
#endif

void ble_nus_on_ble_evt(ble_nus_t * p_nus, ble_evt_t * p_ble_evt)
{
    if ((p_nus == NULL) || (p_ble_evt == NULL))
    {
        return;
    }
#if ((DEBUG_UART_EN) && (ENABLE_BLE_DEBUG))
			//DbgPrintf("id=%x\r\n",p_ble_evt->header.evt_id);
#endif 
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            on_connect(p_nus, p_ble_evt);
            
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            on_disconnect(p_nus, p_ble_evt);
            
            break;

        case BLE_GATTS_EVT_WRITE:
#if ((DEBUG_UART_EN) && (ENABLE_BLE_DEBUG))
			//DbgPrintf("common_evt.conn_handle=%x\r\n,gap_evt.conn_handle=%x\r\n,l2cap_evt.conn_handle=%x\r\n,gattc_evt.conn_handle=%x\r\n,gatts_evt.conn_handle=%x\r\n",
      //  p_ble_evt->evt.common_evt.conn_handle,
      //  p_ble_evt->evt.gap_evt.conn_handle,
      //  p_ble_evt->evt.l2cap_evt.conn_handle,
      //  p_ble_evt->evt.gattc_evt.conn_handle,
      //  p_ble_evt->evt.gatts_evt.conn_handle        
      //  );
#endif           
            on_write(p_nus, p_ble_evt);
            break;

        default:
            // No implementation needed.
            break;
    }
}
#if ENABLE_NEW_SERVECE
void new_service_nus_on_ble_evt(new_server_t * p_nus, ble_evt_t * p_ble_evt)
{
    if ((p_nus == NULL) || (p_ble_evt == NULL))
    {
        return;
    }
#if ((DEBUG_UART_EN) && (ENABLE_BLE_DEBUG))
			//DbgPrintf("id=%x\r\n",p_ble_evt->header.evt_id);
#endif 
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            on_connect2(p_nus, p_ble_evt);            
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            on_disconnect2(p_nus, p_ble_evt);            
            break;

        case BLE_GATTS_EVT_WRITE:          
            on_write2(p_nus, p_ble_evt);
            break;

        default:
            // No implementation needed.
            break;
    }
}
#endif
uint32_t ble_nus_init(ble_nus_t * p_nus, const ble_nus_init_t * p_nus_init)
{
    uint32_t      err_code;
    ble_uuid_t    ble_uuid;
    ble_uuid128_t nus_base_uuid = NUS_BASE_UUID;

    if ((p_nus == NULL) || (p_nus_init == NULL))
    {
        return NRF_ERROR_NULL;
    }

    // Initialize the service structure.
    p_nus->conn_handle             = BLE_CONN_HANDLE_INVALID;
    p_nus->data_handler            = p_nus_init->data_handler;
    p_nus->is_notification_enabled = false;

    /**@snippet [Adding proprietary Service to S110 SoftDevice] */
    // Add a custom base UUID.
    err_code = sd_ble_uuid_vs_add(&nus_base_uuid, &p_nus->uuid_type);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    ble_uuid.type = p_nus->uuid_type;
    ble_uuid.uuid = BLE_UUID_NUS_SERVICE;

    // Add the service.
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                        &ble_uuid,
                                        &p_nus->service_handle);
//#if ((DEBUG_UART_EN) && (ENABLE_BLE_DEBUG))
//      DbgPrintf("service1_handle=%x\r\n",p_nus->service_handle);
//#endif    
    /**@snippet [Adding proprietary Service to S110 SoftDevice] */
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    // Add the TX Characteristic.
    err_code = tx_char_add(p_nus, p_nus_init);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    // Add the RX Characteristic.
    err_code = rx_char_add(p_nus, p_nus_init);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
#if(ENABLE_OLD_UUID==0)   
    // Add the TX2 Characteristic.
  /*  err_code = tx2_char_add(p_nus, p_nus_init);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    // Add the RX2 Characteristic.
    err_code = rx2_char_add(p_nus, p_nus_init);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }*/
#endif

    return NRF_SUCCESS;
}
//===============================================================
/**@brief Function for adding TX characteristic.
 *
 * @param[in] p_nus       Nordic UART Service structure.
 * @param[in] p_nus_init  Information needed to initialize the service.
 *
 * @return NRF_SUCCESS on success, otherwise an error code.
 */
#if(ENABLE_NEW_SERVECE)
static uint32_t new_service_tx_char_add(new_server_t * p_nus, const new_service_nus_init_t * p_nus_init)
{

    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.write         = 1;
    char_md.char_props.write_wo_resp = 1;
    char_md.p_char_user_desc         = NULL;
    char_md.p_char_pf                = NULL;
    char_md.p_user_desc_md           = NULL;
    char_md.p_cccd_md                = NULL;
    char_md.p_sccd_md                = NULL;

    ble_uuid.type = p_nus->uuid_type;
    ble_uuid.uuid = NEW_SERVECE_NUS_TX_CHARACTERISTIC;

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);

    attr_md.vloc    = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth = 0;
    attr_md.wr_auth = 0;
    attr_md.vlen    = 1;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = 1;
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = BLE_NUS_MAX_TX_CHAR_LEN;

    return sd_ble_gatts_characteristic_add(p_nus->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_nus->tx_handles);
#if ((DEBUG_UART_EN) && (ENABLE_BLE_DEBUG))
//      DbgPrintf("new_tx_handles=%x\r\n",p_nus->tx_handles.value_handle);
#endif
}
/**@brief Function for adding RX characteristic.
 *
 * @param[in] p_nus       Nordic UART Service structure.
 * @param[in] p_nus_init  Information needed to initialize the service.
 *
 * @return NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t new_service_rx_char_add(new_server_t * p_nus, const new_service_nus_init_t * p_nus_init)
{
    /**@snippet [Adding proprietary characteristic to S110 SoftDevice] */
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&cccd_md, 0, sizeof(cccd_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);

    cccd_md.vloc = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.notify = 1;
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = &cccd_md;
    char_md.p_sccd_md         = NULL;

    ble_uuid.type = p_nus->uuid_type;
    ble_uuid.uuid = NEW_SERVECE_NUS_RX_CHARACTERISTIC;

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);

    attr_md.vloc    = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth = 0;
    attr_md.wr_auth = 0;
    attr_md.vlen    = 1;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = sizeof(uint8_t);
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = BLE_NUS_MAX_RX_CHAR_LEN;

    return sd_ble_gatts_characteristic_add(p_nus->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_nus->rx_handles);
#if ((DEBUG_UART_EN) && (ENABLE_BLE_DEBUG))
//      DbgPrintf("new_rx_handles=%x\r\n",p_nus->rx_handles.value_handle);
#endif                                           
}
uint32_t new_server_nus_init(new_server_t * p_nus, const new_service_nus_init_t * p_nus_init)
{
		 uint32_t 		 err_code;
			ble_uuid_t		ble_uuid;
			ble_uuid128_t nus_base_uuid = NEW_SERVECE_NUS_BASE_UUID;
	
			if ((p_nus == NULL) || (p_nus_init == NULL))
			{
					return NRF_ERROR_NULL;
			}
	
			// Initialize the service structure.
			p_nus->conn_handle						 = BLE_CONN_HANDLE_INVALID;
			p_nus->data_handler 					 = p_nus_init->data_handler;
			p_nus->is_notification_enabled = false;
	
			/**@snippet [Adding proprietary Service to S110 SoftDevice] */
			// Add a custom base UUID.
			err_code = sd_ble_uuid_vs_add(&nus_base_uuid, &p_nus->uuid_type);
			if (err_code != NRF_SUCCESS)
			{
					return err_code;
			}
	
			ble_uuid.type = p_nus->uuid_type;
			ble_uuid.uuid = NEW_SERVECE_NUS_SERVICE;
	
			// Add the service.
			err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
																					&ble_uuid,
																					&p_nus->service_handle);
//#if ((DEBUG_UART_EN) && (ENABLE_BLE_DEBUG))
//      DbgPrintf("service2_handle=%x\r\n",p_nus->service_handle);
//#endif     
			/**@snippet [Adding proprietary Service to S110 SoftDevice] */
			if (err_code != NRF_SUCCESS)
			{
					return err_code;
			}
			// Add the TX Characteristic.
			err_code = new_service_tx_char_add(p_nus, p_nus_init);
			if (err_code != NRF_SUCCESS)
			{
					return err_code;
			}
			// Add the RX Characteristic.
			err_code = new_service_rx_char_add(p_nus, p_nus_init);
			if (err_code != NRF_SUCCESS)
			{
					return err_code;
			}	
			return NRF_SUCCESS;

}
#endif
//================================================================

uint32_t ble_nus_string_send(ble_nus_t * p_nus, uint8_t * p_string, uint16_t length)
{
    ble_gatts_hvx_params_t hvx_params;

    if (p_nus == NULL)
    {
        return NRF_ERROR_NULL;
    }

    if ((p_nus->conn_handle == BLE_CONN_HANDLE_INVALID) || (!p_nus->is_notification_enabled))
    {
        return NRF_ERROR_INVALID_STATE;
    }

    if (length > BLE_NUS_MAX_DATA_LEN)
    {
        return NRF_ERROR_INVALID_PARAM;
    }
 #if ((DEBUG_UART_EN) && (ENABLE_BLE_DEBUG))
     DbgPrintfbuff("notify1:",strlen("notify1:"));
      for(uint16_t i=0;i<length;i++)
      DbgPrintf("%02x ",p_string[i]);
     DbgPrintfbuff("\r\n",2);
#endif    
    memset(&hvx_params, 0, sizeof(hvx_params));

    hvx_params.handle = p_nus->rx_handles.value_handle;
    hvx_params.p_data = p_string;
    hvx_params.p_len  = &length;
    hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
    return sd_ble_gatts_hvx(p_nus->conn_handle, &hvx_params);
}
//==================================================================
#if(ENABLE_OLD_UUID==0)
/*
uint32_t ble_nus_string_send2(ble_nus_t * p_nus, uint8_t * p_string, uint16_t length)
{
    ble_gatts_hvx_params_t hvx_params;

    if (p_nus == NULL)
    {
        return NRF_ERROR_NULL;
    }

    if ((p_nus->conn_handle == BLE_CONN_HANDLE_INVALID) || (!p_nus->is_notification2_enabled))
    {
        return NRF_ERROR_INVALID_STATE;
    }

    if (length > BLE_NUS_MAX_DATA_LEN)
    {
        return NRF_ERROR_INVALID_PARAM;
    }
//#if DEBUG_UART_EN
//     DbgPrintfbuff("notify 2:",strlen("notify 2:"));
//      for(uint16_t i=0;i<length;i++)
//      DbgPrintf("%x ",p_string[i]);
//     DbgPrintfbuff("\r\n",2);
//#endif 
    memset(&hvx_params, 0, sizeof(hvx_params));

    hvx_params.handle = p_nus->rx2_handles.value_handle;
    hvx_params.p_data = p_string;
    hvx_params.p_len  = &length;
    hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
    return sd_ble_gatts_hvx(p_nus->conn_handle, &hvx_params);
}*/
#endif
#if ENABLE_NEW_SERVECE
uint32_t new_service_ble_nus_string_send(new_server_t * p_nus, uint8_t * p_string, uint16_t length)
{
    ble_gatts_hvx_params_t hvx_params;

    if (p_nus == NULL)
    {
        return NRF_ERROR_NULL;
    }

    if ((p_nus->conn_handle == BLE_CONN_HANDLE_INVALID) || (!p_nus->is_notification_enabled))
    {
        return NRF_ERROR_INVALID_STATE;
    }

    if (length > BLE_NUS_MAX_DATA_LEN)
    {
        return NRF_ERROR_INVALID_PARAM;
    }
#if ((DEBUG_UART_EN) && (ENABLE_BLE_DEBUG))
     DbgPrintfbuff("notify3:",strlen("notify3:"));
      for(uint16_t i=0;i<length;i++)
      DbgPrintf("%02x ",p_string[i]);
     DbgPrintfbuff("\r\n",2);
#endif    
    memset(&hvx_params, 0, sizeof(hvx_params));

    hvx_params.handle = p_nus->rx_handles.value_handle;
    hvx_params.p_data = p_string;
    hvx_params.p_len  = &length;
    hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
    return sd_ble_gatts_hvx(p_nus->conn_handle, &hvx_params);
}
#endif
