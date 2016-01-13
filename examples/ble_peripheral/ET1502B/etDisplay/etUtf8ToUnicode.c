/*
 * Utf8ToUnicode.c
 *
 *  Created on: 2014-11-21
 *      Author: Jerry
 */

#include "ET1502B.h"
#include "etFontApp.h"
#include "etLcdDisplayApp.h"
#include "GT24L16M1Y20.h"
#include "et_debug.h"
static unsigned char Utf8TextBytes (unsigned char *mstr)
{
	unsigned char  textbytes = 0;

	if (mstr == 0)
	{
		return 0;               /*no valid character in mstr.  */
	}
	if (((*mstr) & 0x80) == 0)
	{
		textbytes = 1;          /*0xxxxxxx, normal ASCII code */
	}
	else if ((((*mstr) & 0xe0) == 0xc0) )
	{
		if (((*mstr) & 0xfe) == 0xc0) /*1100,000x is illegal data,*/
		{
			textbytes = 0;
		}
		else
		{
			textbytes = 2;      /*110xxxxx 10xxxxxx*/
		}
	}
	else if (((*mstr) & 0xf0) == 0xe0)
	{

		textbytes = 3;      /*1110xxxx 10xxxxxx 10xxxxxx*/
	}
	else if ((((*mstr) & 0xf8) == 0xf0) )
	{

		textbytes = 4;      /* 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx */

	}
	else if ((((*mstr) & 0xfc) == 0xf8) )
	{

		textbytes = 5;      /*111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx */

	}
	else if ((((*mstr) & 0xfe) == 0xfc) )
	{
		textbytes = 6;      /*1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx*/
	}
	else
	{
		textbytes = 0;         /*illegal formal, return 0 */
	}

	return textbytes;
}

#if(ENABLE_GT24L16M1Y_UT==0)
static unsigned long read_gbk_code_addr_by_unicode(unsigned char  uncode_h, unsigned char uncode_l)
{
	unsigned long  part_addr = 0;
	unsigned long  addr = 0;
	unsigned short   uncode = ((unsigned short)uncode_h<<8) + (unsigned short)uncode_l;


	if((uncode<=0x0451) && (uncode>=0x00a0))
	{
		part_addr = 0;
		addr = part_addr + (uncode-0x00a0)*2;
	}

	if((uncode<=0x2642) && (uncode>=0x2010))
	{
		part_addr = 1892;
		addr = part_addr+(uncode-0x2010)*2;
	}

	if((uncode<=0x33d5) && (uncode>=0x3000))
	{
		part_addr = 5066;
		addr = part_addr+(uncode-0x3000)*2;
	}

	if((uncode<=0x9fa5) && (uncode>=0x4e00))
	{
		part_addr = 7030;
		addr = part_addr+(uncode-0x4e00)*2;
	}
	if((uncode<=0xfe6b) && (uncode>=0xfe30))
	{
		part_addr = 48834;
		addr = part_addr+(uncode-0xfe30)*2;
	}
	if((uncode<=0xff5e) && (uncode>=0xff01))
	{
		part_addr = 48954;
		addr = part_addr+(uncode-0xff01)*2;
	}
	if((uncode<=0xffe5) && (uncode>=0xffe0))
	{
		part_addr = 49142;
		addr = part_addr+(uncode-0xffe0)*2;
	}

	if((uncode<=0xFA29) && (uncode>=0XF92C))
	{
		part_addr = 49312;
		addr = part_addr+(uncode-0XF92C)*2;
	}
	if((uncode<=0XE864) && (uncode>=0XE816))
	{
		part_addr = 49820;
		addr = part_addr+(uncode-0XE816)*2;
	}
	if((uncode<=0X2ECA) && (uncode>=0X2E81))
	{
		part_addr = 49978;
		addr = part_addr+(uncode-0X2E81)*2;
	}
	if((uncode<=0X49B7) && (uncode>=0X4947))
	{
		part_addr = 50126;
		addr = part_addr+(uncode-0X4947)*2;
	}
	if((uncode<=0X4DAE) && (uncode>=0X4C77))
	{
		part_addr = 50352;
		addr = part_addr+(uncode-0X4C77)*2;
	}
	if((uncode<=0X3CE0) && (uncode>=0X3447))
	{
		part_addr = 50976;
		addr = part_addr+(uncode-0X3447)*2;
	}
	if((uncode<=0X478D) && (uncode>=0X4056))
	{
		part_addr = 55380;
		addr = part_addr+(uncode-0X4056)*2;
	}
	return (addr + UNICODE_TO_GBK_BASE_ADDR);
}
#endif



static void read_gbk_code_by_unicode(unsigned char uncode_h, unsigned char uncode_l, unsigned char *gbkcode)
{
#if(ENABLE_GT24L16M1Y_UT==0)
	unsigned long  addr = 0;
//	unsigned short int temp_gbk=0;

	addr = read_gbk_code_addr_by_unicode(uncode_h, uncode_l);

//	set_display_flg(DISP_START);
	read_font_data(addr, gbkcode, 2);
  //temp_gbk=*(unsigned short int *)gbkcode;
	//read_font_data(addr, (unsigned char *)&temp_gbk, 2);
  //*gbkcode=temp_gbk>>8 & 0xff;
	//*(gbkcode+1)=temp_gbk & 0xff; 
#if DEBUG_UART_EN
   //  DbgPrintf("unicode to GBK:%x,%x\r\n",addr,temp_gbk);

#endif

//	set_display_flg(DISP_END);
#else
	//U2G(addr,gbkcode);
	unsigned  int temp_unicod=0;
	unsigned short int temp_gbk=0;
	temp_unicod=uncode_h<<8 | uncode_l; 
	//*gbkcode=U2G(temp_unicod,NULL);
	temp_gbk=U2G(temp_unicod,NULL);
  *gbkcode=temp_gbk>>8 & 0xff;
	*(gbkcode+1)=temp_gbk & 0xff;
#if DEBUG_UART_EN
    // DbgPrintf("unicode to GBK:%x,%x\r\n",temp_unicod,temp_gbk);
#endif 

#endif

}

static void get_gbk_code(unsigned short unicode, unsigned char *gbkcode)
{
	unsigned char  uncode_h = 0, uncode_l = 0;

	uncode_h = (unsigned char)(unicode>>8)&0xff;
	uncode_l = (unsigned char)(unicode)&0xff;
	read_gbk_code_by_unicode(uncode_h, uncode_l, gbkcode);
}

unsigned short utf8_to_gbk(unsigned char *utf8Str, unsigned short utf8StrLen, unsigned char *gbkStr, unsigned char *utf8buf, unsigned char *utf8len1, unsigned char *utf8len2)
{
	unsigned char  i = 0;
	unsigned char  zen_han=0;
	unsigned short   len_dest = 0;
	unsigned short  unStr = 0;
	//unsigned char  *p_src = utf8Str;
	unsigned char  utf8_len = 0;

	
	len_dest = 0;
	//p_src = (unsigned char *) (utf8Str);

	
	//display_gb18030_string(utf8Str, utf8StrLen, 0, 0);
	/* limit the total bytes of the *mstr */
	if (utf8StrLen <= 0)
	{
		return (0);
	}

	for (; (utf8Str[utf8_len] != 0x00) && (utf8_len < utf8StrLen);)
	{
		zen_han = Utf8TextBytes ((unsigned char *) &utf8Str[utf8_len]);
		if ((utf8_len+zen_han) > utf8StrLen)/*utf-8*/
		{
			unsigned char i = 0;

			*utf8len1 = zen_han;
			*utf8len2 = utf8StrLen-utf8_len;
			for (i = 0; i < *utf8len2; i++)
			{
				utf8buf[i] = utf8Str[utf8_len+i];
				//p_src++;
			}
			return len_dest;
		}

		if (zen_han == 1)       /*ASCII, just add 0x00 at beginning  */
		{
 			*gbkStr =  utf8Str[utf8_len];
			gbkStr++;
			//p_src++;
			utf8_len++;
			len_dest += 1;
		}
		else if (zen_han == 2)
		{
			unStr = ((((utf8Str[utf8_len]>> 2) & (unsigned char) 0x07)) & 0x00ff) << 8;
			unStr |= (((utf8Str[utf8_len] << 6) | ((utf8Str[utf8_len+1]) & (unsigned char) 0x3f))) & 0x00ff;

			get_gbk_code(unStr, gbkStr);

			gbkStr += 2;

			//p_src += 2;
			utf8_len += 2;
			len_dest += 2;
		}
		else if (zen_han == 3)
		{
			unStr = ((((utf8Str[utf8_len]) << 4) | (((utf8Str[utf8_len+1]) >> 2) & 0x0f)) & 0x00ff) << 8;

			unStr |= ((((utf8Str[utf8_len+1]) << 6) | ((utf8Str[utf8_len+2]) & (unsigned char) 0x3f))) & 0x00ff;

			get_gbk_code(unStr, gbkStr);
			gbkStr += 2;

			//p_src += 3;
			utf8_len += 3;
			len_dest += 2;
		}
		else if (zen_han == 4)
		{
			unStr = (utf8Str[utf8_len]) & 0x7;
			for (i=1; i<4;i++)
			{
				unStr <<= 6;
				unStr = unStr | ((utf8Str[utf8_len+i]) & 0x3f);
			}

			get_gbk_code(unStr, gbkStr);
			gbkStr += 2;

			//p_src += 4;
			utf8_len += 4;
			len_dest += 2;
		}
		else if (zen_han == 5)
		{
			unStr = (utf8Str[utf8_len]) & 0x3;
			for (i=1; i<5;i++)
			{
				unStr <<= 6;
				unStr = unStr | ((utf8Str[utf8_len+i]) & 0x3f);
			}

			unStr++;  get_gbk_code(unStr, gbkStr);
			gbkStr += 2;

			//p_src += 5;
			utf8_len += 5;
			len_dest += 2;
		}
		else if (zen_han == 6)
		{
			unStr = (utf8Str[utf8_len]) & 0x1;
			for (i=1; i<6;i++)
			{
				unStr <<= 6;
				unStr = unStr | ((utf8Str[utf8_len+i]) & 0x3f);
			}

			get_gbk_code(unStr, gbkStr);
			gbkStr += 2;

			//p_src += 6;
			utf8_len += 6;
			len_dest += 2;
		}
		else                    /*treated as illegal character, search the next character */
		{
			//p_src++;
			utf8_len++;
		}
	}
	//display_gb18030_string(gbkStr, utf8StrLen, 0, 0);
	return (len_dest);
}

