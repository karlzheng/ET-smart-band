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
#include "app_util_platform.h"
#include "ET1502B.h"
#include "uartProtocol.h"
#include "btprotocol.h"
#include "et_debug.h"
#define SIMPLE_UART_RX_BUF_SIZE                512
#define UART_INIT_ENABLE 1
#define UART_INIT_DISABLE 0



static uint8_t  uart_rev_data[SIMPLE_UART_RX_BUF_SIZE];
static uint16_t uart_rev_index = 0;
static uint16_t uart_send_index = 0;
static uint8_t  uart_init_check=UART_INIT_DISABLE;
static uint8_t  uart_close_PW_delay=0;
uint8_t simple_uart_get(void)
{
    while (NRF_UART0->EVENTS_RXDRDY != 1)
    {
        // Wait for RXD data to be received
    }

    NRF_UART0->EVENTS_RXDRDY = 0;
    return (uint8_t)NRF_UART0->RXD;
}

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
   // app_uart_evt_t app_uart_event;
    // No more bytes in FIFO, terminate transmission.
    NRF_UART0->TASKS_STOPTX = 1;
    //m_current_state         = UART_READY;
    // Last byte from FIFO transmitted, notify the application.
    // Notify that new data is available if this was first byte put in the buffer.
   // app_uart_event.evt_type = APP_UART_TX_EMPTY;
   // m_event_handler(&app_uart_event);
}
void simple_uart_put(uint8_t cr)
{
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
    if(Uart_open_check()==0)
    {
      return;
    } 
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
    if(Uart_open_check()==0)
    {
      return;
    }
    simple_uart_tx_send();
    while (i<len)
    {
        simple_uart_put(str[i]);
        //ch = str[i++];
			  i++;
    }
		simple_uart_tx_stop();
		
}
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
unsigned char UartRecv(unsigned char *buf)
{
	if (uart_send_index != uart_rev_index)
	{
		*buf = uart_rev_data[uart_send_index];
		uart_send_index++;
		if (uart_send_index >= SIMPLE_UART_RX_BUF_SIZE)
		{
			uart_send_index = 0;
		}
		return 1;
	}
	return 0;
}
/*
void uart_Looptest(void)
{
	if(uart_rev_index!=uart_send_index)
	{
	simple_uart_tx_send();
	while(uart_rev_index!=uart_send_index)
	{
     simple_uart_put(uart_rev_data[uart_send_index]);		
		 uart_send_index++;
	  if(uart_send_index >= SIMPLE_UART_RX_BUF_SIZE)
		{
      uart_send_index=0;
    }		
		 
  }
	simple_uart_tx_stop();
	}
}
*/
/**@brief   Function for handling UART interrupts.
 *
 * @details This function will receive a single character from the UART and append it to a string.
 *          The string will be be sent over BLE when the last character received was a 'new line'
 *          i.e '\n' (hex 0x0D) or if the string has reached a length of @ref NUS_MAX_DATA_LENGTH.
 */
void UART0_IRQHandler(void)
{
    //uart_rev_data[uart_rev_index] = simple_uart_get();
    //uart_rev_index++;
	  //if(uart_rev_index >= SIMPLE_UART_RX_BUF_SIZE)
		//{
    //  uart_rev_index=0;
    //}
    if (NRF_UART0->EVENTS_RXDRDY != 0)
    {
				NRF_UART0->EVENTS_RXDRDY = 0;
				uart_rev_data[uart_rev_index ] = NRF_UART0->RXD;
				uart_rev_index ++;
				if (uart_rev_index>= SIMPLE_UART_RX_BUF_SIZE)
					uart_rev_index = 0;
		}		
}
void uart_init_lowPower(void)
{
#if DEBUG_UART_EN == 0
//  NRF_GPIO->DIRSET = RX_PIN_MASK|TX_PIN_MASK|CTS_PIN_MASK|RTS_PIN_MASK;
  if(uart_init_check==UART_INIT_ENABLE)
  {
#if ENABLE_COS
// #if(ENABLE_COS)    
      
    z8_Power_ctr(POWER_OFF); 
    cos_clear_buff(); 
    save_COS_status(0x00);    
    Clr_Cos_Ready();
        
//#endif     
    uart_init_check=UART_INIT_DISABLE;
    
    NRF_UART0->ENABLE=UART_ENABLE_ENABLE_Disabled;
    nrf_gpio_cfg_input(RX_PIN_NUMBER, NRF_GPIO_PIN_PULLDOWN);
    nrf_gpio_cfg_input(TX_PIN_NUMBER, NRF_GPIO_PIN_PULLDOWN);
    nrf_gpio_cfg_input(CTS_PIN_NUMBER, NRF_GPIO_PIN_PULLDOWN);
    nrf_gpio_cfg_input(RTS_PIN_NUMBER, NRF_GPIO_PIN_PULLDOWN); 
    
    /*nrf_gpio_cfg_output(RX_PIN_NUMBER);
    nrf_gpio_cfg_output(TX_PIN_NUMBER);
    nrf_gpio_cfg_output(CTS_PIN_NUMBER);
    //nrf_gpio_cfg_output(RTS_PIN_NUMBER);   
    NRF_GPIO->OUTCLR = RX_PIN_MASK;
    NRF_GPIO->OUTCLR = TX_PIN_MASK;
    NRF_GPIO->OUTCLR = CTS_PIN_MASK;*/
    //NRF_GPIO->OUTCLR = RTS_PIN_MASK;    
#endif
  }
#endif
}
/**@snippet [UART Initialization] */
void simple_uart_init(unsigned char t_delay)
{
    if(uart_init_check==UART_INIT_DISABLE)
    {
    
    simple_uart_config(RTS_PIN_NUMBER, TX_PIN_NUMBER, CTS_PIN_NUMBER, RX_PIN_NUMBER, HWFC);    
    NRF_UART0->INTENSET = UART_INTENSET_RXDRDY_Enabled << UART_INTENSET_RXDRDY_Pos;    
    NVIC_SetPriority(UART0_IRQn, APP_IRQ_PRIORITY_LOW);
    NVIC_EnableIRQ(UART0_IRQn);
    uart_init_check=UART_INIT_ENABLE;
#if(ENABLE_COS)
    Clr_Cos_Ready();
    save_COS_status(0x00);
    cos_clear_buff();
    z8_Power_ctr(POWER_ON);
    //ans_work_state_handle();	
#endif  
     uart_close_PW_delay=t_delay;
      
     //nrf_delay_ms(200);
      
    }
    else
    {
     uart_close_PW_delay=t_delay;
    }
}

unsigned char Uart_open_check(void)
{
  if(uart_init_check==UART_INIT_ENABLE)
  {
   return 1;
  }
  else
    return 0;
}
#if ENABLE_COS
void etCheck_to_PoweroffCOS(void)
{
 #if DEBUG_UART_EN    
	//		DbgPrintf("uart_init_check=%d,delay=%d\r\n",uart_init_check,uart_close_PW_delay);
#endif	

  if(uart_init_check==UART_INIT_ENABLE)
  {
  
    //if(Protocol_check_BT_connected()==0)
    //{
    //   uart_init_lowPower();
    //}
    //else
    //{
      save_COS_status(0x00);
      if(uart_close_PW_delay>0)
      {
        uart_close_PW_delay--;
    
        if(uart_close_PW_delay==0)
        {    
          uart_init_lowPower();
        }
      }
      else
      {
        uart_init_lowPower();
      }
      
    //}
  }
}
#endif
#endif
