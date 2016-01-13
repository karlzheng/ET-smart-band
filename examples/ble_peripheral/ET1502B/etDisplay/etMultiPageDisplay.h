#ifndef _ET_MULTI_PAGE_DISPLAY_H__
#define _ET_MULTI_PAGE_DISPLAY_H__

#include "etLcdDisplayApp.h"
#define ENABLE_TESTLCD_MULTIPAGE 0

struct lcd_page{

	/*unsigned char start_point;*/
	unsigned char end_point;
	unsigned char col;
	unsigned char line;

};

struct lcd_multi_page{

	unsigned int  total_len;
	unsigned char page_num;/*page number*/
	unsigned char curr_page;
	struct lcd_page  page[MAX_PAGE_NUM];
	unsigned char buff[MAX_DISPLAY_BUFF_SIZE];

};

extern struct lcd_multi_page  g_lcdMultiPage;



extern void display_page(unsigned char page_num);
extern void display_up_page(void);
extern void display_down_page(void);

extern void free_display_buff(void);


extern void lcd_display_handle(unsigned char *dat, unsigned short len);
extern void lcd_display_complete_handle(void);
extern void lcd_changeline_handle(void);
extern void lcd_clear_handle(void);
#if ENABLE_TESTLCD_MULTIPAGE
void test_lcd(void);
#endif

#endif

