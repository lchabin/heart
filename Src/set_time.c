#include "main.h"
#include "stm32l0xx_hal.h"
#include "led.h"

void MX_USART2_UART_Init(void);
extern UART_HandleTypeDef huart2;
extern RTC_HandleTypeDef hrtc;

#ifdef TESTING
	uint8_t info[] = "\r\nYou are in debug mode\r\n";
#else
	uint8_t info[] = "\r\nYou are in final target mode\r\n";	
#endif

	uint8_t connect[]="\
\r\n\r\nPlease write (or paste) the time and wake up delay. Don't use del,backspace because it is ignored\r\n\
Example :\r\n$21h24m00s05w03m09d18y00060wkup\r\n\
Format : hour, minute, second, weekday, month, day of the month, year, wakeupEvery in seconds from 1 to 65535 (0 for no wake up). 01w == monday\r\n";

	uint8_t closure[] = "\r\nSetting time to $";
	uint8_t candisconnect[]= "\r\nYou can now disconnect usb.";
  uint8_t accepted[]="0123456789hmswdykup";
	
int valid(uint8_t c)
{
	int i;
	
	for (i=0;i<sizeof(accepted)-1;i++)
	{
		if (c==accepted[i])
			return true;
	}
	return false;
}
	
int getConfigByComPort(uint8_t *cfgStr, int clock_time_stringlength)
{
	int cnt = 0;
	HAL_StatusTypeDef result;
	int toggle = 0;
	uint32_t startTick;
	
	// the first $ is for sync purposes. Needed, else, if you unplug now the usb connector, you will receive a garbage char that we should ignore
	int timeoutcount = 0;
	int synchronized = false;

	MX_USART2_UART_Init();
	
	
	HAL_Delay(3000);
	heartBeatOneSecond();
	HAL_UART_Transmit(&huart2,connect,sizeof(connect)-1,HAL_MAX_DELAY);
	HAL_UART_Transmit(&huart2,info,sizeof(info)-1,HAL_MAX_DELAY);
	
	while(cnt<clock_time_stringlength)
	{
		if (synchronized)
		{
//			result = HAL_UART_Receive(&huart2,&cfgStr[cnt],1,HAL_MAX_DELAY); // Too slow for a paste
			while (!(huart2.Instance->ISR & USART_ISR_RXNE)); // Blocking, i wait for you.
			cfgStr[cnt] = huart2.Instance->RDR;
			result = HAL_OK;
		}
		else
		{
//			result = HAL_UART_Receive(&huart2,&cfgStr[cnt],1,10000); // Too slow for a paste
			startTick = HAL_GetTick();
			result = HAL_OK;
			while (!(huart2.Instance->ISR & USART_ISR_RXNE))
			{
				if (HAL_GetTick()-startTick > 10000) // with time out.
				{
					result = HAL_TIMEOUT;
					break;
				}
			};
			if (result == HAL_OK)
				cfgStr[cnt] = huart2.Instance->RDR;
		}
		
		if (result == HAL_TIMEOUT) // display menu, up to 10 times
		{
			timeoutcount++;
			if (timeoutcount==100)
			{
				break;
			}
			heartBeatOneSecond();
			HAL_UART_Transmit(&huart2,connect,sizeof(connect)-1,HAL_MAX_DELAY);
			HAL_UART_Transmit(&huart2,info,sizeof(info)-1,HAL_MAX_DELAY);
		}
		else
		{
			if (!synchronized) // require a '$' as first char
			{
				if (cfgStr[cnt]=='$')
				{
					synchronized = true;
					huart2.Instance->TDR = cfgStr[cnt]; // might loose some chars, best effort, even here cpu freq too low for a paste !
//				HAL_UART_Transmit(&huart2,&cfgStr[cnt],1,HAL_MAX_DELAY); // ECHO (too slow, won't work if you paste)
				}
			}
			else if (valid(cfgStr[cnt])) // accept only chars in our list
			{
				huart2.Instance->TDR = cfgStr[cnt]; // might loose some chars, best effort, even here cpu freq too low for a paste !
//			HAL_UART_Transmit(&huart2,&cfgStr[cnt],1,HAL_MAX_DELAY); // ECHO (too slow, won't work if you paste)
				cnt++;
				toggle = 1-toggle;
				if (toggle)
					ledOn();
				else
					ledOff();
			}
		}
	}

	ledOff();
	
	HAL_UART_Transmit(&huart2,closure,sizeof(closure)-1,HAL_MAX_DELAY);
	HAL_UART_Transmit(&huart2,cfgStr,clock_time_stringlength,HAL_MAX_DELAY);
	HAL_UART_Transmit(&huart2,candisconnect,sizeof(candisconnect)-1,HAL_MAX_DELAY);

	if (cnt<clock_time_stringlength)
		return 0;
	else
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

/* RTC init function for setting the real clock time */
// String format expected
// "18h00m00s06w02m17d18y00000wup" 29+130 bytes
//  hour, minute, second, weekday, month, day of the month, year, wakeupEvery in seconds from 1 to 65535, 01w == monday
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
// ALARMB everyday at midnight to track clock shift over time, and be sure it is still alive after month and years...
// + WakeUp alarm if delay requested non 0

void RTC_Set_Enable_WakeUp_AlarmAB(uint8_t td[30])
{
  RTC_AlarmTypeDef sAlarm;
	uint16_t wakeupEvery;

  // Enable the Alarm A, Saint Valentin. Will wake up every month on the 14th, so we will have to read to check we are in february
  // Since Month is not part of the comparison for the alarm (the RTC does not offer that), 
	//   we will wake up every month on the 14th and we will need to check if we are in february
	// Mask setting is touchy. The way it is here ensures that the alarm does not trigger again while we process it. At least for this application
	
#ifdef TESTING
  sAlarm.AlarmDateWeekDay = 0x10;  	
  sAlarm.AlarmTime.Hours = 0x00;
  sAlarm.AlarmTime.Minutes = 0x02;
  sAlarm.AlarmMask = 0; // RTC_ALARMMASK_DATEWEEKDAY|RTC_ALARMMASK_HOURS|RTC_ALARMMASK_MINUTES|RTC_ALARMMASK_SECONDS
  sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
#else	
  sAlarm.AlarmDateWeekDay = 0x14;  // saint valentin is on the fourteenth
  sAlarm.AlarmTime.Hours = 0x06;
  sAlarm.AlarmTime.Minutes = 0x00;
  sAlarm.AlarmMask = 0; // RTC_ALARMMASK_DATEWEEKDAY|RTC_ALARMMASK_HOURS|RTC_ALARMMASK_MINUTES|RTC_ALARMMASK_SECONDS
  sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
#endif

  sAlarm.AlarmTime.Seconds = 0x00;
  sAlarm.AlarmTime.SubSeconds = 0x00;
  sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
  sAlarm.AlarmSubSecondMask = 0; // RTC_ALARMSUBSECONDMASK_ALL
  sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;

  sAlarm.Alarm = RTC_ALARM_A;
  if (HAL_RTC_SetAlarm(&hrtc, &sAlarm, RTC_FORMAT_BCD) != HAL_OK)
		crash(6);


    /**Enable the Alarm B 
    */
  sAlarm.AlarmTime.Hours = 0x0;
  sAlarm.AlarmTime.Minutes = 0x00;
  sAlarm.AlarmMask = RTC_ALARMMASK_DATEWEEKDAY; // Every Day at midnight
  sAlarm.Alarm = RTC_ALARM_B;
  if (HAL_RTC_SetAlarm(&hrtc, &sAlarm, RTC_FORMAT_BCD) != HAL_OK)
		crash(7);

	wakeupEvery = get16bits(td,TD_WAKE_UP_DELAY);
	
	//Enable the repeated WakeUp 
	if (wakeupEvery)
	{
		if (HAL_RTCEx_SetWakeUpTimer(&hrtc, wakeupEvery-1, RTC_WAKEUPCLOCK_CK_SPRE_16BITS) != HAL_OK)
			crash(8);
	}
	else
		HAL_RTCEx_DeactivateWakeUpTimer(&hrtc);
	
	// Interrupt Enable (required ? YES)
	__HAL_RTC_WRITEPROTECTION_DISABLE(&hrtc);
	RTC->CR |= RTC_CR_ALRAIE;
	RTC->CR |= RTC_CR_ALRBIE;
	if (wakeupEvery)
		RTC->CR |= RTC_CR_WUTIE; 
	else
		RTC->CR &= ~RTC_CR_WUTIE; 
		
	HAL_Delay(2);
	__HAL_RTC_WRITEPROTECTION_ENABLE(&hrtc);
	
}


