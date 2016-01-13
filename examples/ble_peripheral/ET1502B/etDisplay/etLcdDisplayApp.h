#ifndef _ET_LCD_DISPLAY_APP__
#define _ET_LCD_DISPLAY_APP__

#include "et_define.h"
#include "etWorkMod.h"
#include "ET1502B.h"

#define BT_LOGO_FLASH_ENABLE   1
#define BT_LOGO_FLASH_DISABLE  0

#define USB_LOGO_DISP_ENABLE    1
#define USB_LOGO_DISP_DISABLE   0


#define LCD_MATRIX_HEIGHT              32//64
#define LCD_MATRIX_WIDTH               128
#define LCD_LAST_PAGE_NUM              3

#define LCD_LOG_WIDTH                  0//16//0//16


#define ASCII_HEIGHT                   16
#define ASCII_WIDTH                    8

#define HANZI_HEIGHT                   16
#define HANZI_WIDTH                    16

#define MAX_LINE_ASCII                 16//14  //((LCD_MATRIX_WIDTH - COLUME_OFFSET - LCD_LOG_WIDTH)/ASCII_WIDTH)
#define MAX_PAGE_NUM                   18//5   //8page
#define MAX_DISPLAY_BUFF_SIZE          300//300
#define MAX_LINE                       ( LCD_MATRIX_HEIGHT/ASCII_HEIGHT)



#define MAX_PAGE_BYTES                 64
#define LCD_LOG_COLUME_START           (LCD_MATRIX_WIDTH - LCD_LOG_WIDTH)
#define MAX_NUM_PER_COLUME             (LCD_LOG_COLUME_START/ASCII_WIDTH)

#define COLUME_OFFSET 0

#define DYNAMIC_DISPLAY_LOGO_BY_TIMER   0
/******************************************************************************************/
#define DISP_LOGO_DELAY 5 //second
#define DISP_RSSI_DELAY 30 //second

#define DISP_MODE_DELAY 1 //second
#define DISP_KEYID_DELAY 30 //second
#define DISP_INCOMING_LOGO_DELAY 30 //second
/***********************************************************************************************/
#define LCD_ADDRESS 0x78
#define LCD_CMD_REG 0x00
#define LCD_DATA_REG 0x40
#define LCD_OPEN 0x01
#define LCD_CLOSE 0x00
#define DISP_TIME_RIGHTNOW 0
#define DISP_TIME_LATER 1
/***************************************************************************************************/
#define LCD_RSSI_GET_FAIL 1
#define LCD_RSSI_NOT_CONNECTED 2
#define LCD_RSSI_GET_SUCCESS 0
/*****************************************test define**********************************************************/
#define ENALBE_TEST_CHARGING 0
/***************************************************************************************************/
#define COS_STA_COUNTER 8
/***************************************************************************************************/
#define PAGE_SET 0
#define PAGE_CHANGE 1
/***************************************************************************************************/
#define DEFAULT_CLOCK_WIDTH 12
#define DEFAULT_STEP_WIDTH 16
/***************************************************************************************************/


typedef struct {
unsigned LCD_display_last_tag:8;
unsigned LCD_display_keymode_delay:8;
unsigned LCD_disp_Corrent:8;
unsigned LCD_disp_distance_count:8;

unsigned LCD_disp_MainMenu:8;
unsigned LCD_disp_SlaveMenu:8;
unsigned LCD_disp_PassWd_count:8;
unsigned LCD_disp_Logo:2;
unsigned LCD_disp_SetpLogoFromPowerOff:2;
unsigned LCD_disp_LCD_is_close:4;
}disp_stru;
extern disp_stru disp_str;

typedef struct{
signed char LCD_disp_rssi_value;
unsigned char LCD_disp_Rssi_flag;
unsigned char LCD_disp_Rssi_data_ready;

}disp_stru2;
extern disp_stru2 disp_str_rssi;

/*********************************************display tag*********************************************/
typedef enum
{
	DISP_TAG_NULL=0x00,
	//DISP_TAG_POWERON_LOGO,
	DISP_TAG_PAIRING,
	DISP_TAG_CONNECTED,
	DISP_TAG_DISCONNECTED,
	DISP_TAG_AG_DATA,
	DISP_TAG_INCOMING_CALL,
	DISP_TAG_STEP_NUM,//===MAIN MENU==
	DISP_TAG_DISTANCE,
	DISP_TAG_ENERGE,
	DISP_TAG_SLEEP_STATUE,
	DISP_TAG_SLEEP_TIME,
	DISP_TAG_ID,
	DISP_TAG_SYSTIME,
	DISP_TAG_VERSION,
	DISP_TAG_RSSI,//0x0c
	DISP_TAG_STATUS,
	/*//DISP_TAG_UPDATED,	
	//DISP_TAG_STEP_NUM_SLAVE,//===SLAVE MENU==	
	//DISP_TAG_DISTANCE_SLAVE,	
	//DISP_TAG_ENERGE_SLAVE,
	//DISP_TAG_SYSTIME_SLAVE,
	//DISP_TAG_VERSION_SLAVE,
	*/
	DISP_TAG_RSSI_SLAVE,//0x0e
	//DISP_TAG_UPDATED_SLAVE,//MENU END
	//DISP_TAG_POWER_OFF,
	//DISP_TAG_OVERTIME_TO_POWEROFF,
	//DISP_LOGO_FROM_POWEROFF,
	DISP_TAG_AG_NAME,	
	DISP_TAG_PASSKEY,
	DISP_TAG_PAIR_OK,
	DISP_TAG_QR_CODE,//0x13
	DISP_TAG_COS_DATA,
	DISP_TAG_KEYMODE,	
	//DISP_TAG_GER_RSSI_FAIL,
	//DISP_TAG_GER_RSSI_SUCESS,
	DISP_TAG_CHARING,
	DISP_TAG_BT_BATT_STATUS,
	DISP_TAG_IDLE_LOGO,
	DISP_TAG_ALARM,
	DISP_TAG_TEST_LCD,
	DISP_TAG_MAX
}LCD_DISP_TAG;

typedef enum
{
  KeyBoard_Down_2_DISP_NULL=0,
  KeyBoard_Down_2_DISP_IDLE,
  KeyBoard_Down_2_DISP_STEP_NUM,
  KeyBoard_Down_2_DISP_DISTANCE,
  KeyBoard_Down_2_DISP_ENERGY,
  KeyBoard_Down_2_DISP_SLEEP_STATUS,
  KeyBoard_Down_2_DISP_SLEEP_TIME,
  KeyBoard_Down_2_DISP_ID,
  KeyBoard_Down_2_DISP_SYS_TIME,
  KeyBoard_Down_2_DISP_RSSI,
  KeyBoard_Down_2_DISP_VERSION,
  //KeyBoard_Down_2_DISP_STATUS,
  //KeyBoard_Down_2_DISP_UPDATED,  
  KeyBoard_Down_2_DISP_MAX
}KeyBoard_Down_2_Disp_MainMenu;
typedef enum
{
  KeyBoard_Down_2_DISP_Slave_NULL=0,
  KeyBoard_Down_2_DISP_Slave_UPDATED,
  KeyBoard_Down_2_DISP_Slave_MAX
}KeyBoard_Down_2_Disp_SlaveMenu;
//#define IIC_RES_LOW    nrf_gpio_pin_clear(LCD_RES_PIN)
//#define IIC_RES_HIGH   nrf_gpio_pin_set(LCD_RES_PIN)
//#define IIC_RES_LOW     do {  NRF_GPIO->OUTSET = (LCD_RES_PIN_MASK) & (LCD_RES_PIN_MASK & LCD_RES_PIN_MASK); \
//                            NRF_GPIO->OUTCLR = (LCD_RES_PIN_MASK) & (LCD_RES_PIN_MASK & ~LCD_RES_PIN_MASK); } while (0)
//#define IIC_RES_HIGH    do {  NRF_GPIO->OUTCLR = (LCD_RES_PIN_MASK) & (LCD_RES_PIN_MASK & LCD_RES_PIN_MASK); \
//                           NRF_GPIO->OUTSET = (LCD_RES_PIN_MASK) & (LCD_RES_PIN_MASK & ~LCD_RES_PIN_MASK); } while (0)


#define LCD_RES_OUTPUT  nrf_gpio_cfg_output(LCD_RES_PIN)
#define LCD_RES_LOW    NRF_GPIO->OUTCLR = LCD_RES_PIN_MASK
#define LCD_RES_HIGH   NRF_GPIO->OUTSET = LCD_RES_PIN_MASK

#define LCD_POWER_HIGH  NRF_GPIO->OUTSET = LCD_POWER_PIN_MASK
#define LCD_POWER_LOW  NRF_GPIO->OUTCLR = LCD_POWER_PIN_MASK
extern uint8_t check_lastDisplayTag(uint8_t now_tag);
extern void SetPageColume(uint8_t temp_page,uint8_t temp_col,uint8_t col_change_flag);
extern UINT8 Get_lastDisplayTag(void);
extern void Set_lastDisplayTag(uint8_t now_tag);
extern uint8_t check_LCDDispCanChange(void);
extern void clear_AllScreen(void);
//extern void clear_secterScreen(uint8_t start_page,uint8_t page_num,uint8_t start_colume,uint8_t colume_width);
extern void display_check_to_clear_Screen(void);
extern void Display_delay_check(void);
//extern void lcd_power_turn_off_timer(unsigned short const msgid);

//extern void display_LOGO(void);

extern void display_USB_BT_logo(void);
extern void display_alarm(uint8_t temp_hour,uint8_t temp_munite);
//extern void Display_BT_Logo_flash(unsigned short const msgid);
//extern void DISP_CLR_ButtonLOGO(unsigned short const msgid);


//extern void display_day_clock(uint16_t temp_year,uint8_t temp_month,uint8_t temp_day,uint8_t temp_week,unsigned temp_hour,unsigned char temp_munite,unsigned char temp_second);
void display_IncomingCall(uint8_t flag,unsigned char * data_srt,uint8_t temp_length);
extern void display_clr_COSdata(void);
//extern void display_Keymode(KEY_MODE temp_mode);
//extern void Display_sysTime(uint8_t candisp_flag);

//extern void display_start_get_RSSI(unsigned short const msgid);

//extern void display_stepNUM(KeyBoard_Down_2_Disp_MainMenu disp_flag);

extern void Display_KeyDownData(void);
//extern void Display_default_message(void);
extern void lcd_power_control(unsigned char flg);
extern void ET_LCD_PW_close(void);
extern void DisplayTestBT(unsigned char switch_val);
extern void DisplayTestBT2222(uint16_t temp_number,uint8_t start,uint8_t clo_start);
extern void LCD_init(void);

/*******************************************************************************************/
struct lcd_disp_para{
	 unsigned char  lcd_col;
	 unsigned char  lcd_page;
};

#define DISP_START      	0
#define DISP_END          	1

struct bat_cos_level
{    
	unsigned char  cos_sta_level :4;
	unsigned char  bat_level :3;
	unsigned char  lcd_display_flg:1;
};



//#define BAT_CHARGER_PROGRESS           0
//#define BAT_CHARGER_FULL               1
//#define BAT_UNCHARGER                  2

#if (DYNAMIC_DISPLAY_LOGO_BY_TIMER)
#define BAT_VOLT_COUNTER               50
#else
#define BAT_VOLT_COUNTER               500
#endif

#define BT_LOGO_FLASH_COUNTER     500

extern  struct lcd_disp_para  LcdDispPara;
extern  BOOL key_ver_disp_start ;
extern BOOL key_ver_disp_end;


//extern void init_lcd(void);
void BT_Get_KeyVersion(unsigned char * prt,unsigned char *str_length);

void clear_AllScreen(void);

extern void clear_all_screen(void);
extern void DisplayEntongLogo(unsigned char page_start);
extern void display_usb_logo_16x24(unsigned char disp_enable);
//extern void display_bt_logo_16x16(unsigned char disp_enable);
//extern void open_lcd(void);
//extern void close_lcd(void);
extern BOOL Display_idle_logo(void);
extern void display_gb18030_string(const unsigned char *text, unsigned int len, unsigned char row, unsigned char col);
//extern void DisplaySwitchState(unsigned char SW_Value);
extern void DisplayKeyState(unsigned char SW_Value);
extern void  dynamic_display_cos_staus(void);
extern void set_cos_progress(unsigned int ms);
extern void dynamic_display_bat_logo(void);	 
//extern void set_bt_logo_flash_state(BOOL  bt_flash_enable);
//extern void dynamic_display_bluetooth_logo(void);
extern void display_pairing_connecting_menu(void);
//extern void set_display_flg(BOOL flg);
extern void display_Rssi_distance(signed char temp_RSSI,uint8_t flag);
extern void display_key_ver_counter(void);
extern void display_key_version(void);
extern void VoltageDetectInit(void);
extern void Dislay_Test_LCD(void);
extern void Check_RSSI_display(void);
//extern void Display_CosStatus_Batt_BT_ICON(unsigned short const msgid);
//===================Cos display=================================
//void display_CosData_prepare(void);
//void display_CosData_tansfer(uint8_t* str,uint16_t str_length);
//void display_CosData_ok(void);
//void display_CosData_complete(void);


#endif
