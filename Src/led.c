#include "main.h"
#include "stm32l0xx_hal.h"


void ledOn(void)
{
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
}

void ledOff(void)
{
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
}


// fait clignoter la led RUN avec un code lisible
// led run == PB8
void crash(uint8_t crash_code)
{
	int j;

	while(1)
	{
		ledOn();
		HAL_Delay(40);
		ledOff();
		HAL_Delay(20);
		ledOn();
		HAL_Delay(40);
		ledOff();
		HAL_Delay(400);

		for (j=0;j<8;j++)
		{
			if (crash_code & (1<<(7-j)))
				ledOn();
			HAL_Delay(100);
			ledOff();
			HAL_Delay(400);
		}
	}
}


// brightness 0..20, duration 20ms
void ledPWM(uint8_t brightness)
{
	ledOn();
	HAL_Delay(brightness);
	ledOff();
	HAL_Delay(20-brightness);
}

void heartBeatOneSecondOld(void)
{
		ledOn();
		HAL_Delay(150);
		ledOff();
		HAL_Delay(150);
		ledOn();
		HAL_Delay(150);
		ledOff();
		HAL_Delay(550);
}

// 200 ms
void ledPulse(void)
{
	int i;
	uint8_t brightness = 20;

	for (i=0;i<10;i++)
	{
		ledPWM(brightness);
		brightness-=2;
	}
}

void heartBeatOneSecond(void)
{
	ledPulse(); // 200ms
	HAL_Delay(100);
	ledPulse(); // 200ms
	HAL_Delay(500);
}


