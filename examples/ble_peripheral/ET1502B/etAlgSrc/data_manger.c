#include "ET1502B.h"
#include "data_manger.h"
#include "et_WDT.h"
#if DATA_MANGER_FEATURE_ENABLE
#include "et_debug.h"
//#include "arch.h"
#include "string.h"
#include "sleep_record.h"
#include "arithmetic_data_manger.h"
#include "free_notic.h" 
#include "ke_timer.h"
#include "usr_task.h"
#include "etSpi.h"


#define FLASH_ERASE_LABEL	"ENTONG"
const unsigned char g_DataHeadLabel[HEAD_PREVIOUS_HEAD_BYTE_NUM]= {0x1A, 0x2B, 0x3C, 0x4D};

static unsigned char sport_buf[FLASH_RW_BLOCK_SIZE] = {0};
static unsigned char sleep_buf[FLASH_RW_BLOCK_SIZE] = {0};

static sector_bitmap_t g_flash_sport_DataPartitionSectorMaps[DEV_GSENSOR_SPORT_DATA_SECTOR_NUM]={{0,}};
static sector_bitmap_t g_flash_sport_ErasePartitionSectorMaps[DEV_GSENSOR_SPORT_DATA_SECTOR_NUM]={{0,}};

static sector_bitmap_t g_flash_sleep_DataPartitionSectorMaps[DEV_GSENSOR_SLEEP_DATA_SECTOR_NUM]={{0,}};
static sector_bitmap_t g_flash_sleep_ErasePartitionSectorMaps[DEV_GSENSOR_SLEEP_DATA_SECTOR_NUM]={{0,}};

data_block_t g_RamSportBlock;   /*包头+数据格式*/
data_block_t g_RamSleepBlock;   /*包头+数据格式*/

data_read_str g_DataReadStr = {0,0,0};
gsensor_data_temp_struct gsensor_data_temp={0};
flash_erase_label_t g_erase_label = {"ENTONG"};



//unsigned char DateChangeFlg=0;/*每天结束标志*/
//static unsigned short DATANumber=0;/*总存储多少天的记录最大255天*/



flash_data_partition_t g_flash_sport_DataPartition =
{
	PARTITION_STATUS_EMPTY,
	DEV_GSENSOR_SPORT_DATA_START_ADDR,
	0,
	FLASH_PRE_SECTOR_BLOCK_NUM*DEV_GSENSOR_SPORT_DATA_SECTOR_NUM,
	0,
	0,
	g_flash_sport_DataPartitionSectorMaps,
	g_flash_sport_ErasePartitionSectorMaps,
	DEV_GSENSOR_SPORT_DATA_SECTOR_NUM,
};

flash_data_partition_t g_flash_sleep_DataPartition =
{
	PARTITION_STATUS_EMPTY,
	DEV_GSENSOR_SLEEP_DATA_START_ADDR,
	0,
	FLASH_PRE_SECTOR_BLOCK_NUM*DEV_GSENSOR_SLEEP_DATA_SECTOR_NUM,
	0,
	0,
	g_flash_sleep_DataPartitionSectorMaps,
	g_flash_sleep_ErasePartitionSectorMaps,
	DEV_GSENSOR_SLEEP_DATA_SECTOR_NUM,
};
/*
void flash_init(void)
{
	unsigned char ReadIDBuf[8];
	Spim1Init(0, HSIZE_BYTE, DSSP_8BIT);
	Spim1SetBaudrate(2, 2);
	
	Spim1CheckNorFlashBusy(); 
	Spim1CheckNorFlashWriteEnable(); 
	memset(ReadIDBuf, 0, sizeof(ReadIDBuf)); 
	Spim1ReadNorFlashID(ReadIDBuf); 
	Spim1CheckNorFlashBusy();
}
*/
void read_flash(unsigned int addr, unsigned char *pBuf, unsigned short nByte)
{
	etSpim1ReadNorFlashStdMode(addr, pBuf, nByte);
}

static void write_flash(unsigned int  addr, unsigned char *pBuf, unsigned short nByte)
{
	etSpim1NorFlashPageProgrameStdMode(addr, pBuf, nByte);
}

static void sector_erase_flash(unsigned int addr, unsigned short n)
{
	while (n--)
    {
       //etSpim1CheckNorFlashWriteEnable();
		    //etSpim1NorFlashSectorErase(addr);
        etSpim1NorFlashSectorErase(addr);
      
#if ENABLE_WDT	 
     et_WDT_feed();/*WDT Clear*/
#endif			  
        addr += 4*1024;            //all flash sector erase command erasing size is 4K
      
    }
}

static void init_ram_pools(void)
{
    g_RamSportBlock.head = (data_block_head_t *)sport_buf;    
	g_RamSportBlock.head->dataType = DATA_TYPE_SPORT;
	g_RamSportBlock.head->sotreMedia = STORE_MEDIA_MEM;
	g_RamSportBlock.head->dataItemSize = SPORT_ITEM_MAX_SIZE;
	g_RamSportBlock.head->dataItemNums = 0;
	
    memcpy(g_RamSportBlock.head->previousHead, g_DataHeadLabel, HEAD_PREVIOUS_HEAD_BYTE_NUM);//Nick
	memset(g_RamSportBlock.head->timeStamp,0,HEAD_TIME_STAMP_BYTE_NUM);
	g_RamSportBlock.pdata = (unsigned char*)sport_buf + DATA_BLOCK_HEAD_SIZE;


    g_RamSleepBlock.head = (data_block_head_t *)sleep_buf;
	g_RamSleepBlock.head->dataType = DATA_TYPE_SLEEP;
	g_RamSleepBlock.head->sotreMedia = STORE_MEDIA_MEM;
	g_RamSleepBlock.head->dataItemSize = SLEEP_ITEM_MAX_SIZE;
	g_RamSleepBlock.head->dataItemNums = 0;
    memcpy(g_RamSleepBlock.head->previousHead, g_DataHeadLabel, HEAD_PREVIOUS_HEAD_BYTE_NUM);//Nick
	memset(g_RamSleepBlock.head->timeStamp,0,HEAD_TIME_STAMP_BYTE_NUM);
	g_RamSleepBlock.pdata = (unsigned char*)sleep_buf + DATA_BLOCK_HEAD_SIZE;
}

void data_manger_set_previous_head(unsigned char* pHead)
{
	*pHead     = 0x1A;
	*(pHead+1) = 0x2B;
	*(pHead+2) = 0x3C;
	*(pHead+3) = 0x4D;
}
/*
static void data_manger_set_time_stamp(unsigned char* pTimeStamp)
{
	;
}
*/
static void ram_block_reset(data_block_t* pRamBlock)
{
	if(pRamBlock)
	{
		pRamBlock->head->dataItemNums = 0;
		pRamBlock->head->sotreMedia = STORE_MEDIA_MEM;
		memset(pRamBlock->pdata, 0, DATA_BLOCK_LOAD_SIZE);
	}
}

void Get_DataHeadLabel(unsigned char *pbuf)
{
	memcpy(pbuf,g_DataHeadLabel,sizeof(g_DataHeadLabel));
}

data_block_t* get_ram_block(unsigned char dataType)
{
	data_block_t* pRamBlock = NULL;
	switch(dataType)
	{
		case DATA_TYPE_SPORT:
			pRamBlock = (data_block_t*)&g_RamSportBlock;
			break;
		case DATA_TYPE_SLEEP:
			pRamBlock = (data_block_t*)&g_RamSleepBlock;
			break;
		default:
			break;
	}
	return pRamBlock;
}

flash_data_partition_t * get_dataPartition(unsigned char dataType)
{
	flash_data_partition_t* dataPartition = NULL;
	
	switch(dataType)
	{
		case DATA_TYPE_SPORT:
			dataPartition = &g_flash_sport_DataPartition;
			break;
		case DATA_TYPE_SLEEP:
			dataPartition = &g_flash_sleep_DataPartition;
			break;
		default:
			break;
	}
	return dataPartition;
}

static unsigned char ram_gsensor_data_write_block_2_flash(unsigned char dataType)
{
	flash_data_partition_t* dataPartition;
	data_block_t* pBlockData = get_ram_block(dataType);

	unsigned int flashAddr = 0;
	unsigned char i = 0, j = 0, find = 0, ret = 0;

	dataPartition = get_dataPartition(dataType);
	if(!dataPartition)
	{
		return 0; 
	}
		
	if (dataPartition->totalBlockDataNum == dataPartition->maxBlockDataNum)/*FLASH FULL*/
    	{
		//DATA_MANA_DUG("gsensor write flash: no idle flash block to write, need to release flash block\r\n");
		//flash_gsensor_data_partition_reset();
    	}
	
	for(i = dataPartition->writeSectorNum; i < dataPartition->sector_num; i++)
	{
		if (dataPartition->sectorDataBitmaps[i].sector_maps != 0xffff)/*4096/256=16*//*判断已经写到哪个SENTOR*/
        	{
        		dataPartition->writeSectorNum = i; /*该SENTOR未写完，从这个个SECTOR开始*/
			find = 1;
           		break;
        	}
	}
	if(find == 0)  /*该SENTOR未写完，后续从此SECTOR开始*/
	{	
		//if(dataType==DATA_TYPE_SPORT){dataPartition->writeSectorNum=DEV_GSENSOR_SPORT_DATA_SECTOR_NUM;}//Nick ADD
	
		for(i = 0; i < dataPartition->writeSectorNum; i++)
		{
			if (dataPartition->sectorDataBitmaps[i].sector_maps != 0xffff)
	        {
	        	dataPartition->writeSectorNum = i;
				find = 1;
	            break;
	        }
		}
	}/* i == SectorNum*/
	
	if(find)    /*该SENTOR已经写完，后续从下一个SECTOR开始*/
	{
		for(j = 0; j < FLASH_PRE_SECTOR_BLOCK_NUM; j++)   /*取出总共16个BLOCK(256 BYTE)中应该从那个BLOCK开始*/
		{
			if(!(dataPartition->sectorDataBitmaps[i].sector_maps & (1<<j)))
			{
				break;     /* j == BLOCK*/
			}
		}
		
		flashAddr = dataPartition->base_addr + i*FLASH_SECTOR_PRE_SIZE + j*FLASH_RW_BLOCK_SIZE;
		write_flash(flashAddr, (unsigned char *)pBlockData->head, FLASH_RW_BLOCK_SIZE);
        	ret = 1;
		/*数据写完设置标志为和其他参数*/	
		dataPartition->sectorDataBitmaps[i].sector_maps |= 1<<j;	/*保存16 BLOCK 已写过的标志为1*/
		dataPartition->totalBlockDataNum += 1;
		if (dataPartition->sectorDataBitmaps[i].sector_maps == 0xffff)
        	{
        		if((++dataPartition->writeSectorNum) >= dataPartition->sector_num)/*判断是否满了。*/
       		{
				dataPartition->writeSectorNum = 0;       /*满了，清除*/
        		}
        	}
	}

	if(dataPartition->totalBlockDataNum == dataPartition->maxBlockDataNum)
	{
		dataPartition->partitionStatus = PARTITION_STATUS_FULL;
    	}
	else
	{		
       	dataPartition->partitionStatus = PARTITION_STATUS_NO_FULL;
	}

	DATA_MANA_DUG("status=%d,totalBlockDataNum=%d,readSectorNum=%d,writeSecNum=%d,dataBitMap=0x%x,dataEarBitMap=0x%x,\r\n", 
		dataPartition->partitionStatus,
		dataPartition->totalBlockDataNum,
		dataPartition->readSectorNum,
		dataPartition->writeSectorNum,
		dataPartition->sectorDataBitmaps[i].sector_maps,
		dataPartition->sectorEraseBitmaps[dataPartition->readSectorNum].sector_maps);

	return ret;
}


/*256byte RAM 已经满此处存储到flash*/
unsigned char flush_mem_poll2flash(unsigned char dataType)
{
	unsigned char ret = 0;
	if((dataType == DATA_TYPE_SPORT) || (dataType == DATA_TYPE_SLEEP))
	{
		data_block_t* pRamBlock = get_ram_block(dataType);

		if(pRamBlock->head->dataItemNums == 0)
			return 0;
	
	
		data_manger_set_previous_head(pRamBlock->head->previousHead);/*加头0x22命令包头*/
		//data_manger_set_time_stamp(pRamBlock->head->timeStamp);
		pRamBlock->head->sotreMedia = STORE_MEDIA_FLASH;
	
        ret = ram_gsensor_data_write_block_2_flash(dataType);
		ram_block_reset(pRamBlock);
		if(ret == 0)
		{
            		//DATA_MANA_DUG("write data item 2 mem pool: gsensor data write pool fail\r\n");
			//DATA_MANA_DUG("ram to flash fail\r\n");
			return 1;
        	}
	}
	return ret;
}
#if 0
unsigned char get_mem_data(unsigned char *str)//kevin add
{
	//unsigned char ret = 0;
	//if((dataType == DATA_TYPE_SPORT) || (dataType == DATA_TYPE_SLEEP))
	//{
		data_block_t* pRamBlock = get_ram_block(DATA_TYPE_SPORT);
    //unsigned char * pRamBlock = (unsigned char *)get_ram_block(DATA_TYPE_SPORT);
		if(pRamBlock->head->dataItemNums == 0)
			return 0;
		str= (unsigned char *)pRamBlock;
	  return pRamBlock->head->dataItemNums;
		
	  #if 0
		data_manger_set_previous_head(pRamBlock->head->previousHead);/*加头0x22命令包头*/
		//data_manger_set_time_stamp(pRamBlock->head->timeStamp);
		pRamBlock->head->sotreMedia = STORE_MEDIA_FLASH;
	
        ret = ram_gsensor_data_write_block_2_flash(dataType);
		ram_block_reset(pRamBlock);
		if(ret == 0)
		{
            		//DATA_MANA_DUG("write data item 2 mem pool: gsensor data write pool fail\r\n");
			//DATA_MANA_DUG("ram to flash fail\r\n");
			return 1;
    }
		#endif
	//}
	//return ret;
	
}
#endif
unsigned char write_data_item_2_mem_pool(unsigned char dataType, unsigned char* pData)
{
	data_block_t* pRamBlock = get_ram_block(dataType);
	unsigned char *pDataPos = NULL;
    unsigned char ret = 0;

	if(!pRamBlock)
	{
		//DATA_MANA_DUG("the ramblock type is null\r\n");
		return 1;
	}
	/* if data has read,so lock it ,wait read finish unlock it,
	** then write it */
	if(gsensor_data_temp.lock == dataType)
	{
		gsensor_data_temp.itemSize = pRamBlock->head->dataItemSize;
		if(gsensor_data_temp.itemNums < (sizeof(gsensor_data_temp.pdata)/gsensor_data_temp.itemSize))
		{
			DATA_MANA_DUG("gsensor data has lock,so save data to temp\r\n");
			pDataPos = gsensor_data_temp.pdata + gsensor_data_temp.itemNums * gsensor_data_temp.itemSize;
			memcpy(pDataPos,pData,gsensor_data_temp.itemSize);
			gsensor_data_temp.itemNums += 1;
			return 0;
		}
		DATA_MANA_DUG("gsensor temp data itemNums is %d full\r\n",gsensor_data_temp.itemNums);
	}

	pDataPos = pRamBlock->pdata + pRamBlock->head->dataItemNums * pRamBlock->head->dataItemSize;
	
	memcpy(pDataPos,pData,pRamBlock->head->dataItemSize);
	
	pRamBlock->head->dataItemNums += 1;
	
    	pRamBlock->head->dataType = dataType;
	
    	DATA_MANA_DUG("write data 2 mem pool: data type=%d, size=%d, item nums=%d\r\n", 
                                        dataType, pRamBlock->head->dataItemSize, pRamBlock->head->dataItemNums);
	
	if(pRamBlock->head->dataItemNums == DATA_BLOCK_LOAD_SIZE/pRamBlock->head->dataItemSize)  /*数据=256-16 RAM 满了*/

	{
		DATA_MANA_DUG("write data 2 mem pool: RAM data is full,write to flash now\r\n");
		ret = flush_mem_poll2flash(dataType);
       	return ret;
	}
	return 0;
}

static void flash_gsensor_data_partition_erase_sector(unsigned char dataType,unsigned char index)
{
	flash_data_partition_t* dataPartition;
	unsigned int flashAddr = 0;
	
	dataPartition = get_dataPartition(dataType);
	if(dataPartition == NULL)
	{
		return; 
	}
	
    DATA_MANA_DUG("gsensor erase sector: sector num=%d\r\n", index);
	if(index >= dataPartition->sector_num)
	{
        DATA_MANA_DUG("gsensor data erase sector: index larger than data sector max num\r\n");
		return;
    }

	flashAddr = dataPartition->base_addr + index*FLASH_SECTOR_PRE_SIZE;
	if((flashAddr < DEV_GSENSOR_DATA_MANGER_START_ADDR)||
	   (flashAddr > (DEV_GSENSOR_DATA_MANGER_START_ADDR+(DEV_GSENSOR_SPORT_DATA_SECTOR_NUM_ALL+DEV_GSENSOR_SLEEP_DATA_SECTOR_NUM-1)*FLASH_SECTOR_PRE_SIZE)))	
	{
		DATA_MANA_DUG("gsensor data erase addr is wrong\r\n");
		return;
	}

	sector_erase_flash(flashAddr,1);
}


/********************************************************
** return 0 is no find ,return 1 is find
**
*********************************************************/
static unsigned char flash_gsensor_data_partition_find(unsigned char dataType,unsigned char *pSectorIndex, unsigned char* pBlockIndex, unsigned char start,unsigned char end)
{
	flash_data_partition_t* dataPartition;
	unsigned char i = 0, j = 0;

	dataPartition = get_dataPartition(dataType);
	if(dataPartition == NULL)
	{
		return 0; 
	}
		
	for(i = start; i < end; i++)
	{
		if(dataPartition->sectorDataBitmaps[i].sector_maps != 0)
		{
			for(j = 0; j < FLASH_PRE_SECTOR_BLOCK_NUM; j++)
			{
				if((dataPartition->sectorDataBitmaps[i].sector_maps & (1<<j))
					&& (!(dataPartition->sectorEraseBitmaps[i].sector_maps & (1<<j))))
				{
					dataPartition->readSectorNum = i;
					*pSectorIndex = i;
					*pBlockIndex = j;
					return 1;
				}
			}
		}
	}
	return 0;
}


unsigned int get_flash_data_num(unsigned char dataType)
{
	unsigned int num = 0;

	if(dataType == DATA_TYPE_SPORT)
	{
		num = g_flash_sport_DataPartition.totalBlockDataNum;/*FLASH中的位置*/
		if(num==0)
			num = g_RamSportBlock.head->dataItemNums;/*FLASH中位置为0 就取RAM中的位置*/
	}
	else if(dataType == DATA_TYPE_SLEEP)
	{
		num = g_flash_sleep_DataPartition.totalBlockDataNum;
		if(num==0)
			num = g_RamSleepBlock.head->dataItemNums;
	}

	return num;
}

static void set_gsensor_data_lock(unsigned char dataType, unsigned char onoff)
{
	if(onoff)
	{
		DATA_MANA_DUG("lock gsensor data\r\n");
		gsensor_data_temp.lock = dataType;
	}
	else
	{
		if(gsensor_data_temp.lock)
		{
			DATA_MANA_DUG("unlock gsensor data,itemNums=%d\r\n",gsensor_data_temp.itemNums);
			dataType = gsensor_data_temp.lock;
			gsensor_data_temp.lock = 0;	//must clear it ,then save data !
			for(unsigned char i = 0;i<gsensor_data_temp.itemNums;i++)
			{
				write_data_item_2_mem_pool(dataType,&gsensor_data_temp.pdata[i*gsensor_data_temp.itemSize]);
			}
			gsensor_data_temp.itemNums = 0;
			gsensor_data_temp.itemSize = 0;
			gsensor_data_temp.lock = 0;
			memset(gsensor_data_temp.pdata,0,FLASH_RW_BLOCK_SIZE);
		}
	}
}

/******************************************************
** read gsensor data 
** if return 0,read data from ram pool,if return 1,read data
** from flash,if return 2,read NULL
*******************************************************/
unsigned char flash_gsensor_data_partition_read_block(unsigned char dataType,unsigned char* pbuf,unsigned short *dataLength)
{
	unsigned int flashAddr = 0;
	flash_data_partition_t* dataPartition;
	unsigned char find=0;
	data_block_t* pRamBlock = get_ram_block(dataType);

	*dataLength= 0;
	g_DataReadStr.read_dataType = 0;
	
	dataPartition = get_dataPartition(dataType);
	if((dataPartition == NULL)||(pRamBlock == NULL))
	{
		return 2; 
	}

	if((dataPartition->partitionStatus == PARTITION_STATUS_EMPTY)  /*空*/
		&& (pRamBlock->head->dataItemNums == 0))	/*记录为0*/

	{
		return 2;
	}
	g_DataReadStr.read_dataType = dataType;
	
	/* if flash is empty ,so read data from ram pool */
	//if(dataPartition->partitionStatus == PARTITION_STATUS_EMPTY)
	//if(((pRamBlock->head->dataItemNums > 0)&&(dataType == DATA_TYPE_SPORT))
	//  || ((dataPartition->partitionStatus == PARTITION_STATUS_EMPTY)&&(dataType == DATA_TYPE_SLEEP)))
	
	/*read data from RAM*/
	if(dataPartition->partitionStatus == PARTITION_STATUS_EMPTY)
	{
		DATA_MANA_DUG("read data from ram \r\n");
		memcpy(pbuf,pRamBlock->head,FLASH_RW_BLOCK_SIZE);
		g_DataReadStr.read_ram_flag = STORE_MEDIA_MEM;
		
		*dataLength = (pRamBlock->head->dataItemNums+1)* DATA_BLOCK_HEAD_SIZE;// 包头+记录 长度
		//when read ram data,so lock ram data!
		set_gsensor_data_lock(dataType,1);
		return 0;
	}
	else	/*read data from flash*/
	{	
		find = flash_gsensor_data_partition_find(dataType,&g_DataReadStr.read_sectorIndex, &g_DataReadStr.read_blockIndex,
										dataPartition->readSectorNum,
										dataPartition->sector_num);
		if(find == 0)
		{
			find = flash_gsensor_data_partition_find(dataType,&g_DataReadStr.read_sectorIndex, &g_DataReadStr.read_blockIndex,
										0,
										dataPartition->readSectorNum);
		}
	}
	DATA_MANA_DUG("find data in flash sector=%d ,block=%d \r\n",g_DataReadStr.read_sectorIndex,g_DataReadStr.read_blockIndex);
	
	flashAddr = dataPartition->base_addr + g_DataReadStr.read_sectorIndex*FLASH_SECTOR_PRE_SIZE + g_DataReadStr.read_blockIndex*FLASH_RW_BLOCK_SIZE;

	read_flash(flashAddr, pbuf,FLASH_RW_BLOCK_SIZE);

	g_DataReadStr.read_ram_flag = STORE_MEDIA_FLASH;
	
	*dataLength = (pbuf[7]+1) * DATA_BLOCK_HEAD_SIZE;// kai.ter add get data length Old
	return 1;
}

/*
	if send data success   flg set 1 to erase data
	if send data fale	   flg set 0 to unlock ram data
*/
void erase_flash_gsensor_data(unsigned char flg)
{
	if(flg == 0)
	{
		set_gsensor_data_lock(g_DataReadStr.read_dataType,0);
		return;
	}
	
	flash_data_partition_t* dataPartition;
	data_block_t* pRamBlock = get_ram_block(g_DataReadStr.read_dataType);

	dataPartition = get_dataPartition(g_DataReadStr.read_dataType);
	if((dataPartition == NULL)||(pRamBlock == NULL))
	{
		return; 
	}
	
	if(g_DataReadStr.read_ram_flag == STORE_MEDIA_MEM)  //erase ram data
	{
		DATA_MANA_DUG("erase ram data = %d dataType\r\n",g_DataReadStr.read_dataType);
		ram_block_reset(pRamBlock);
		//when erase ram data,so unlock ram data!
		set_gsensor_data_lock(g_DataReadStr.read_dataType,0);
	}
	else{	//erase flash data
		DATA_MANA_DUG("erase flash data = %d dataType\r\n",g_DataReadStr.read_dataType);
		
		dataPartition->sectorEraseBitmaps[g_DataReadStr.read_sectorIndex].sector_maps |= 1<<g_DataReadStr.read_blockIndex;

		/*if sectorEraseBitmaps is full(0xFFFF),so erase this sector,and clear sectorDataBitmaps*/
		if((dataPartition->sectorEraseBitmaps[g_DataReadStr.read_sectorIndex].sector_maps == 0xFFFF)
		  ||(g_DataReadStr.read_blockIndex == 15))
		{
			DATA_MANA_DUG("read erase bit maps is full ,so erase this sector\r\n");
			flash_gsensor_data_partition_erase_sector(g_DataReadStr.read_dataType,g_DataReadStr.read_sectorIndex);
			dataPartition->sectorDataBitmaps[g_DataReadStr.read_sectorIndex].sector_maps = 0;
			dataPartition->sectorEraseBitmaps[g_DataReadStr.read_sectorIndex].sector_maps = 0;
			
			if((++dataPartition->readSectorNum) >= dataPartition->sector_num)
			{
				dataPartition->readSectorNum = 0;
			}
		}
		
		if(dataPartition->totalBlockDataNum > 0)
		{
			dataPartition->totalBlockDataNum -= 1;
		}
	
		if (dataPartition->totalBlockDataNum == 0)
		{
			dataPartition->partitionStatus = PARTITION_STATUS_EMPTY;
		}
		else{
			dataPartition->partitionStatus = PARTITION_STATUS_NO_FULL;			
		}
	}
	g_DataReadStr.read_dataType = 0;
}

#if 0
#if ((DEBUG_UART_EN) )
	//UINT16 K;
	if(dataType==DATA_TYPE_SPORT)
	{
      		 DbgPrintfbuff("sector_num_Sport:",strlen("sector_num_Sport:"));
    		 // for(K=0;K<256;K++)
     		DbgPrintf("%x ",dataPartition->sector_num);
     		DbgPrintfbuff("\r\n",2);
	}
	if(dataType==DATA_TYPE_SLEEP)
	{
		DbgPrintfbuff("sector_num_sleep:",strlen("sector_num_sleep:"));
    		 // for(K=0;K<256;K++)
     		DbgPrintf("%x ",dataPartition->sector_num);
     		DbgPrintfbuff("\r\n",2);

	}
#endif 
#endif
static void flash_gsensor_sport_sleep_data_partition_refresh(unsigned char dataType)
{
	flash_data_partition_t* dataPartition;
	int count = 0;
	unsigned int flashAddr=0;
	unsigned char buf[FLASH_RW_BLOCK_SIZE];
	unsigned short i = 0, j = 0;
    	data_block_t data;
	unsigned char num = 0,eraseFlg = 0;
	unsigned char testBuf[HEAD_PREVIOUS_HEAD_BYTE_NUM] ={0xff,0xff,0xff,0xff};
	
	dataPartition = get_dataPartition(dataType);
	if(dataPartition == NULL)
	{
		return ; 
	}
	
	sector_bitmap_t * pBitMaps = dataPartition->sectorDataBitmaps;
	sector_bitmap_t * pEraseBitMaps = dataPartition->sectorEraseBitmaps;
	
	memset((void*)pBitMaps,0,dataPartition->sector_num*sizeof(unsigned short));
	memset((void*)pEraseBitMaps,0,dataPartition->sector_num*sizeof(unsigned short));
	dataPartition->writeSectorNum = 0;
	dataPartition->readSectorNum = 0;
	for(i = 0; i < dataPartition->sector_num; i++)		
	{
		flashAddr = dataPartition->base_addr + i*FLASH_SECTOR_PRE_SIZE;
		num = 0;
		eraseFlg = 0;
		for(j = 0; j < FLASH_PRE_SECTOR_BLOCK_NUM; j++)//16
		{
			read_flash(flashAddr + j*FLASH_RW_BLOCK_SIZE, (unsigned char *)buf, FLASH_RW_BLOCK_SIZE); /*读256 Byte*/

            		data.head = (data_block_head_t *)buf;
            		if (!(memcmp(data.head->previousHead, g_DataHeadLabel, HEAD_PREVIOUS_HEAD_BYTE_NUM)))/*比较相等为0*/
			{
				pBitMaps->sector_maps |= 1<<j;		/*该Block有数据sector_maps Set 1*/
				count++;								/*有数据Block总数*/
				num++;								/*该Page有数据Block数*/
			}
			else if(!(memcmp(data.head->previousHead, testBuf, HEAD_PREVIOUS_HEAD_BYTE_NUM)))
			{
				eraseFlg++; 					/*包头=0XFF 0XFF 0XFF 0XFF   eraseFlg 该Page无数据Block数*/
			}
		}
		if(pBitMaps->sector_maps == 0)			/*该Sector无数据*/
		{
			//if this sector maps is 0,so erase this sector!
			if(eraseFlg < FLASH_PRE_SECTOR_BLOCK_NUM)   
				sector_erase_flash(flashAddr, 1);
			count -= num;
			
			
		}
		else
		{
			for(j = 0; j < (FLASH_PRE_SECTOR_BLOCK_NUM-1); j++)/*16-1*/
			{
				if(((pBitMaps->sector_maps & (1<<j)) == 0)&&((pBitMaps->sector_maps & (1<<(j+1))) != 0))
				{
					//if this sector maps is not continuous,so erase this sector!
					sector_erase_flash(flashAddr, 1);//Nick Remove
					count -= num;
					break;
				}
			}
		}
		pBitMaps++;
	}

	dataPartition->totalBlockDataNum = count;   /*have write block number 已经写的256byte 数*/
	if(count <= 0)
	{
		dataPartition->partitionStatus = PARTITION_STATUS_EMPTY;
	    //dataPartition->maxBlockDataNum = FLASH_PRE_SECTOR_BLOCK_NUM*dataPartition->sector_num;
	}
	else
	{
		if(count == FLASH_PRE_SECTOR_BLOCK_NUM*dataPartition->sector_num)
			dataPartition->partitionStatus = PARTITION_STATUS_FULL;
		else
			dataPartition->partitionStatus = PARTITION_STATUS_NO_FULL;
	}

	//confirm write position
	for(i = 0, j = 0; i < dataPartition->sector_num; i++)
	{
		//find the first write block end
		if(dataPartition->sectorDataBitmaps[i].sector_maps != 0)
		{
			j = 1;
			dataPartition->writeSectorNum = i;
		}
		else
		{
			if(j == 1)
			{
				break;
			}
		}
	}
	if(dataPartition->sectorDataBitmaps[dataPartition->writeSectorNum].sector_maps == 0xFFFF)
	{
		dataPartition->writeSectorNum++;
	}
	if(dataPartition->writeSectorNum >= dataPartition->sector_num)
	{
		dataPartition->writeSectorNum = 0;
	}
	//confirm read position
	for(i = 0, j = 0; i < dataPartition->sector_num; i++)
	{
		//find the last write block start
		if(j == 0)
		{
			if(dataPartition->sectorDataBitmaps[i].sector_maps != 0)
			{
				j = 1;
				dataPartition->readSectorNum = i;
			}
		}
		else
		{
			if(dataPartition->sectorDataBitmaps[i].sector_maps == 0)
			{
				j = 0;
			}
		}
	}
	
	DATA_MANA_DUG("R=%d W=%d\r\n",dataPartition->readSectorNum,dataPartition->writeSectorNum);
}


/*
static void flash_gsensor_histry_sport_data_partition_refresh(void)
{
	unsigned char buf[16];
	unsigned int addr;
	addr=DEV_GSENSOR_SPORT_Histry_DATA_Head_ADDR;
	read_flash(addr,buf,16);
	DATANumber=0;
	if (0 == memcmp(buf, g_DataHeadLabel, FLASH_ERASE_LEBAL_SIZE))
	{
		DATANumber=buf[7];
	}

}*/
static void flash_gsensor_data_partition_refresh(void)
{
	flash_gsensor_sport_sleep_data_partition_refresh(DATA_TYPE_SPORT);	//Nick Remove
	flash_gsensor_sport_sleep_data_partition_refresh(DATA_TYPE_SLEEP);      //Nick Remove
	//flash_gsensor_histry_sport_data_partition_refresh();/*历史运动数据累计刷新*/
}

static void read_flash_data_to_struct(unsigned int addr ,unsigned char *data ,unsigned short length)
{
	unsigned char buf[FLASH_RW_BLOCK_SIZE]={0};

	read_flash(addr,buf,FLASH_RW_BLOCK_SIZE);
	if(buf[0] == 0xAA)
	{
		DATA_MANA_DUG("=========read data is ok======== \r\n");
		length = length > FLASH_RW_BLOCK_SIZE ? FLASH_RW_BLOCK_SIZE:length;
		memcpy(data,buf,length);
	}
}

void read_infos_from_flash_first_page(void)
{
	unsigned int flashAddr;
	
	#if WALK_STEP_RECORD_ARITHMETIC	
	// read user info from block 1
	flashAddr = Usr_Infos_Data_Sector_START_ADDR + FLASH_RW_BLOCK_SIZE*0;   
	DATA_MANA_DUG("read user info from block 1\r\n");
	read_flash_data_to_struct(flashAddr,(unsigned char*)&g_user,sizeof(g_user));/*取取*/
	#endif
	
	#if FREE_NOTIC_ARITHMETIC
	// read free_notice info from block 5
	flashAddr = Usr_Infos_Data_Sector_START_ADDR + FLASH_RW_BLOCK_SIZE*1;
	DATA_MANA_DUG("read free_notice info from block 5\r\n");
	read_flash_data_to_struct(flashAddr,(unsigned char*)&free_notic_info,sizeof(free_notic_info));
	free_notic_info.count_time = 0;
	DATA_MANA_DUG("free_notic_info.free_enable = %d \r\n",free_notic_info.free_enable);
	DATA_MANA_DUG("free_notic_info.time = %d \r\n",free_notic_info.time);
	DATA_MANA_DUG("free_notic_info.start_time_hour = %d \r\n",free_notic_info.start_time_hour);
	DATA_MANA_DUG("free_notic_info.start_time_minute = %d \r\n",free_notic_info.start_time_minute);
	DATA_MANA_DUG("free_notic_info.end_time_hour = %d \r\n",free_notic_info.end_time_hour);
	DATA_MANA_DUG("free_notic_info.end_time_minute = %d \r\n",free_notic_info.end_time_minute);
	#endif
}

void write_infos_2_flash_first_page(void)
{
	//write entong to flash
	unsigned int flashAddr = Entong_char_Data_Sector_START_ADDR;	
	sector_erase_flash(flashAddr,1);	
    	write_flash(flashAddr,(unsigned char *)&g_erase_label ,FLASH_ERASE_LEBAL_SIZE);
	//write user info block1
	flashAddr = Usr_Infos_Data_Sector_START_ADDR;
	sector_erase_flash(flashAddr,1);	
    	write_flash(flashAddr,(unsigned char *)&g_user ,sizeof(g_user));

#if FREE_NOTIC_ARITHMETIC
	//write free_notice block2
	flashAddr = Usr_Infos_Data_Sector_START_ADDR + FLASH_RW_BLOCK_SIZE * 1;
    	write_flash(flashAddr,(unsigned char *)&free_notic_info ,sizeof(free_notic_info));
#endif

}

static void init_flash_erase(void)
{
	unsigned char buf[FLASH_RW_BLOCK_SIZE] = {0};
	unsigned int flashAddr = Entong_char_Data_Sector_START_ADDR;
	
	memcpy(g_erase_label.label, FLASH_ERASE_LABEL, FLASH_ERASE_LEBAL_SIZE);

	read_flash(flashAddr, buf,FLASH_RW_BLOCK_SIZE);
	if (0 != memcmp(buf, g_erase_label.label, FLASH_ERASE_LEBAL_SIZE))
    	{
        	DATA_MANA_DUG("first boot, erase all flash and write celink lablel to flash!\r\n");
        	sector_erase_flash(DEV_INPUT_DATA_START_SECTOR, DEV_FLASH_SECTOR_TOTAL_NUM);//erase all flash data
		memcpy(buf, g_erase_label.label, FLASH_ERASE_LEBAL_SIZE);
        	write_flash(flashAddr, buf,FLASH_RW_BLOCK_SIZE);
	}
}

void init_data_manger(void)
{
	DATA_MANA_DUG("enter init_data_manger \r\n");

	init_ram_pools();

	read_infos_from_flash_first_page();
	
	init_flash_erase();//Nick Remove
	
	flash_gsensor_data_partition_refresh();//Nick Remove

	//ke_timer_set(USR_AUTO_SEND_DATA_TO_APP_TIMER,1);
}

void flash_all_data_clear(void)
{
	unsigned int flashAddr = DEV_GSENSOR_DATA_MANGER_START_ADDR;
	//unsigned char sector_num = DEV_GSENSOR_SLEEP_DATA_SECTOR_NUM +
	//						   DEV_GSENSOR_SPORT_DATA_SECTOR_NUM+DEV_GSENSOR_SPORT_Histry_DATA_SECTOR_NUM;/*Nick*/
	unsigned char sector_num = DEV_GSENSOR_SPORT_DATA_SECTOR_NUM_ALL+DEV_GSENSOR_SLEEP_DATA_SECTOR_NUM;

	DATA_MANA_DUG("clear all data\r\n");
	
	/* 1.clear sport steps */
	#if WALK_STEP_RECORD_ARITHMETIC
	app_arthmetic_clear_steps(1);
	#endif
	
	/* 2.clear sleep data */
	#if SLEEP_RECORD_ARITHMETIC
	Init_SleepArithmetic();
	#endif
	
	/* 3.clear flash data */
	sector_erase_flash(flashAddr,sector_num);

	/* 4.clear ram data */	
	ram_block_reset(&g_RamSportBlock);
	ram_block_reset(&g_RamSleepBlock);

	/* 5.reset bitmap */
	flash_gsensor_data_partition_refresh();

}


	#if 0
		unsigned char k;
		//DbgPrintfbuff("Recordation:",strlen("Recordation:"));
    		// for(k=0;k<16;k++)
     		//DbgPrintf("%x ",Recordation[k]);
     		//DbgPrintfbuff("\r\n",2);

		DbgPrintf("walkSteps:");
    		for(k=0;k<4;k++)
     		 	DbgPrintf("%x ",g_sportInfoDMItem.walkSteps[k]);
     		 //DbgPrintfbuff(g_sportInfoDMItem.walkSteps,4);
     		DbgPrintfbuff("\r\n",2);
			
		DbgPrintf("DATANumber:%x \r\n",DATANumber);
		DbgPrintf("flashAddr:%x \r\n",flashAddr);
		DbgPrintf("buff:\r\n");
    		for(k=0;k<250;k++)
     			DbgPrintf("%x ",Buf[k]);
     		DbgPrintfbuff("\r\n",2);
     		//DbgPrintfbuff(Buf,256);
	#endif

#if 0
void SaveEveryDayHistySport_Data_info2_datamanger(void)
{
	data_block_t* pRamBlock = get_ram_block(DATA_TYPE_SPORT);
	//unsigned char Recordation[DEV_GSENSOR_SPORT_Histry_DATA_Size]={0};
	unsigned char headHistry[16]={0x1A, 0x2B, 0x3C, 0x4D,0x01,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
	
	unsigned char Buf[16]={0};

	unsigned int flashAddr = 0;

	
	/*系统设置的每天结束时间发生变化表示过了一天*/
	if(DATANumber>DATA_BLOCK_LOAD_SIZE)	/*255清除历史每天所有记录*/
	{
		DATANumber=0;
		sector_erase_flash(DEV_GSENSOR_SPORT_Histry_DATA_START_ADDR,DEV_GSENSOR_SPORT_Histry_DATA_SECTOR_NUM);
	}
		/*记录前天最终累计记录从第2个16BYTE开始*/
		flashAddr=DEV_GSENSOR_SPORT_Histry_DATA_Head_ADDR;/*包头*/
		read_flash(flashAddr,Buf,16);/*把记录包头读出来*/
		if (0 != memcmp(Buf, g_DataHeadLabel, HEAD_PREVIOUS_HEAD_BYTE_NUM))
		{	
			DATANumber=0;
			memcpy(Buf,headHistry,16);
		}
		etSpim1NorFlashSectorErase(flashAddr);
		Buf[7]++;/*记录增加*/
		DATANumber=Buf[7];
		write_flash(flashAddr, Buf, 16);/*记录包头*/
		//memcpy(Buf,(unsigned char*)&g_sportInfoDMItemSave,16); /*新数据写到FLASH*/
		memcpy(Buf,(unsigned char*)&g_sportInfoDMItem,16); /*新数据写到FLASH*/
		
		flashAddr = DEV_GSENSOR_SPORT_Histry_DATA_START_ADDR+(DATANumber-1)*16;
		write_flash(flashAddr,Buf,16);
		
		memset(&g_walkInfoStruct,0,sizeof(walk_info_t));
		g_walkInfoStruct.sportMode = SPORT_MODE_MAX;
		memset(&g_sportInfoDMItem,0,sizeof(sport_info_struct));
		memset(pRamBlock->head, 0, 16);
		memset(pRamBlock->pdata, 0, DATA_BLOCK_LOAD_SIZE);
		
		//app_arthmetic_clear_steps(0);

		/*擦除前天所有Flash记录*/
		sector_erase_flash(DEV_GSENSOR_SPORT_DATA_START_ADDR,DEV_GSENSOR_SPORT_DATA_SECTOR_NUM);
		flash_gsensor_sport_sleep_data_partition_refresh(DATA_TYPE_SPORT);		
}
#endif
#if 1//new
void SaveEveryDayHistySport_Data_info2_datamanger(void)
{

		data_block_t* pRamBlock = get_ram_block(DATA_TYPE_SPORT);
		//unsigned char Recordation[DEV_GSENSOR_SPORT_Histry_DATA_Size]={0};
		unsigned char headHistry[16]={0x1A, 0x2B, 0x3C, 0x4D,0x01,0x00,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
		
		unsigned char Buf[256]={0};
		unsigned char DATANumber=0;
	 // unsigned char Buf2[256]={0};
		unsigned int flashAddr = 0;

		flashAddr=DEV_GSENSOR_SPORT_Histry_DATA_START_ADDR;/*????????*/
		read_flash(flashAddr,Buf,256);
		
		if((Buf[0]==0x1A)&&(Buf[1]==0x2B)&&(Buf[2]==0x3C)&&(Buf[3]==0x4D))
		{	
			Buf[7]=Buf[7]+1;
			DATANumber=Buf[7];
		}
		else
		{			
		  DATANumber=1;
			memcpy(Buf,headHistry,16);
			 Buf[7]=DATANumber;
		}
		if(Buf[7]>=15)
		{
			Buf[7]=15;
			DATANumber=15;
			memcpy(&Buf[16],&Buf[32],256-32);
		}
			etSpim1NorFlashSectorErase(flashAddr);
			memcpy(&Buf[DATANumber*16],(unsigned char*)&g_sportInfoDMItem,16); /*?????FLASH*/			
			flashAddr = DEV_GSENSOR_SPORT_Histry_DATA_START_ADDR;
			write_flash(flashAddr,Buf,256);
		  
			memset(&g_walkInfoStruct,0,sizeof(walk_info_t));
			g_walkInfoStruct.sportMode = SPORT_MODE_MAX;
			memset(&g_sportInfoDMItem,0,sizeof(sport_info_struct));
			memset(pRamBlock->head, 0, 16);
			memset(pRamBlock->pdata, 0, DATA_BLOCK_LOAD_SIZE);
			
			//app_arthmetic_clear_steps(0);
	
			/*??????Flash??*/
			sector_erase_flash(DEV_GSENSOR_SPORT_DATA_START_ADDR,DEV_GSENSOR_SPORT_DATA_SECTOR_NUM);
			flash_gsensor_sport_sleep_data_partition_refresh(DATA_TYPE_SPORT);		
#if DEBUG_UART_EN    
				DbgPrintf("erase yesterday step record:\r\n");
#endif 			
	}
#endif
#endif

