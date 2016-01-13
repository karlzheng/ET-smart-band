/* Copyright (c) 2009 Nordic Semiconductor. All Rights Reserved.
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
#ifdef ENABLE_SIMPLE_UART
#include <stdint.h>

#include "nrf.h"
#include "etSimple_uart.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "ble_nus.h"
#include "ET1502B.h"
#include "uartProtocol.h"
#include "app_util_platform.h"
#define SIMPLE_UART_RX_BUF_SIZE                512
//static uint8_t simple_uart_rev[SIMPLE_UART_RX_BUF_SIZE];
//static uint16_t simple_uart_rev_index = 0;
//static uint16_t simple_uart_send_index = 0;
struct _uart_recv_buf
{
	volatile unsigned int rp;//read pointer
	volatile unsigned int wp;//write pointer
	volatile unsigned char dat[SIMPLE_UART_RX_BUF_SIZE];
};
static struct _uart_recv_buf UartRecvBuf={0, 0};

/*
uint8_t simple_uart_get(void)
{
    while (NRF_UART0->EVENTS_RXDRDY != 1)
    {
        // Wait for RXD data to be received
    }

    NRF_UART0->EVENTS_RXDRDY = 0;
    return (uint8_t)NRF_UART0->RXD;
}
*/
/*
bool simple_uart_get_with_timeout(int32_t timeout_ms, uint8_t * rx_data)
{
    bool ret = true;

    while (NRF_UART0->EVENTS_RXDRDY != 1)
    {
        if (timeout_ms-- >= 0)
        {
            // wait in 1ms chunk before checking for status.
            nrf_delay_us(1000);
        }
        else
        {
            ret = false;
            break;
        }
    } // Wait for RXD data to be received.

    if (timeout_ms >= 0)
    {
        // clear the event and set rx_data with received byte.
        NRF_UART0->EVENTS_RXDRDY = 0;
        *rx_data                 = (uint8_t)NRF_UART0->RXD;
    }

    return ret;
}
*/
static void simple_uart_tx_send(void)
{
    //uint8_t tx_byte;
    NRF_UART0->TASKS_STARTTX = 1;
    
}

static void simple_uart_tx_stop(void)
{
    // No more bytes in FIFO, terminate transmission.
    NRF_UART0->TASKS_STOPTX = 1;

}
void simple_uart_put(uint8_t cr)
{
    //while (NRF_UART0->EVENTS_TXDRDY ==0);
    NRF_UART0->TXD = (uint8_t)cr;

    while (NRF_UART0->EVENTS_TXDRDY != 1)
    {
        // Wait for TXD data to be sent.
    }

    NRF_UART0->EVENTS_TXDRDY = 0;
}


void simple_uart_putstring(const uint8_t * str)
{
    uint_fast8_t i  = 0;
    uint8_t      ch = str[i++];
    simple_uart_tx_send();
    while (ch != '\0')
    {
        simple_uart_put(ch);
        ch = str[i++];
    }
		simple_uart_tx_stop();
		
}
void simple_uart_putstringbuff(uint8_t * str,uint16_t len)
{
    uint16_t i  = 0;
   // uint8_t      ch = str[i++];
    simple_uart_tx_send();
    while (i<len)
    {
        simple_uart_put(str[i]);
        //ch = str[i++];
			  i++;
    }
		simple_uart_tx_stop();		
}
/*
void Uart_Send_String(uint8_t * str, unsigned short len)
{
	unsigned short i = 0;
	
	simple_uart_tx_send();
	for (i = 0; i < len; i++)
	{
		simple_uart_put(*str);
		if (i == (len - 1))
			break;
		str++;
	}
	simple_uart_tx_stop();
}
*/

void simple_uart_config(uint8_t rts_pin_number,
                        uint8_t txd_pin_number,
                        uint8_t cts_pin_number,
                        uint8_t rxd_pin_number,
                        bool    hwfc)
{
/** @snippet [Configure UART RX and TX pin] */
    nrf_gpio_cfg_output(txd_pin_number);
    nrf_gpio_cfg_input(rxd_pin_number, NRF_GPIO_PIN_NOPULL);

    NRF_UART0->PSELTXD = txd_pin_number;
    NRF_UART0->PSELRXD = rxd_pin_number;
/** @snippet [Configure UART RX and TX pin] */
    if (hwfc)
    {
        nrf_gpio_cfg_output(rts_pin_number);
        nrf_gpio_cfg_input(cts_pin_number, NRF_GPIO_PIN_NOPULL);
        NRF_UART0->PSELCTS = cts_pin_number;
        NRF_UART0->PSELRTS = rts_pin_number;
        NRF_UART0->CONFIG  = (UART_CONFIG_HWFC_Enabled << UART_CONFIG_HWFC_Pos);
    }

    NRF_UART0->BAUDRATE      = (UART_BAUDRATE_BAUDRATE_Baud115200 << UART_BAUDRATE_BAUDRATE_Pos);
    NRF_UART0->ENABLE        = (UART_ENABLE_ENABLE_Enabled << UART_ENABLE_ENABLE_Pos);
    NRF_UART0->TASKS_STARTTX = 1;
    NRF_UART0->TASKS_STARTRX = 1;
    NRF_UART0->EVENTS_RXDRDY = 0;
}
/**@brief  Function for initializing the UART module.
 */
unsigned char UartRecv(unsigned char *buf)
{
	if (UartRecvBuf.rp != UartRecvBuf.rp)
	{
		*buf = UartRecvBuf.dat[UartRecvBuf.rp];
		UartRecvBuf.rp++;
		if (UartRecvBuf.rp >= SIMPLE_UART_RX_BUF_SIZE)
			UartRecvBuf.rp = 0;

		return 1;
	}
	return 0;
}


void uart_data_check(void)
{
	if(UartRecvBuf.rp!=UartRecvBuf.wp)
	{
	simple_uart_tx_send();
	while(UartRecvBuf.rp!=UartRecvBuf.wp)
	{
     simple_uart_put(UartRecvBuf.dat[UartRecvBuf.rp]);
		 UartRecvBuf.rp++;
		if (UartRecvBuf.rp >= SIMPLE_UART_RX_BUF_SIZE)
		{
					UartRecvBuf.rp = 0;		 
		}
  }
	simple_uart_tx_stop();
	}
}

/**@brief   Function for handling UART interrupts.
 *
 * @details This function will receive a single character from the UART and append it to a string.
 *          The string will be be sent over BLE when the last character received was a 'new line'
 *          i.e '\n' (hex 0x0D) or if the string has reached a length of @ref NUS_MAX_DATA_LENGTH.
 */
void UART0_IRQHandler(void)
{

    /**@snippet [Handling the data received over UART] */
		// Handle reception
    if (NRF_UART0->EVENTS_RXDRDY != 0)
    {
				NRF_UART0->EVENTS_RXDRDY = 0;
				UartRecvBuf.dat[UartRecvBuf.wp] = NRF_UART0->RXD;
				UartRecvBuf.wp++;
				if (UartRecvBuf.wp >= SIMPLE_UART_RX_BUF_SIZE)
					UartRecvBuf.wp = 0;
		}
		// Handle transmission.
    if (NRF_UART0->EVENTS_TXDRDY != 0)
    {
				// Clear UART TX event flag.
				NRF_UART0->EVENTS_TXDRDY = 0;
    }
		if (NRF_UART0->EVENTS_ERROR != 0)
    {
        uint32_t       error_source;
        // Clear UART ERROR event flag.
        NRF_UART0->EVENTS_ERROR = 0;

        // Clear error source.
        error_source        = NRF_UART0->ERRORSRC;
        NRF_UART0->ERRORSRC = error_source;
    }
    /**@snippet [Handling the data received over UART] */
}
void uart_init_lowPower(void)
{
//  NRF_GPIO->DIRSET = RX_PIN_MASK|TX_PIN_MASK|CTS_PIN_MASK|RTS_PIN_MASK;
  nrf_gpio_cfg_input(RX_PIN_NUMBER, NRF_GPIO_PIN_PULLDOWN);
  nrf_gpio_cfg_input(TX_PIN_NUMBER, NRF_GPIO_PIN_PULLDOWN);
  nrf_gpio_cfg_input(CTS_PIN_NUMBER, NRF_GPIO_PIN_PULLDOWN);
  nrf_gpio_cfg_input(RTS_PIN_NUMBER, NRF_GPIO_PIN_PULLDOWN);

}
#endif
