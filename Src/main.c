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
#include "set_time.h"
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
RTC_HandleTypeDef hrtc;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
void MX_RTC_Init(void);
void MX_USART2_UART_Init(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

// Even with a quartz 32.768Khz, the clock will drift a lot over the years
// You might measure luminosity (with some electronics and code) and steer RTC to follow the daylight to compensate drift for years
// your code here
// Will be called every "wakeupEvery"
// but I think that 20ppm is enough : only 1.75 hours shift in 10 years... OK.
// 24h*365.25*10*20e-6 = 1.75h
void wakeupStep(void)
{
		ledOn();
		HAL_Delay(150);
		ledOff();	
}

void daylyStep(void)
{
		ledPulse();
}

// TESTING can be defined in the project properties.
// Remove it when done.
// Run Beating Heart for ... seconds
#ifdef TESTING
#define SECONDS_BEATING_HEART  120 
#define RTC_MONTH_SELECTED RTC_MONTH_MARCH
#else
// beating heart for 18 hours :  3600*18
#define SECONDS_BEATING_HEART  (3600*18)
// saint valentin is in february
#define RTC_MONTH_SELECTED RTC_MONTH_FEBRUARY
#endif

void saintValentin(void)
{
	for (int i=0;i<SECONDS_BEATING_HEART ;i++)
		heartBeatOneSecond();
}

uint8_t g_saintValentin;
uint8_t g_wakeUp;
uint8_t g_dailyAlarm;

// Default config that will be written if nothing sent after 100secs on the serial port
// Useful in dev phase, we don't need to have a complex setup 
// But when on battery 3V, will need to use the serial port 
uint8_t clock_time[31] = "23h59m00s06w03m09d18y00120wkup"; // 30+1 for a 0x00 terminating char inserted by the compilator, but not required to have one
int clock_time_stringlength = sizeof(clock_time)-1; // 30 is the good number

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
	RTC_DateTypeDef sDate1;
	RTC_DateTypeDef sDate2;
	uint8_t outOfStandby;
  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
// WE DONT CALL MX_RTC_Init(), we run our own code
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  /* USER CODE BEGIN 2 */
	
	HAL_PWREx_EnableLowPowerRunMode();
	
	hrtc.Instance = RTC;
	
	if (PWR->CSR & PWR_CSR_SBF) 
	{
		outOfStandby = true;	// We come back from standby
		PWR->CR |= PWR_CR_CSBF; // reset bit.  
	}
	else
		outOfStandby = false;

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1) 
  {
		g_saintValentin = false; // XXXXXX  Put a breakpoint here, debug, run, breakpoint, reset, remove breakpoint then run to debug the setup code 
		g_wakeUp = false;
		g_dailyAlarm = false;

		// Identify the cause of Interrupt, and reset int flag 
		
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
			
			g_wakeUp = true;
		}

		// Identify the cause of Interrupt, and reset int flag 
		
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

// APB1 Clock = 131Khz < 7*32.768Khz
/*
To be able to read the RTC calendar register when the APB1 clock frequency is less than
seven times the RTC clock frequency (7*RTCLCK), the software must read the calendar
time and date registers twice.
If the second read of the RTC_TR gives the same result as the first read, this ensures that
the data is correct. Otherwise a third read access must be done.
*/		
				
			HAL_RTC_GetDate(&hrtc, &sDate1, RTC_FORMAT_BCD);
			HAL_RTC_GetDate(&hrtc, &sDate2, RTC_FORMAT_BCD);
			if (sDate1.Month!=sDate2.Month)
			{
				HAL_RTC_GetDate(&hrtc, &sDate1, RTC_FORMAT_BCD);
			}
				
			if (sDate1.Month==RTC_MONTH_SELECTED)
				g_saintValentin = true;
		}

		// Identify the cause of Interrupt, and reset int flag 
		
		if (__HAL_RTC_ALARM_GET_FLAG(&hrtc,RTC_ISR_ALRBF)) // By ALARM B
		{
				/* Disable the write protection for RTC registers */
				__HAL_RTC_WRITEPROTECTION_DISABLE(&hrtc);
				__HAL_RTC_ALARMB_DISABLE(&hrtc);
				__HAL_RTC_ALARM_CLEAR_FLAG(&hrtc, RTC_FLAG_ALRBF);
				tickstart = HAL_GetTick();
				/* Wait till RTC ALRBWF flag is set and if Time out is reached exit */
				while(__HAL_RTC_ALARM_GET_FLAG(&hrtc, RTC_FLAG_ALRBWF) == RESET)
				{
					if((HAL_GetTick() - tickstart ) > RTC_TIMEOUT_VALUE)
					{
						/* Enable the write protection for RTC registers */
						__HAL_RTC_WRITEPROTECTION_ENABLE(&hrtc);					
						crash(9);
					}
				}
				__HAL_RTC_ALARMB_ENABLE(&hrtc);
				/* Enable the write protection for RTC registers */
				__HAL_RTC_WRITEPROTECTION_ENABLE(&hrtc);

				g_dailyAlarm = true;
		}
		
		
		
		if (outOfStandby) // We come back from standby
		{
			// Process interrupts
			if (g_wakeUp)
			{
				wakeupStep();
			}
			if (g_dailyAlarm)
			{
				daylyStep();
			}
			if (g_saintValentin)
			{
				saintValentin();
			}
		}
		else // this is a power on startup : setup code : get time by serial port or set it to some default value (for tests)
		{ 
			
			pulseTrain3s();  
			getConfigByComPort(clock_time,clock_time_stringlength);
			
			__HAL_RTC_WRITEPROTECTION_DISABLE(&hrtc);
			__HAL_RTC_WAKEUPTIMER_DISABLE(&hrtc);
			__HAL_RTC_ALARMA_DISABLE(&hrtc);
			__HAL_RTC_WRITEPROTECTION_ENABLE(&hrtc);

			pulseTrain3s();
			RTC_Init_Time_and_Date(clock_time);
			RTC_Set_Enable_WakeUp_AlarmAB(clock_time);
		}
/*
	DBGMCU->CR |= DBGMCU_CR_DBG_STOP;
	DBGMCU->CR |= DBGMCU_CR_DBG_STANDBY;
*/
	
	HAL_PWREx_EnableFastWakeUp();
	HAL_PWREx_EnableUltraLowPower();
	HAL_PWR_EnterSTANDBYMode();	 // on exit from standby, the CPU will reset
	// but if a new interrupt is again pending, in that case we don't enter standby mode...
	// this while loop saves us, we process again that new interrupt. At least we should reset the flag
	// this can happen if during a long saintValentin() call, the wakeup timer triggers.
		
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

    /**Configure LSE Drive Capability 
    */
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_1;
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
  sTime.Hours = 0x12;
  sTime.Minutes = 0x0;
  sTime.Seconds = 0x0;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sDate.WeekDay = RTC_WEEKDAY_SATURDAY;
  sDate.Month = RTC_MONTH_MARCH;
  sDate.Date = 0x10;
  sDate.Year = 0x18;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Enable the Alarm A 
    */
  sAlarm.AlarmTime.Hours = 0x6;
  sAlarm.AlarmTime.Minutes = 0x0;
  sAlarm.AlarmTime.Seconds = 0x0;
  sAlarm.AlarmTime.SubSeconds = 0x0;
  sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
  sAlarm.AlarmMask = RTC_ALARMMASK_NONE;
  sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
  sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
  sAlarm.AlarmDateWeekDay = 0x14;
  sAlarm.Alarm = RTC_ALARM_A;
  if (HAL_RTC_SetAlarm(&hrtc, &sAlarm, RTC_FORMAT_BCD) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Enable the Alarm B 
    */
  sAlarm.AlarmTime.Hours = 0x0;
  sAlarm.AlarmMask = RTC_ALARMMASK_DATEWEEKDAY;
  sAlarm.AlarmDateWeekDay = 0x1;
  sAlarm.Alarm = RTC_ALARM_B;
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
void MX_USART2_UART_Init(void)
{

  huart2.Instance = USART2;
  huart2.Init.BaudRate = 4800;
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
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);

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
