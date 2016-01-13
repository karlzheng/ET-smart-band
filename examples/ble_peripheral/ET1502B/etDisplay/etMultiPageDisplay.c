#include <string.h>
#include <stdio.h>
#include "etUtf8ToUnicode.h"
#include "etMultiPageDisplay.h"
#include "et_debug.h"
#include "etMotor.h"
#include "btprotocol.h"
struct lcd_multi_page  g_lcdMultiPage = {0,  0,  0, {0,0,0},0};


static unsigned char  utf8Buff[7] = {0};
static unsigned char  utf8BuffLen1 = 0;
static unsigned char  utf8BuffLen2 = 0;
#if ENABLE_TESTLCD_MULTIPAGE
//const unsigned char test_lcd_code[]="?D?a¨¨???12o¨ª1¨²1???\n¨º?¨¦??¨²¨ºD??¨¦????¡ão¡ê?yo?¨°?¨ª¡§????¨®D?T1???T2\nB?¡ã511,hello engtongkejiyouxiangongsiyuanyongxintest01234567890abcdefghijklmnopqrstuvwxyz0011223344556677889900aabbccddeeffkevinyuantestcomplete!~";
#endif
void free_display_buff(void)
{
	g_lcdMultiPage.curr_page = 0;
	g_lcdMultiPage.page_num = 0;
	g_lcdMultiPage.total_len = 0;
	utf8BuffLen1 = 0;
	utf8BuffLen2 = 0;
	memset(g_lcdMultiPage.page, 0x00, sizeof(g_lcdMultiPage.page));
	memset(g_lcdMultiPage.buff, 0x00, MAX_DISPLAY_BUFF_SIZE);
}

void copy_display_data_to_buff(unsigned char *buff, unsigned short len)
{

	unsigned short  len_t = 0;
//#if DEBUG_UART_EN
//     DbgPrintf("copy_display_data_to_buff len=%d\r\n",len);
//     for(unsigned char i=0;i<len;i++)
//     {
//      DbgPrintf("%x ",buff[i]);
//     }
//     DbgPrintf("\r\n");
//#endif 
	if (len == 0)
		return;

	if (g_lcdMultiPage.total_len >= MAX_DISPLAY_BUFF_SIZE)
		return;

	if (g_lcdMultiPage.total_len == 0)
	{
		free_display_buff();
	}

	for(len_t = 0; len_t < len; len_t++)
	{
		if (g_lcdMultiPage.total_len < MAX_DISPLAY_BUFF_SIZE)
		{
			g_lcdMultiPage.buff[g_lcdMultiPage.total_len] = buff[len_t];
			g_lcdMultiPage.total_len++;
		}
	}

}




void analyse_display_data(void)
{
	unsigned int   len_t = 0;
	unsigned char  line_num = 0;
	unsigned char  col_num = 0;
	unsigned char  *buff = g_lcdMultiPage.buff;
	unsigned int   total_len = g_lcdMultiPage.total_len;

	g_lcdMultiPage.page_num = 0;
	g_lcdMultiPage.curr_page = 0;
	memset(g_lcdMultiPage.page, 0x00, sizeof(g_lcdMultiPage.page));

	if (buff == NULL)
	{
		g_lcdMultiPage.page_num = 0;
		return;
	}
	while(total_len)
	{
		if ( ((buff[len_t] <= 0x7F) && (buff[len_t] >= 0x20)) || (buff[len_t] == 0x01)  )/*ASCII jerry 20140707*/
		{

			total_len--;
			len_t++;
			line_num++;
			if (line_num > MAX_LINE_ASCII)/*change line*/
			{
				col_num++;
				line_num = 1;
				if (col_num >= MAX_LINE)/*change page*/
				{
					if ((g_lcdMultiPage.page_num + 1) >= MAX_PAGE_NUM)
						return;
					g_lcdMultiPage.page_num++;
					col_num = 0;
				}
			}
			g_lcdMultiPage.page[g_lcdMultiPage.page_num].end_point++;

		}
		else if (buff[len_t] == 0x0D)
		{
			g_lcdMultiPage.page[g_lcdMultiPage.page_num].end_point++;
			len_t++;
			total_len--;
		}
		else if (buff[len_t] == 0x0A)
		{
			g_lcdMultiPage.page[g_lcdMultiPage.page_num].end_point++;
			total_len--;
			len_t++;
			col_num++;
			line_num = 0;
			if (col_num >= MAX_LINE)/**/
			{
				if ((g_lcdMultiPage.page_num + 1) >= MAX_PAGE_NUM)
					return;
				if (total_len)
					g_lcdMultiPage.page_num++;
				col_num = 0;
			}
		}
		else if ((buff[len_t] >= 0x81) && (buff[len_t]  <= 0xFE))
		{
			total_len--;
			len_t++;

			if (total_len > 0)
			{
				if ((buff[len_t] >=0x40) && (buff[len_t] <= 0xFE))
				{
					line_num += 2;
					total_len--;
					len_t++;
					if (line_num > MAX_LINE_ASCII)/*change line*/
					{
						col_num++;
						line_num = 2;
						if (col_num >=  MAX_LINE)/**/
						{
							if ((g_lcdMultiPage.page_num + 1) >= MAX_PAGE_NUM)
								return;
							g_lcdMultiPage.page_num++;
							col_num = 0;
						}
					}
					g_lcdMultiPage.page[g_lcdMultiPage.page_num].end_point += 2;
				}
				else if ((buff[len_t] >=0x30) && (buff[len_t] <= 0x39))
				{
					len_t += 3;

					if (total_len >= 3)
					{
						line_num += 2;
						total_len -= 3;
						if (line_num > MAX_LINE_ASCII)/*change line*/
						{
							col_num++;
							line_num = 2;
							if (col_num >=  MAX_LINE)/**/
							{
								if ((g_lcdMultiPage.page_num + 1) >= MAX_PAGE_NUM)
									return;
								g_lcdMultiPage.page_num++;
								col_num = 0;
							}
						}

						g_lcdMultiPage.page[g_lcdMultiPage.page_num].end_point += 4;
					}
					else
						return;
				}
				else
					return;

			}
			else
				return;
		}
		else
			return;

	}
	return;
}




static unsigned int get_display_addr_off(unsigned char page_num)
{
	unsigned int   len_off = 0;

	while(page_num)
	{
		page_num--;
		len_off +=  g_lcdMultiPage.page[page_num].end_point;
	}

	return len_off;
}

static void get_page_display_data(unsigned char *buff, unsigned int start_point,  unsigned short total_len)
{
	if (total_len > MAX_DISPLAY_BUFF_SIZE)
		total_len = MAX_DISPLAY_BUFF_SIZE;

	if (start_point <= MAX_DISPLAY_BUFF_SIZE)
	{
		if ((start_point+total_len) <= MAX_DISPLAY_BUFF_SIZE)
		{
			memcpy(buff, g_lcdMultiPage.buff+start_point, total_len);
		}
		else
		{
			memcpy(buff, g_lcdMultiPage.buff+start_point, MAX_DISPLAY_BUFF_SIZE-start_point);
		}
	}
}


void display_page(unsigned char page_num)
{
	unsigned int   addr_off = 0;
	unsigned char  display_buff[250];


	if (g_lcdMultiPage.page_num == 0)
		return;

	if (page_num >= g_lcdMultiPage.page_num)
		return;

	addr_off = get_display_addr_off(page_num);
    lcd_power_control(LCD_OPEN);
    lcd_power_control(LCD_OPEN);
	get_page_display_data(display_buff, addr_off,  g_lcdMultiPage.page[page_num].end_point);
    display_check_to_clear_Screen();

	display_gb18030_string(display_buff, g_lcdMultiPage.page[page_num].end_point, 0, 0);
	Set_lastDisplayTag(DISP_TAG_COS_DATA);
  

	return;
}

void display_down_page(void)
{
	if (g_lcdMultiPage.page_num <= 1)
		return;

	if (g_lcdMultiPage.curr_page >= (g_lcdMultiPage.page_num - 1))
		return;

	g_lcdMultiPage.curr_page++;

	//clear_secterScreen(0,4,0,LCD_LOG_COLUME_START);////clear_AllScreen();

	display_page(g_lcdMultiPage.curr_page);

	return;
}


void display_up_page(void)
{
	if (g_lcdMultiPage.page_num <= 1)
		return;

	if (g_lcdMultiPage.curr_page == 0)
		return;

	if (g_lcdMultiPage.curr_page > 0)
		g_lcdMultiPage.curr_page--;

    //clear_secterScreen(0,4,0,LCD_LOG_COLUME_START);//clear_AllScreen();
	display_page(g_lcdMultiPage.curr_page);


	return;
}

//========================================================================================================================
void lcd_display_handle(unsigned char *dat, unsigned short len)
{
	unsigned short  display_buf_len = len;
	unsigned char   display_buff[MAX_DISPLAY_BUFF_SIZE/2];
	unsigned short  tmpLen = 0;

	//display_gb18030_string(dat, len, 0, 0);
	memset(display_buff, 0x00, MAX_DISPLAY_BUFF_SIZE/2);
	if (utf8BuffLen2)
	{
		unsigned char slen = utf8BuffLen1-utf8BuffLen2;
		if ( (utf8BuffLen1-utf8BuffLen2) > display_buf_len)
		{
			memcpy(utf8Buff+utf8BuffLen2, &dat[0], display_buf_len);
			utf8BuffLen2 += display_buf_len;
			return;
		}
		memcpy(utf8Buff+utf8BuffLen2, &dat[0], utf8BuffLen1-utf8BuffLen2);

		display_buf_len = utf8_to_gbk(utf8Buff, utf8BuffLen1, display_buff, utf8Buff, &utf8BuffLen1, &utf8BuffLen2);
		copy_display_data_to_buff( display_buff, display_buf_len);
		display_buf_len = len - slen;
		tmpLen = slen;
		memset(utf8Buff, 0x00, sizeof(utf8Buff));
		utf8BuffLen1 = 0;
		utf8BuffLen2 = 0;
	}
	memset(display_buff, 0x00, MAX_DISPLAY_BUFF_SIZE/2);
	display_buf_len = utf8_to_gbk(&dat[0+ tmpLen], display_buf_len, display_buff, utf8Buff, &utf8BuffLen1, &utf8BuffLen2);


	LcdDispPara.lcd_page = 0;
	LcdDispPara.lcd_col = 0;
	copy_display_data_to_buff( display_buff, display_buf_len);
	//display_gb18030_string(display_buff, display_buf_len, 0, 0);
	/*************************************************************************************/
}

void lcd_display_complete_handle(void)
{
	analyse_display_data();
	g_lcdMultiPage.page_num++;
	display_page(0);
}

void lcd_changeline_handle(void)
{
	unsigned char  line_feed[] = "\r\n";
	copy_display_data_to_buff( line_feed, 2);
}


void lcd_clear_handle(void)
{
	//clear_secterScreen(0,4,0,LCD_LOG_COLUME_START);//clear_AllScreen();
  MotorCtr(0,0);//close Motor
	display_check_to_clear_Screen();
	free_display_buff();
	lcd_power_control(LCD_CLOSE);
	Set_lastDisplayTag(DISP_TAG_NULL);
	
}

#if ENABLE_TESTLCD_MULTIPAGE
void test_lcd(void)
{ 
/*  unsigned char data1[]={0xE4,0xBB,0x98,0xE6,0xAC,0xBE};
  unsigned char data2[]={0xE8,0xB4,0xA6,0xE5,0x8F,0xB7,0xEF,0xBC,0x9A};
  unsigned char data3[]={0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x30,0x31,0x32,0x33,0x34,0x35,0x36};
  //unsigned char data4[]={0x0a};
  unsigned char data5[]={0xE6,0x94,0xB6,0xE6,0xAC,0xBE,0xE4,0xBA};
  unsigned char data6[]={0xBA,0xE5,0x90,0x8D,0xE7,0xA7,0xB0,0xEF,0xBC,0x9A};
  unsigned char data7[]={0xE5,0xBC,0xA0,0xE4,0xB8,0x89}; 
  //unsigned char data8[]={0x0a};
  unsigned char data9[]={0xE6,0x94,0xB6,0xE6,0xAC,0xBE,0xE4,0xBA,0xBA,0xE8,0xB4,0xA6,0xE5,0x8F,0xB7,0xEF,0xBC};   
  unsigned char data10[]={0x9A}; 
  unsigned char data11[]={0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x30,0x31,0x32,0x33,0x34,0x35,0x36};
  //unsigned char data12[]={0x0a};
  unsigned char data13[]={0xE9,0x87,0x91,0xE9,0xA2,0x9D,0xEF,0xBC,0x9A};
  unsigned char data14[]={0x31,0x32,0x33,0x2E,0x32,0x33};
  //unsigned char data15[]={0x0a};
  free_display_buff();
  lcd_display_handle(data1,sizeof(data1));
  lcd_display_handle(data2,sizeof(data2));
  lcd_display_handle(data3,sizeof(data3));
  //lcd_display_handle(data4,sizeof(data4));
  lcd_changeline_handle();
  lcd_display_handle(data5,sizeof(data5));
  lcd_display_handle(data6,sizeof(data6));
  lcd_display_handle(data7,sizeof(data7));
  //lcd_display_handle(data8,sizeof(data8));
  lcd_changeline_handle();
  lcd_display_handle(data9,sizeof(data9));
  lcd_display_handle(data10,sizeof(data10));
  lcd_display_handle(data11,sizeof(data11));
  //lcd_display_handle(data12,sizeof(data12));
  lcd_changeline_handle();
  lcd_display_handle(data13,sizeof(data13));
  lcd_display_handle(data14,sizeof(data14));
  //lcd_display_handle(data15,sizeof(data15));
  lcd_changeline_handle();

  
  clear_AllScreen();
  //copy_display_data_to_buff((unsigned char *)test_lcd_code,sizeof(test_lcd_code));
  lcd_display_complete_handle();
*/
}
#endif


#if 0
void display_utf8_menment(void)
{
	unsigned char  buff[] = "\xE5\x89\x8D\xE6\xB5\xB7\xE9\x93\xB6\xE9\x80\x9A";//{0x4d, 0x52, 0x77,  0x6d};
	lcd_display_handle(buff, sizeof(buff));
	lcd_display_complete_handle();
}
#endif
//========================================================================================================================
//====================================================================================================================
