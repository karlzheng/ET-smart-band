#ifndef _ETSPI_H_
#define _ETSPI_H_
/*Winbond-W25Q128BV Flash*/
//Register1
#include <stdint.h>
#include "nrf_gpio.h"
#include "ET1502B.h"
#define NORFLASH_BUSY		(1<<0)
#define	NORFLASH_WEL		(1<<1)/*write enable latch*/

#define FLASH_PAGE_SIZE 256
/*
#ifdef ENABLE_GT24L16M1Y_UT
#define KEYID_START_ADDR 0x00000000
#else
#define KEYID_START_ADDR 0x00100000
#endif
*/
#define ENABLE_HARDWARE_SPI 0

typedef enum 
{
	FLASH_WriteEnable = 0x06,
	write_status_register_enable = 0x50,
	FLASH_WriteDisable = 0x04,
	FLASH_ReadStatusReg1 = 0x05,
	FLASH_ReadStatusReg2 = 0x35,
	FLASH_WriteStatusReg = 0x01,
	FLASH_PageProgram = 0x02,
	quad_page_program = 0x32,
	FLASH_SectorErase = 0x20,
	FLASH_BlockErase = 0xd8,
	chip_erase = 0x60,
	FLASH_ChipErase=0xC7,
	FLASH_PowerDown = 0xb9,
	continuous_read_mode_reset = 0xff,
	FLASH_ReadData = 0x03,
	FLASH_FastReadData = 0x0b,
	FLASH_FastReadDual = 0x3b,
	fast_read_quad_output = 0x6b,
	fast_read_dual_io = 0xbb,
	fast_read_quad_io = 0xeb,
	word_read_quad_io = 0xe7,
	octal_word_read_quad_io = 0xe3,
	FLASH_ReleasePowerDown = 0xab,
	FLASH_ManufactDeviceID = 0x90,
	FLASH_JedecDeviceID = 0x9f
}FLASH_COMMAND;



extern void etSpim1NorFlashSectorErase(uint32_t start_add);
extern void etSpim1NorFlashBlockErase(unsigned start_add);
extern void etSpim1NorFlashChipErase(void);
extern void etSpim1NorFlashPageProgrameStdMode(uint32_t start_add, uint8_t *write_buf, uint32_t write_len);
extern void etSpim1ReadNorFlashStdMode(uint32_t start_add, uint8_t *data_buf, uint32_t read_len);
extern void etGensor_spi_read(unsigned char reg_addr, unsigned char *buffer, unsigned short len);
extern void etGensor_spi_write(unsigned char reg_addr, unsigned char *buffer, unsigned short len);
extern void etWrite_KEYID_to_flash(uint32_t reg_addr,unsigned char *buffer, unsigned short len);
extern void etRead_KEYID_from_flash(unsigned char *buffer,unsigned short len);
extern void etSpim1NorFlashInit(void);
#endif
