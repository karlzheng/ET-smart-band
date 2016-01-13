#include "ET1502B.h"
#include "nrf_gpio.h"
#include "et_define.h"
//#define SPI_MISO_HIGH   NRF_GPIO->OUTSET= SPIM0_MISO_MASK                          
//#define SPI_MISO_LOW    NRF_GPIO->OUTCLR = SPIM0_MISO_MASK

#define SPI_MOSI_HIGH   NRF_GPIO->OUTSET = SPIM0_MOSI_PIN_MASK
#define SPI_MOSI_LOW    NRF_GPIO->OUTCLR = SPIM0_MOSI_PIN_MASK

//#define SPI_CS_HIGH   NRF_GPIO->OUTSET = SFLASH_SPIM0_SS_PIN_MASK
//#define SPI_CS_LOW    NRF_GPIO->OUTCLR = SFLASH_SPIM0_SS_PIN_MASK

#define SPI_CLK_HIGH   NRF_GPIO->OUTSET = SPIM0_SCK_PIN_MASK
#define SPI_CLK_LOW    NRF_GPIO->OUTCLR = SPIM0_SCK_PIN_MASK

#ifndef  ENABLE_NEW_BOARD
  #define G_SPI_CS_HIGH   NRF_GPIO->OUTSET = GSENSOR_SPIM0_SS_PIN_MASK
  //#define G_SPI_CS_LOW    NRF_GPIO->OUTCLR = GSENSOR_SPIM0_SS_PIN_MASK                        
  #define G_SPI_CS_LOW 	do {NRF_GPIO->OUTSET = SFLASH_SPIM0_SS_PIN_MASK; \
                NRF_GPIO->OUTCLR = GSENSOR_SPIM0_SS_PIN_MASK; } while (0)
#endif

#define FLASH_CS_HIGH   NRF_GPIO->OUTSET = SFLASH_SPIM0_SS_PIN_MASK//nrf_gpio_pin_set(FLASH_SPIM0_SS_PIN);

#ifndef  ENABLE_NEW_BOARD
  #define FLASH_CS_LOW 	do {NRF_GPIO->OUTSET = GSENSOR_SPIM0_SS_PIN_MASK; \
                NRF_GPIO->OUTCLR = SFLASH_SPIM0_SS_PIN_MASK; } while (0)
#else
  #define FLASH_CS_LOW    NRF_GPIO->OUTCLR =SFLASH_SPIM0_SS_PIN_MASK//nrf_gpio_pin_clear(FLASH_SPIM0_SS_PIN)
#endif
//#define FLASH_CS_OUTPUT  nrf_gpio_cfg_output(FLASH_SPIM0_SS_PIN);

//#define SENSOR_CS_LOW  NRF_GPIO->OUTCLR =GSENSOR_SPIM0_SS_PIN_MASK//nrf_gpio_pin_clear(GSENSOR_SPIM0_SS_PIN)
//#define SENSOR_CS_HIGH   NRF_GPIO->OUTSET =GSENSOR_SPIM0_SS_PIN_MASK //nrf_gpio_pin_set(GSENSOR_SPIM0_SS_PIN);
//#define SENSOR_CS_OUTPUT  nrf_gpio_cfg_output(GSENSOR_SPIM0_SS_PIN);


#define SPI_DATA_GET  nrf_gpio_pin_read(SPIM0_MISO_PIN)

void SpiSent_1Byte(unsigned char dat);
void SpiSentByte(UINT8 *write,UINT32 len);
void SpiReadByte(UINT8 *read, UINT32 len);
//void GPIO_Spi_read(UINT32 SectorAddr, UINT8 *buffer, UINT32 len);
//void GPIO_Spi_write(UINT32 SectorAddr,const UINT8 *buffer, UINT32 len);
void GPIO_Spi_init(void);

void GPIO_Spi_MISO_high(void);
void GPIO_Spi_MISO_low(void);

