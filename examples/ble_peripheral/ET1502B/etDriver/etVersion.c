#include "etVersion.h"
#include "et_define.h"
#include "btprotocol.h"
#include "string.h"
#include "et_flash_offset.h"
#include "etSpi.h"
const char* cosversion="1.00.00";
//const char* HWversion="H9.06-1.06";//"HR-1.00.03";
const char* HWversion2="2.0-1.5";//"HR-1.00.03";
const char* BTversion="1.00.05";//SW version
void Get_KeyVersion(unsigned char * prt,unsigned char *str_length)
{
#if(ENABLE_PROTOCOL)
	UINT8 temp=0,temp2=0;
  unsigned char dis_data[32];
	 prt[temp++]=FLAG_FIRMWARE_VERSION;	
   prt[temp++]=strlen(BTversion);	
	 for(temp2=0;temp2<strlen(BTversion);temp2++)
		prt[temp++]=BTversion[temp2];
	
	 prt[temp++]=FLAG_HW_VERSION;	 
   prt[temp++]=strlen(HWversion2);
	 for(temp2=0;temp2<strlen(HWversion2);temp2++)
		prt[temp++]=HWversion2[temp2];
	
	 
	 prt[temp++]=FLAG_COS_VERSION;
   prt[temp++]=strlen(cosversion);
	 
   etSpim1ReadNorFlashStdMode(KEYID_START_ADDR+COS_VERSION_TAG1_ADDR_OFFSET,dis_data,32);	
	 if((dis_data[0]==MAC_VALID_TAG1) && (dis_data[1]==MAC_VALID_TAG2))
	 {
		 	 for(temp2=0;temp2<dis_data[2];temp2++)
			 {
			  if(temp2<32)
			  {
				 prt[temp++]=dis_data[temp2+3];
			  }
			 }
	 }
	 else
	 {
		 for(temp2=0;temp2<strlen(cosversion);temp2++)
		 {
			prt[temp++]=cosversion[temp2];
		 }

	 }
	 *str_length=temp;
#endif
}
void Get_HWVersion(unsigned char * prt,unsigned char *str_length)
{
#if(ENABLE_PROTOCOL)
	 UINT8 temp=0;
	 *str_length=strlen(HWversion2);
	 for(temp=0;temp<strlen(HWversion2);temp++)
		prt[temp]=HWversion2[temp];

#endif
}
void Get_COSVersion(unsigned char * prt,unsigned char *str_length)
{
#if(ENABLE_PROTOCOL)
	 UINT8 temp=0;
	 *str_length=strlen(cosversion);
	 for(temp=0;temp<strlen(cosversion);temp++)
		prt[temp]=cosversion[temp];

#endif
}
