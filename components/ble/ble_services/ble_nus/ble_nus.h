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

/**@file
 *
 * @defgroup ble_sdk_srv_nus Nordic UART Service
 * @{
 * @ingroup  ble_sdk_srv
 * @brief    Nordic UART Service implementation.
 *
 * @details The Nordic UART Service is a simple GATT-based service with TX and RX characteristics.
 *          Data received from the peer is passed to the application, and the data received
 *          from the application of this service is sent to the peer as Handle Value
 *          Notifications. This module demonstrates how to implement a custom GATT-based
 *          service and characteristics using the S110 SoftDevice. The service
 *          is used by the application to send and receive ASCII text strings to and from the
 *          peer.
 *
 * @note The application must propagate S110 SoftDevice events to the Nordic UART Service module
 *       by calling the ble_nus_on_ble_evt() function from the ble_stack_handler callback.
 */

#ifndef BLE_NUS_H__
#define BLE_NUS_H__

#include "ble.h"
#include "ble_srv_common.h"
#include <stdint.h>
#include <stdbool.h>

#define ENABLE_OLD_UUID 1
#define ENABLE_NEW_SERVECE 1

#if(ENABLE_OLD_UUID)
#define BLE_UUID_NUS_SERVICE 0x4243//                     /**< The UUID of the Nordic UART Service. */
#define ADV_UUID 0xfffe
#if ENABLE_NEW_SERVECE
#define NEW_SERVECE_NUS_SERVICE 0xfe01
#endif
#else
#define BLE_UUID_NUS_SERVICE 0xff01//                     /**< The UUID of the Nordic UART Service. */
#define ADV_UUID 0xff01//0xfffe
#endif

#define BLE_NUS_MAX_DATA_LEN (GATT_MTU_SIZE_DEFAULT - 3) /**< Maximum length of data (in bytes) that can be transmitted to the peer by the Nordic UART service module. */

/* Forward declaration of the ble_nus_t type. */
typedef struct ble_nus_s ble_nus_t;

#if ENABLE_NEW_SERVECE
typedef struct ble_nus_s_new_server new_server_t;
#endif
/**@brief Nordic UART Service event handler type. */
typedef void (*ble_nus_data_handler_t) (ble_nus_t * p_nus, uint8_t * p_data, uint16_t length);
#if ENABLE_NEW_SERVECE
typedef void (*new_service_nus_data_handler_t) (new_server_t * p_nus, uint8_t * p_data, uint16_t length);
#endif
/**@brief Nordic UART Service initialization structure.
 *
 * @details This structure contains the initialization information for the service. The application
 * must fill this structure and pass it to the service using the @ref ble_nus_init
 *          function.
 */
typedef struct
{
    ble_nus_data_handler_t data_handler; /**< Event handler to be called for handling received data. */
} ble_nus_init_t;
#if ENABLE_NEW_SERVECE
typedef struct
{
    new_service_nus_data_handler_t data_handler; /**< Event handler to be called for handling received data. */
} new_service_nus_init_t;
#endif
/**@brief Nordic UART Service structure.
 *
 * @details This structure contains status information related to the service.
 */
struct ble_nus_s
{
    uint8_t                  uuid_type;               /**< UUID type for Nordic UART Service Base UUID. */
    uint16_t                 service_handle;          /**< Handle of Nordic UART Service (as provided by the S110 SoftDevice). */
    ble_gatts_char_handles_t tx_handles;              /**< Handles related to the TX characteristic (as provided by the S110 SoftDevice). */
    ble_gatts_char_handles_t rx_handles;              /**< Handles related to the RX characteristic (as provided by the S110 SoftDevice). */
#if(ENABLE_OLD_UUID==0)
   // ble_gatts_char_handles_t tx2_handles;              /**< Handles related to the TX characteristic (as provided by the S110 SoftDevice). */
   // ble_gatts_char_handles_t rx2_handles;              /**< Handles related to the RX characteristic (as provided by the S110 SoftDevice). */ 
#endif  
    uint16_t                 conn_handle;             /**< Handle of the current connection (as provided by the S110 SoftDevice). BLE_CONN_HANDLE_INVALID if not in a connection. */
    bool                     is_notification_enabled; /**< Variable to indicate if the peer has enabled notification of the RX characteristic.*/
#if(ENABLE_OLD_UUID==0) 
   // bool                     is_notification2_enabled; /**< Variable to indicate if the peer has enabled notification of the RX characteristic.*/ 
#endif
    ble_nus_data_handler_t   data_handler;            /**< Event handler to be called for handling received data. */
};
#if ENABLE_NEW_SERVECE
struct ble_nus_s_new_server
{
    uint8_t                  uuid_type;               /**< UUID type for Nordic UART Service Base UUID. */
    uint16_t                 service_handle;          /**< Handle of Nordic UART Service (as provided by the S110 SoftDevice). */
    ble_gatts_char_handles_t tx_handles;              /**< Handles related to the TX characteristic (as provided by the S110 SoftDevice). */
    ble_gatts_char_handles_t rx_handles;              /**< Handles related to the RX characteristic (as provided by the S110 SoftDevice). */ 
    uint16_t                 conn_handle;             /**< Handle of the current connection (as provided by the S110 SoftDevice). BLE_CONN_HANDLE_INVALID if not in a connection. */
    bool                     is_notification_enabled; /**< Variable to indicate if the peer has enabled notification of the RX characteristic.*/
    new_service_nus_data_handler_t   data_handler;            /**< Event handler to be called for handling received data. */
};
#endif
/**@brief Function for initializing the Nordic UART Service.
 *
 * @param[out] p_nus      Nordic UART Service structure. This structure must be supplied
 *                        by the application. It is initialized by this function and will
 *                        later be used to identify this particular service instance.
 * @param[in] p_nus_init  Information needed to initialize the service.
 *
 * @retval NRF_SUCCESS If the service was successfully initialized. Otherwise, an error code is returned.
 * @retval NRF_ERROR_NULL If either of the pointers p_nus or p_nus_init is NULL.
 */
uint32_t ble_nus_init(ble_nus_t * p_nus, const ble_nus_init_t * p_nus_init);
#if ENABLE_NEW_SERVECE
uint32_t new_server_nus_init(new_server_t * p_nus, const new_service_nus_init_t * p_nus_init);
#endif
/**@brief Function for handling the Nordic UART Service's BLE events.
 *
 * @details The Nordic UART Service expects the application to call this function each time an
 * event is received from the S110 SoftDevice. This function processes the event if it
 * is relevant and calls the Nordic UART Service event handler of the
 * application if necessary.
 *
 * @param[in] p_nus       Nordic UART Service structure.
 * @param[in] p_ble_evt   Event received from the S110 SoftDevice.
 */
void ble_nus_on_ble_evt(ble_nus_t * p_nus, ble_evt_t * p_ble_evt);
#if ENABLE_NEW_SERVECE
void new_service_nus_on_ble_evt(new_server_t * p_nus, ble_evt_t * p_ble_evt);
#endif
/**@brief Function for sending a string to the peer.
 *
 * @details This function sends the input string as an RX characteristic notification to the
 *          peer.
 *
 * @param[in] p_nus       Pointer to the Nordic UART Service structure.
 * @param[in] p_string    String to be sent.
 * @param[in] length      Length of the string.
 *
 * @retval NRF_SUCCESS If the string was sent successfully. Otherwise, an error code is returned.
 */
uint32_t ble_nus_string_send(ble_nus_t * p_nus, uint8_t * p_string, uint16_t length);
#if ENABLE_NEW_SERVECE
uint32_t new_service_ble_nus_string_send(new_server_t * p_nus, uint8_t * p_string, uint16_t length);
#endif
/****
kevin define for etProtocol Use 150925
*****/
//void bt_send_to_AG(uint8_t* buff,uint16_t temp_length);
#endif // BLE_NUS_H__

/** @} */
