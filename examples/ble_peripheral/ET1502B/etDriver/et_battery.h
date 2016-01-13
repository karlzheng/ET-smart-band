#ifndef _ET_BATTERY_H_
#define _ET_BATTERY_H_
#include <stdint.h>


#define ADC_INPUT_CHANNEL  ADC_CONFIG_PSEL_AnalogInput5

#define ADC_REF_VOLTAGE_IN_MILLIVOLTS        1200                                     /**< Reference voltage (in  milli volts) used by ADC while doing conversion. */
#define ADC_PRE_SCALING_COMPENSATION         1                                         /**< The ADC is configured to use VDD with no prescaling as input. And hence the result of conversion is to be multiplied by 3 to get the actual value of the battery voltage.*/
#define ADC_HW_PRE_SCALING_COMPENSATION      4

#define BATTERY_ADC_RESOLUTION  (ADC_CONFIG_RES_10bit)
#define BATTERY_ADC_DIV         (1023)

#define MIN_BAT_MV (3600)
#define CHARGER_CHECK_INTERVAL_MS (APP_TIMER_TICKS(1000, APP_TIMER_PRESCALER))
#define BATTERY_VOLTAGE_ADJUSTMENT           60     /**< Adjustment for charging */

/**@brief Macro to convert the result of ADC conversion in millivolts.
 *
 * @param[in]  ADC_VALUE   ADC result.
 * @retval     Result converted to 0.1 millivolts.
 */
#define ADC_RESULT_IN_MILLI_VOLTS(ADC_VALUE)\
        ((((ADC_VALUE) * ADC_REF_VOLTAGE_IN_MILLIVOLTS)  * ADC_PRE_SCALING_COMPENSATION * ADC_HW_PRE_SCALING_COMPENSATION)/ BATTERY_ADC_DIV)


extern void battery_adc_dev_init(void);
extern void battery_start(void);
//void battery_adc_read_once(uint16_t* batt_lvl_in_milli_volts);
extern uint16_t battery_adc_read_once(void);

extern uint8_t USB_detect(void);
extern uint8_t Charge_full_detect(void);

extern void USB_detect_init(void);
extern uint8_t Get_BatteryPercent(void);

#endif
