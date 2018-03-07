/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2018 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32l0xx_hal.h"

/* USER CODE BEGIN Includes */
#include "stm32l0xx_hal_rtc.h"
#include "led.h"
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc;

RTC_HandleTypeDef hrtc;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
void MX_ADC_Init(void);
void MX_RTC_Init(void);
static void MX_USART2_UART_Init(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

uint8_t getBCD(uint8_t *str, int i)
{
	uint8_t nibble_low, nibble_high;
	
	i = i*3;
	
	nibble_high = str[i]-'0';
	nibble_low  = str[i+1]-'0';
	
	return (nibble_high<<4)|nibble_low;
}

/* RTC init function */
// "18h00m00s06w02m17d18y" 21 bytes
// 01w == monday
enum time_date_fields {TD_HOUR, TD_MIN, TD_SEC, TD_WEEKDAY, TD_MONTH, TD_DAY, TD_YEAR};
	
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

void RTC_Set_Enable_WakeUp_AlarmA(void)
{
  RTC_AlarmTypeDef sAlarm;

  // Enable the Alarm A 
  
  sAlarm.AlarmTime.Hours = 0x06;
  sAlarm.AlarmTime.Minutes = 0x00;
  sAlarm.AlarmTime.Seconds = 0x30;
  sAlarm.AlarmTime.SubSeconds = 0x0;
  sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
  sAlarm.AlarmMask = RTC_ALARMMASK_DATEWEEKDAY|RTC_ALARMMASK_HOURS
                              |RTC_ALARMMASK_MINUTES;
  sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
  sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
  sAlarm.AlarmDateWeekDay = 0x14;
  sAlarm.Alarm = RTC_ALARM_A;
  if (HAL_RTC_SetAlarm(&hrtc, &sAlarm, RTC_FORMAT_BCD) != HAL_OK)
		crash(6);
	
  //Enable the WakeUp 
    
  if (HAL_RTCEx_SetWakeUpTimer(&hrtc, 21, RTC_WAKEUPCLOCK_CK_SPRE_16BITS) != HAL_OK)
		crash(7);

	__HAL_RTC_WRITEPROTECTION_DISABLE(&hrtc);
	RTC->CR |= RTC_CR_ALRAIE;
	RTC->CR |= RTC_CR_WUTIE; 
	HAL_Delay(2);
	__HAL_RTC_WRITEPROTECTION_ENABLE(&hrtc);
	
}



// Even with a quartz 32.768Khz, the clock will drift a lot over the years
// measure luminosity and steer RTC to follow the daylight to compensate drift for years
// night is defined by level < 30/256
// middle of night is middle of 20 consecutive matching (delta<2h) blacks of duration >= 6 hours.
// night is hours between 12am and 6am. Put middle of night at 3am.
// assume drift is linear

// measure luminosity TBD
void hourStep(void)
{
		ledOn();
		HAL_Delay(150);
		ledOff();	
}

//Run for 
void saintValentin(void)
{
	for (int i=0;i<10 /*3600*(24-2)*/;i++)
		heartBeatOneSecond();
}

uint8_t g_saintValentin = 0;
uint8_t g_wakeUp = 0;

uint8_t WakeUp_AlarmA_Str[30] = "18h00m00s06w02m17d18y";

int waitConfigSerial(uint8_t *cfgStr)
{
	int cnt = 0;
	int toggle = 0;
	HAL_StatusTypeDef result;
	uint8_t connect[]="\r\nPlease send my time\r\n";
	int timeoutcount = 0;
	
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
	HAL_Delay(1000);
	ledOff();
	return 1;
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  *
  * @retval None
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	uint32_t tickstart;
  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
//  MX_ADC_Init();
//  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

	hrtc.Instance = RTC;

	if (__HAL_RTC_WAKEUPTIMER_GET_FLAG(&hrtc,RTC_ISR_WUTF)) // by WAKE UP
	{
		/* Disable the write protection for RTC registers */
		__HAL_RTC_WRITEPROTECTION_DISABLE(&hrtc);
		__HAL_RTC_WAKEUPTIMER_DISABLE(&hrtc);
		__HAL_RTC_WAKEUPTIMER_CLEAR_FLAG(&hrtc, RTC_FLAG_WUTF);
		tickstart = HAL_GetTick();
		/* Wait till RTC ALRAWF flag is set and if Time out is reached exit */
		while(__HAL_RTC_WAKEUPTIMER_GET_FLAG(&hrtc, RTC_ISR_WUTWF) == RESET)
		{
			if((HAL_GetTick() - tickstart ) > RTC_TIMEOUT_VALUE)
			{
				/* Enable the write protection for RTC registers */
				__HAL_RTC_WRITEPROTECTION_ENABLE(&hrtc);
				crash(1);
			}
		}
		__HAL_RTC_WAKEUPTIMER_ENABLE(&hrtc);
		/* Enable the write protection for RTC registers */
		__HAL_RTC_WRITEPROTECTION_ENABLE(&hrtc);
		
		g_wakeUp = 1;
	}
	
	if (__HAL_RTC_ALARM_GET_FLAG(&hrtc,RTC_ISR_ALRAF)) // By ALARM A
	{
		/* Disable the write protection for RTC registers */
		__HAL_RTC_WRITEPROTECTION_DISABLE(&hrtc);
		__HAL_RTC_ALARMA_DISABLE(&hrtc);
		__HAL_RTC_ALARM_CLEAR_FLAG(&hrtc, RTC_FLAG_ALRAF);
		tickstart = HAL_GetTick();
		/* Wait till RTC ALRAWF flag is set and if Time out is reached exit */
		while(__HAL_RTC_ALARM_GET_FLAG(&hrtc, RTC_FLAG_ALRAWF) == RESET)
		{
			if((HAL_GetTick() - tickstart ) > RTC_TIMEOUT_VALUE)
			{
				/* Enable the write protection for RTC registers */
				__HAL_RTC_WRITEPROTECTION_ENABLE(&hrtc);					
				crash(2);
			}
		}
		__HAL_RTC_ALARMA_ENABLE(&hrtc);
		/* Enable the write protection for RTC registers */
		__HAL_RTC_WRITEPROTECTION_ENABLE(&hrtc);

		g_saintValentin = 1;
	}

	if (PWR->CSR & PWR_CSR_SBF) // We come back from standby
	{
		PWR->CR |= PWR_CR_CSBF; // reset bit
		if (g_wakeUp)
		{
			hourStep();
		}
		if (g_saintValentin)
		{
			saintValentin();
		}
	}
	else // this is a power on startup
	{
		// WakeUp every 
		// Alarm every 
		heartBeatOneSecond();
		MX_USART2_UART_Init();
		waitConfigSerial(WakeUp_AlarmA_Str);
		
		__HAL_RTC_WRITEPROTECTION_DISABLE(&hrtc);
		__HAL_RTC_WAKEUPTIMER_DISABLE(&hrtc);
		__HAL_RTC_ALARMA_DISABLE(&hrtc);
		__HAL_RTC_WRITEPROTECTION_ENABLE(&hrtc);

		heartBeatOneSecond();
		heartBeatOneSecond();
		RTC_Init_Time_and_Date(WakeUp_AlarmA_Str);
		RTC_Set_Enable_WakeUp_AlarmA();
	}

	g_saintValentin = 0;
	g_wakeUp = 0;
	
	DBGMCU->CR |= DBGMCU_CR_DBG_STOP;
	DBGMCU->CR |= DBGMCU_CR_DBG_STANDBY;

	HAL_PWREx_EnableFastWakeUp();
	HAL_PWREx_EnableUltraLowPower();
	HAL_PWR_EnterSTANDBYMode();	
		
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		saintValentin();
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */

}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

    /**Configure the main internal regulator output voltage 
    */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /**Configure LSE Drive Capability 
    */
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_4;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2|RCC_PERIPHCLK_RTC;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* ADC init function */
void MX_ADC_Init(void)
{

  ADC_ChannelConfTypeDef sConfig;

    /**Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion) 
    */
  hadc.Instance = ADC1;
  hadc.Init.OversamplingMode = DISABLE;
  hadc.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV1;
  hadc.Init.Resolution = ADC_RESOLUTION_8B;
  hadc.Init.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  hadc.Init.ScanConvMode = ADC_SCAN_DIRECTION_FORWARD;
  hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc.Init.ContinuousConvMode = DISABLE;
  hadc.Init.DiscontinuousConvMode = DISABLE;
  hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc.Init.DMAContinuousRequests = DISABLE;
  hadc.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc.Init.LowPowerAutoWait = DISABLE;
  hadc.Init.LowPowerFrequencyMode = DISABLE;
  hadc.Init.LowPowerAutoPowerOff = DISABLE;
  if (HAL_ADC_Init(&hadc) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure for the selected ADC regular channel to be converted. 
    */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* RTC init function */
void MX_RTC_Init(void)
{

  RTC_TimeTypeDef sTime;
  RTC_DateTypeDef sDate;
  RTC_AlarmTypeDef sAlarm;

    /**Initialize RTC Only 
    */
  hrtc.Instance = RTC;
//if(HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR0) != 0x32F2)
	{
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initialize RTC and set the Time and Date 
    */
  sTime.Hours = 0x18;
  sTime.Minutes = 0x0;
  sTime.Seconds = 0x0;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sDate.WeekDay = RTC_WEEKDAY_SATURDAY;
  sDate.Month = RTC_MONTH_FEBRUARY;
  sDate.Date = 0x17;
  sDate.Year = 0x18;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Enable the Alarm A 
    */
  sAlarm.AlarmTime.Hours = 0x6;
  sAlarm.AlarmTime.Minutes = 0x2;
  sAlarm.AlarmTime.Seconds = 0x0;
  sAlarm.AlarmTime.SubSeconds = 0x0;
  sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
  sAlarm.AlarmMask = RTC_ALARMMASK_DATEWEEKDAY|RTC_ALARMMASK_HOURS
                              |RTC_ALARMMASK_SECONDS;
  sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
  sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
  sAlarm.AlarmDateWeekDay = 0x14;
  sAlarm.Alarm = RTC_ALARM_A;
  if (HAL_RTC_SetAlarm(&hrtc, &sAlarm, RTC_FORMAT_BCD) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Enable the WakeUp 
    */
  if (HAL_RTCEx_SetWakeUpTimer(&hrtc, 3600, RTC_WAKEUPCLOCK_CK_SPRE_16BITS) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    HAL_RTCEx_BKUPWrite(&hrtc,RTC_BKP_DR0,0x32F2);
  }

}

/* USART2 init function */
static void MX_USART2_UART_Init(void)
{

  huart2.Instance = USART2;
  huart2.Init.BaudRate = 9600;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
static void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(PHOTOPWR_GPIO_Port, PHOTOPWR_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : PHOTOPWR_Pin */
  GPIO_InitStruct.Pin = PHOTOPWR_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(PHOTOPWR_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LED_Pin */
  GPIO_InitStruct.Pin = LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  file: The file name as string.
  * @param  line: The line in file as a number.
  * @retval None
  */
void _Error_Handler(char *file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
