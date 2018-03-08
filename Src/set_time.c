#include "main.h"
#include "stm32l0xx_hal.h"
#include "led.h"

void MX_USART2_UART_Init(void);
extern UART_HandleTypeDef huart2;
extern RTC_HandleTypeDef hrtc;

int getConfigByComPort(uint8_t *cfgStr)
{
	int cnt = 0;
	int toggle = 0;
	HAL_StatusTypeDef result;
	uint8_t connect[]="\r\nPlease send me the time and wake up delay. Example : $22h07m00s04w03m08d18y00060wkup\r\n";
	// the first $ is for sync purposes. Needed, else, if you unplug now the usb connector, you will receive a garbage char that we should ignore
	int timeoutcount = 0;
	int synchronized = 0;

	MX_USART2_UART_Init();
	
	HAL_Delay(3000);
	heartBeatOneSecond();
	HAL_UART_Transmit(&huart2,connect,sizeof(connect)-1,HAL_MAX_DELAY);
	
	while(cnt<21)
	{
		result = HAL_UART_Receive(&huart2,&cfgStr[cnt],1,10000); // Blocking
		if (result == HAL_TIMEOUT)
		{
			timeoutcount++;
			if (timeoutcount==10)
			{
				ledOff();
				return 0;
			}
			heartBeatOneSecond();
			HAL_UART_Transmit(&huart2,connect,sizeof(connect)-1,HAL_MAX_DELAY);
		}
		else
		{
			if (!synchronized)
			{
				if (cfgStr[cnt]=='$')
					synchronized = 1;
			}
			else if (cfgStr[cnt]!='$')
			{
				cnt++;
				if (toggle)
					ledOn();
				else 
					ledOff();
				toggle = 1-toggle;
			}
		}
	}
	HAL_Delay(1000);
	ledOff();

	return 1;
}


uint8_t getBCD(uint8_t *str, int i)
{
	uint8_t nibble_low, nibble_high;
	
	i = i*3;
	
	nibble_high = str[i]-'0';
	nibble_low  = str[i+1]-'0';
	
	return (nibble_high<<4)|nibble_low;
}

uint16_t get16bits(uint8_t *str, int i)
{
	uint8_t nibble;
	uint32_t total;
	
	i = i*3;
	
	total = 0;
	for (int x=0;x<5;x++)
	{
		nibble = str[i]-'0';
		total = total*10+nibble;
		i++;
	}
	
	if (total>65535)
		total = 65535;
	return total;
}

/* RTC init function */
// "18h00m00s06w02m17d18y00000wup" 29+130 bytes
//  hour, minute, second, weekday, month, day of the month, year, wakeupEvery in seconds from 1 to 65535, 
// 01w == monday
enum time_date_fields {TD_HOUR, TD_MIN, TD_SEC, TD_WEEKDAY, TD_MONTH, TD_DAY, TD_YEAR, TD_WAKE_UP_DELAY, TD_FILLER};
	
void RTC_Init_Time_and_Date(uint8_t td[30])
{

  RTC_TimeTypeDef sTime;
  RTC_DateTypeDef sDate;

	/**Initialize RTC Only 
	*/
  hrtc.Instance = RTC;

  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
	
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
		crash(3);
	
	/**Initialize RTC and set the Time and Date 
	*/
  sTime.Hours   = getBCD(td,TD_HOUR);
  sTime.Minutes = getBCD(td,TD_MIN);
  sTime.Seconds = getBCD(td,TD_SEC);
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
	
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
		crash(4);

  sDate.WeekDay = getBCD(td,TD_WEEKDAY);
  sDate.Month   = getBCD(td,TD_MONTH);
  sDate.Date    = getBCD(td,TD_DAY);
  sDate.Year    = getBCD(td,TD_YEAR);
	
  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
		crash(5);

}


// ALARMA every Valentine's Day !

void RTC_Set_Enable_WakeUp_AlarmA(uint8_t td[30])
{
  RTC_AlarmTypeDef sAlarm;
	uint16_t wakeupEvery;

  // Enable the Alarm A, Saint Valentin. Will wake up every month on the 14th, so we will have to read to check we are in february
  // Since Month is not part of the comparison for the alarm (the RTC does not offer that), 
	//   we will wake up every month on the 14th and we will need to check if we are in february
	
  sAlarm.AlarmTime.Hours = 0x06;
  sAlarm.AlarmTime.Minutes = 0x00;
  sAlarm.AlarmTime.Seconds = 0x00;
  sAlarm.AlarmTime.SubSeconds = 0x00;
  sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
  sAlarm.AlarmMask = RTC_ALARMMASK_MINUTES|RTC_ALARMMASK_SECONDS; // RTC_ALARMMASK_DATEWEEKDAY|RTC_ALARMMASK_HOURS 
  sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
  sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
  sAlarm.AlarmDateWeekDay = 0x14; //0x14;  
  sAlarm.Alarm = RTC_ALARM_A;
  if (HAL_RTC_SetAlarm(&hrtc, &sAlarm, RTC_FORMAT_BCD) != HAL_OK)
		crash(6);
	
	wakeupEvery = get16bits(td,TD_WAKE_UP_DELAY);
	
	//Enable the repeated WakeUp 
	if (wakeupEvery)
	{
		if (HAL_RTCEx_SetWakeUpTimer(&hrtc, wakeupEvery-1, RTC_WAKEUPCLOCK_CK_SPRE_16BITS) != HAL_OK)
			crash(7);
	}
	else
		HAL_RTCEx_DeactivateWakeUpTimer(&hrtc);
	
	// Interrupt Enable (required ?)
	__HAL_RTC_WRITEPROTECTION_DISABLE(&hrtc);
	RTC->CR |= RTC_CR_ALRAIE;
	if (wakeupEvery)
		RTC->CR |= RTC_CR_WUTIE; 
	else
		RTC->CR &= ~RTC_CR_WUTIE; 
		
	HAL_Delay(2);
	__HAL_RTC_WRITEPROTECTION_ENABLE(&hrtc);
	
}


