#ifndef _DATA_MANGER_H_
#define _DATA_MANGER_H_
#include "ET1502B.h"
#include "GT24L16M1Y20.h"

#if DATA_MANGER_FEATURE_ENABLE


#if 0
#define FLASH_SECTOR_PRE_SIZE		(4096)
#if(ENABLE_GT24L16M1Y_UT)
#define DEV_INPUT_DATA_START_SECTOR			PROGRAMMING_ROM_ADDR_BASE+0x2000//0x100000//(0x2e0000)	//8K
#else
#define DEV_INPUT_DATA_START_SECTOR			0x100000//(0x2e0000)	//8K
#endif
#define DEV_GSENSOR_DATA_MANGER_START_ADDR	(DEV_INPUT_DATA_START_SECTOR + 0x2000)
#define DEV_GSENSOR_SPORT_DATA_START_ADDR 	(DEV_GSENSOR_DATA_MANGER_START_ADDR)
#define DEV_GSENSOR_SPORT_DATA_SECTOR_NUM	(20) 	//80k
#define DEV_GSENSOR_SLEEP_DATA_START_ADDR 	(DEV_GSENSOR_DATA_MANGER_START_ADDR + (DEV_GSENSOR_SPORT_DATA_SECTOR_NUM*FLASH_SECTOR_PRE_SIZE))
#define DEV_GSENSOR_SLEEP_DATA_SECTOR_NUM	(10) 	//40k
#define DEV_FLASH_SECTOR_TOTAL_NUM  		(32)  // 8K(APP) + 80K(sport_gsensor) + 40K(sleep_gsensor)  = 128K / 4 = 32


#define FLASH_RW_BLOCK_SIZE				(256)	// every write operation must be 256 intergrate times bytes
#define FLASH_PRE_SECTOR_BLOCK_NUM  	(FLASH_SECTOR_PRE_SIZE/FLASH_RW_BLOCK_SIZE)
#define FLASH_RECORD_SIZE				(16)

#define DATA_BLOCK_HEAD_SIZE			(16)
#define HEAD_PREVIOUS_HEAD_BYTE_NUM		(4)
#define HEAD_TIME_STAMP_BYTE_NUM		(8)
#define DATA_BLOCK_LOAD_SIZE			(FLASH_RW_BLOCK_SIZE - DATA_BLOCK_HEAD_SIZE)


// item remain max size
#define SPORT_ITEM_MAX_SIZE				(16) 
#define SLEEP_ITEM_MAX_SIZE				(16) 
#endif

#if 0
/*
�ֿ�8K
ÿ���˶�Ƭ������: 4K
˯������:40K
��ʷÿ���ۼ��˶�����:4K
USR Information �û���Ϣ�洢�ռ�4K
*/
#define FLASH_SECTOR_PRE_SIZE		(4096)
#if(ENABLE_GT24L16M1Y_UT)
#define DEV_INPUT_DATA_START_SECTOR			PROGRAMMING_ROM_ADDR_BASE+0x2000//0x100000//(0x2e0000)	//8K
#else
#define DEV_INPUT_DATA_START_SECTOR			0x100000//(0x2e0000)	//8K
#endif
/*USR Information �û���Ϣ�洢�ռ�8K ��ʼ��ַ0x2000*/
#define Usr_Infos_Data_Sector_NUM	(2) 

/*�˶����ݿ�ʼ��ַ0X4000*/
#define DEV_GSENSOR_DATA_MANGER_START_ADDR	(DEV_INPUT_DATA_START_SECTOR + 0x2000)
#define DEV_GSENSOR_SPORT_DATA_START_ADDR 	(DEV_GSENSOR_DATA_MANGER_START_ADDR)
#define DEV_GSENSOR_SPORT_DATA_SECTOR_NUM	(1) //  4K     //(20) 	//80k
/*˯�����ݿ�ʼ��ַ0X5000*/
#define DEV_GSENSOR_SLEEP_DATA_START_ADDR 	(DEV_GSENSOR_DATA_MANGER_START_ADDR + (DEV_GSENSOR_SPORT_DATA_SECTOR_NUM*FLASH_SECTOR_PRE_SIZE))
#define DEV_GSENSOR_SLEEP_DATA_SECTOR_NUM	(10) 	//40k
#define DEV_FLASH_SECTOR_TOTAL_NUM  		(32)  // 8K(APP) + 80K(sport_gsensor) + 40K(sleep_gsensor)  = 128K / 4 = 32


#define FLASH_RW_BLOCK_SIZE				(256)	// every write operation must be 256 intergrate times bytes
#define FLASH_PRE_SECTOR_BLOCK_NUM  	(FLASH_SECTOR_PRE_SIZE/FLASH_RW_BLOCK_SIZE)
#define FLASH_RECORD_SIZE				(16)

#define DATA_BLOCK_HEAD_SIZE			(16)
#define HEAD_PREVIOUS_HEAD_BYTE_NUM		(4)
#define HEAD_TIME_STAMP_BYTE_NUM		(8)
#define DATA_BLOCK_LOAD_SIZE			(FLASH_RW_BLOCK_SIZE - DATA_BLOCK_HEAD_SIZE)


/*��ʷÿ����ۻ���¼��ʼ��ַ:0XF000  8K�ռ�*/
#define DEV_GSENSOR_SPORT_Histry_DATA_START_ADDR     (DEV_GSENSOR_SLEEP_DATA_START_ADDR+(DEV_GSENSOR_SLEEP_DATA_SECTOR_NUM*FLASH_SECTOR_PRE_SIZE))
#define DEV_GSENSOR_SPORT_Histry_DATA_SECTOR_NUM	(2)
#define DEV_GSENSOR_SPORT_Histry_DATA_BLOCK_NUM	       (16)
#define DEV_GSENSOR_SPORT_Histry_DATA_Size	       (16)
#define DEV_GSENSOR_SPORT_Histry_DATA_Head_ADDR 	(DEV_GSENSOR_SPORT_Histry_DATA_START_ADDR+4096) /*ÿ����ʷ��¼�İ�ͷ*/
#endif


#if 1
/*
�ܹ�128K
		��    ��			 :8K		(0x0000~0x1FFF) 
USR Information �û���Ϣ   :8K		(0x2000~0x3FFF)
��ʷÿ����ۻ���¼:8K 		(0x4000~0x5FFF)
ÿ���˶�Ƭ������      :60K 	(0x6000~0x14FFF)
˯        ��       ��       ��      :44K	(0x15000~0x1FFFF)
*/
#define FLASH_SECTOR_PRE_SIZE		(4096)
#if(ENABLE_GT24L16M1Y_UT)
#define DEV_INPUT_DATA_START_SECTOR			PROGRAMMING_ROM_ADDR_BASE+0x2000	//8K
#else
#define DEV_INPUT_DATA_START_SECTOR			0x100000	// 4K
#endif

/*"ENTONG" 0x2000~0x2FFF   4K*/
#define Entong_char_Data_Sector_START_ADDR	(0x2000) 
#define Usr_Infos_Data_Sector_NUM	(1) 


/*USR Information 0x3000~0x3FFF   4K*/
#define Usr_Infos_Data_Sector_START_ADDR	(0x3000) 
#define Usr_Infos_Data_Sector_NUM	(1) 

/*Histry sport data addr:0x4000~0x5FFF  8K*/
#define DEV_GSENSOR_SPORT_Histry_DATA_START_ADDR     (0x4000)
#define DEV_GSENSOR_SPORT_Histry_DATA_SECTOR_NUM	(2)
#define DEV_GSENSOR_SPORT_Histry_DATA_BLOCK_NUM	       (16)
#define DEV_GSENSOR_SPORT_Histry_DATA_Size	       	(16)
#define DEV_GSENSOR_SPORT_Histry_DATA_Head_ADDR 	(DEV_GSENSOR_SPORT_Histry_DATA_START_ADDR+4096) /*ÿ����ʷ��¼�İ�ͷ*/


/*Sport data addr:0x6000~0x14FFF*/
#define DEV_GSENSOR_DATA_MANGER_START_ADDR	(0x600)
#define DEV_GSENSOR_SPORT_DATA_START_ADDR 	(0x6000)
#define DEV_GSENSOR_SPORT_DATA_SECTOR_NUM	(1) //  4K     //(20) 	//80k
#define DEV_GSENSOR_SPORT_DATA_SECTOR_NUM_ALL	(15) //  4K     //(20) 	//80k

/*Sleep data addr:0x15000~0x1FFFF*/
#define DEV_GSENSOR_SLEEP_DATA_START_ADDR 	(0x15000)
#define DEV_GSENSOR_SLEEP_DATA_SECTOR_NUM	(11) 	//44k

#define DEV_GSENSOR_SLEEP_DATA_DAYS	(3) // 3 DAY Sleep data
#define DEV_GSENSOR_SLEEP_RECORD_NUMBER	(DEV_GSENSOR_SLEEP_DATA_DAYS*24*60/5) // 3 DAY Sleep data
#define SLEEP_DATA_BLOCK_NUM	((DEV_GSENSOR_SLEEP_RECORD_NUMBER/15)+1) 



/*All data sector number*/
#define DEV_FLASH_SECTOR_TOTAL_NUM  		(32)  // 8K(APP) + 80K(sport_gsensor) + 40K(sleep_gsensor)  = 128K / 4 = 32


#define FLASH_RW_BLOCK_SIZE				(256)	// every write operation must be 256 intergrate times bytes
#define FLASH_PRE_SECTOR_BLOCK_NUM  	(FLASH_SECTOR_PRE_SIZE/FLASH_RW_BLOCK_SIZE)
#define FLASH_RECORD_SIZE				(16)

#define DATA_BLOCK_HEAD_SIZE			(16)
#define HEAD_PREVIOUS_HEAD_BYTE_NUM		(4)
#define HEAD_TIME_STAMP_BYTE_NUM		(8)
#define DATA_BLOCK_LOAD_SIZE			(FLASH_RW_BLOCK_SIZE - DATA_BLOCK_HEAD_SIZE)


#endif








// item remain max size
#define SPORT_ITEM_MAX_SIZE				(16) 
#define SLEEP_ITEM_MAX_SIZE				(16) 
/************************* flash data define ********************************/
#define FLASH_ERASE_LEBAL_SIZE       6
typedef struct
{
    unsigned char label[FLASH_ERASE_LEBAL_SIZE];
}flash_erase_label_t;


typedef union{
    unsigned short sector_maps;
    struct{
	    unsigned short bit_0   : 1;
	    unsigned short bit_1   : 1;
	    unsigned short bit_2   : 1;
	    unsigned short bit_3   : 1;
	    unsigned short bit_4   : 1;
	    unsigned short bit_5   : 1;
	    unsigned short bit_6   : 1;
	    unsigned short bit_7   : 1;
		unsigned short bit_8   : 1;
	    unsigned short bit_9   : 1;
	    unsigned short bit_10  : 1;
	    unsigned short bit_11  : 1;
	    unsigned short bit_12  : 1;
	    unsigned short bit_13  : 1;
	    unsigned short bit_14  : 1;
	    unsigned short bit_15  : 1;
    } bits;
}sector_bitmap_t ;

typedef struct
{
	unsigned char partitionStatus;
	unsigned int base_addr;			/*��ʼ��ַ8K��ʼ*/
	unsigned char totalBlockDataNum;//have write block number
	const unsigned short maxBlockDataNum;/*�ܹ����Blck(256) Num=80K/256byte=320*/
	unsigned char readSectorNum;	//read sector number
	unsigned char writeSectorNum;	//write sector number
	sector_bitmap_t * const sectorDataBitmaps;
	sector_bitmap_t * const sectorEraseBitmaps;
	const unsigned char sector_num;    /*�ܹ�Sector number*/
	
}flash_data_partition_t;


typedef struct
{
	unsigned char partitionStatus;
	unsigned int base_addr;			/*��ʼ��ַ8K��ʼ*/
	unsigned char totalBlockDataNum;//have write block number
	const unsigned short maxBlockDataNum;/*�ܹ����Blck(256) Num=80K/256byte=320*/
	unsigned char readSectorNum;	//read sector number
	unsigned char writeSectorNum;	//write sector number
	sector_bitmap_t * const sectorDataBitmaps;
	sector_bitmap_t * const sectorEraseBitmaps;
	const unsigned char sector_num;    /*�ܹ�Sector number*/
	
}Histrt_flash_data_partition_t;




typedef struct 
{
	unsigned char previousHead[HEAD_PREVIOUS_HEAD_BYTE_NUM];  /*�����ͷ�� ͷ 0x1a��0x2b��0x3c��0x4d*/
	unsigned char dataType;			/*������ļ�¼���ͣ��ǲ���0x01  ����  ˯��:0x02��*/
	unsigned char sotreMedia;  		/*�����¼��ŵط���MEM��0x00��FLASH��0x01��*/
	unsigned char dataItemSize;		/*�����¼�Ĵ�С��Ŀǰ�ǲ���˯�� ��СΪ16�ֽڣ�0x10��*/
	unsigned char dataItemNums;		/*�����������¼*/
	unsigned char timeStamp[HEAD_TIME_STAMP_BYTE_NUM];/*����ʱ�����Ŀǰȫ��Ϊ0x00*/
}data_block_head_t;

typedef struct
{
    unsigned char lock;
    unsigned char itemNums;
	unsigned char itemSize;
	unsigned char pdata[FLASH_RW_BLOCK_SIZE];
}gsensor_data_temp_struct;

typedef enum
{
	STORE_MEDIA_MEM = 0,
	STORE_MEDIA_FLASH,
}store_media_enum;

typedef struct /*��ͷ+���ݸ�ʽ*/
{
	data_block_head_t *head;
	unsigned char *pdata;
}data_block_t;

typedef struct
{
	unsigned char read_dataType;
	unsigned char read_ram_flag;
	unsigned char read_sectorIndex;
	unsigned char read_blockIndex;
}data_read_str;


typedef enum
{
	PARTITION_STATUS_EMPTY = 0,
	PARTITION_STATUS_NO_FULL,
	PARTITION_STATUS_FULL,
	PARTITION_STATUS_UNKNOW,
}partition_status_enum;

typedef enum{
	DEV_TO_APP_START_TYPE = 0,
	DATA_TYPE_SPORT,	
	DATA_TYPE_SLEEP,
	APP_TO_DEV_START_TYPE = 100,
	DATA_TYPE_USERINFO,
}DATA_TYPE;


extern unsigned char DateChangeFlg;

//extern const unsigned char g_DataHeadLabel[HEAD_PREVIOUS_HEAD_BYTE_NUM];


extern data_block_t g_RamSportBlock;   /*��ͷ+���ݸ�ʽ*/
extern data_block_t g_RamSleepBlock;   /*��ͷ+���ݸ�ʽ*/

//extern const unsigned char g_DataHeadLabel;
extern void SaveEveryDayHistySport_Data_info2_datamanger(void);
extern void data_manger_set_previous_head(unsigned char* pHead);
extern void Get_DataHeadLabel(unsigned char *pbuf);



extern data_block_t* get_ram_block(unsigned char dataType);

extern unsigned char write_data_item_2_mem_pool(unsigned char dataType, unsigned char* pData);
extern unsigned char flash_gsensor_data_partition_read_block(unsigned char dataType,unsigned char* pbuf,unsigned short *dataLength);
extern void erase_flash_gsensor_data(unsigned char flg);
extern void init_data_manger(void);
extern void write_infos_2_flash_first_page(void);
extern void flash_all_data_clear(void);

extern void flash_init(void);
extern void test_flash_write_and_read(void);
extern unsigned int get_flash_data_num(unsigned char dataType);
extern void read_flash(unsigned int addr, unsigned char *pBuf, unsigned short nByte);
#endif

#endif


