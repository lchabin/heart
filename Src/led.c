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


// displays a crash code in binary by pulsing the LED 
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
			if (crash_code & (1<<(7-j))) // starting from b7
			{
				ledOn();
				HAL_Delay(300); // if 1, long pulse
				ledOff();
				HAL_Delay(200);
			}
			else
			{
				ledOn();
				HAL_Delay(100); // if 0, short pulse
				ledOff();
				HAL_Delay(400);
			}
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


