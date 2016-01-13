#include "ET1502B.h"
#include "common_arithmetic.h"

void ShortSetTwoChar(char* pVal, short setVal)
{
	*pVal = (char)(setVal & 0xff);
	*(pVal+1) = (char)((setVal>>8) & 0xff);
}
void IntSetThreeChar(char* pVal, int setVal)
{
	*pVal = (char)(setVal & 0xff);
	*(pVal+1) = (char)((setVal>>8) & 0xff);
	*(pVal+2) = (char)((setVal>>16) & 0xff);
}

void IntSetFourChar(char* pVal, int setVal)
{
	*pVal = (char)(setVal & 0xff);
	*(pVal+1) = (char)((setVal>>8) & 0xff);
	*(pVal+2) = (char)((setVal>>16) & 0xff);
	*(pVal+3) = (char)((setVal>>24) & 0xff);
}

short TwoCharGetShort(char* pVal)
{
	short val = 0;
	val = *(pVal + 1);
	val = (val<<8) | *pVal;
	return val;
}

unsigned short TwoCharGetUShort(unsigned char* pVal)
{
	unsigned short val = 0;
	val = *(pVal + 1);
	val = (val<<8) | *pVal;
	return val;
}


int FourCharGetInt(char* pVal)
{
	int val = 0;
	val = *(pVal + 3);
	val = (val<<8) | *(pVal+2);
	val = (val << 8) | *(pVal+1);
	val = (val << 8) | *pVal;
	return val;
}

unsigned short sqrt_16(unsigned int value)	
{  
	unsigned short sqrtVal, i;	
	unsigned int tmp, ttp;	 
	if (value == 0) 			 
		return 0;  

	sqrtVal = 0;  

	tmp = (value >> 30);		 
	value <<= 2;  
	if (tmp > 1)		   
	{  
		sqrtVal ++; 			   
		tmp -= sqrtVal;  
	}  

	for (i=15; i>0; i--)	
	{  
		sqrtVal <<= 1;			   

		tmp <<= 2;	
		tmp += (value >> 30);	

		ttp = sqrtVal;	
		ttp = (ttp<<1)+1;  

		value <<= 2;  
		if (tmp >= ttp)    
		{  
			tmp -= ttp;  
			sqrtVal++;	
		}  

	}  

	return sqrtVal;  
} 




