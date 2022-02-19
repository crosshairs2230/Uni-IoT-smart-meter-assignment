/*
 * main.c
 *
 * 
 *
 * author:    Chris Kay
 * date:      02/02/2022
 * purpose:   
 */

// include the c standard io functions
#include <stdio.h>

// include the basic headers and hal drivers
#include "stm32f7xx_hal.h"

// include the shu bsp libraries for the stm32f7 discovery board
#include "pinmappings.h"
#include "clock.h"
#include "stm32746g_discovery_lcd.h"

// ADC libary for sensor readings
#include "adc.h"

// Add GPIO library for pushbuttons and LEDs 
#include "gpio.h"

// define boarder
#define BOARDER     "----------------------------"

int main()
{
  // initialise the HAL library and define the SystemCoreClock speed
  HAL_Init();
  init_sysclk_216MHz();

  // initialise the lcd
  BSP_LCD_Init();
  BSP_LCD_LayerDefaultInit(LTDC_ACTIVE_LAYER, SDRAM_DEVICE_ADDR);
  BSP_LCD_SelectLayer(LTDC_ACTIVE_LAYER);

  // set the background colour to blue and clear the lcd
  BSP_LCD_SetBackColor(LCD_COLOR_BLACK);
  BSP_LCD_Clear(LCD_COLOR_BLACK);
  
  // set the font to use
  BSP_LCD_SetFont(&Font24); 
  
  // display GUI framework
  BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
  BSP_LCD_DisplayStringAtLine(0, (uint8_t *)BOARDER);
  BSP_LCD_DisplayStringAtLine(1, (uint8_t *)"    SMART METER READINGS");
	
	BSP_LCD_DisplayStringAtLine(2, (uint8_t *)BOARDER);
	BSP_LCD_DisplayStringAtLine(3, (uint8_t *)" ELECTRICITY USAGE (KW/h):");
	
	BSP_LCD_DisplayStringAtLine(5, (uint8_t *)BOARDER);
	BSP_LCD_DisplayStringAtLine(6, (uint8_t *)" GAS USAGE (M^3):");
	
	BSP_LCD_DisplayStringAtLine(8, (uint8_t *)BOARDER);
	BSP_LCD_DisplayStringAtLine(9, (uint8_t *)" WATER USAGE (M^3):");
	
	// Initialise the sensors and button pins
	gpio_pin_t eSens = {PA_0, GPIOA, GPIO_PIN_0};
	gpio_pin_t gSens = {PF_10, GPIOF, GPIO_PIN_10};
	gpio_pin_t wSens = {PF_9, GPIOF, GPIO_PIN_9};
	
	gpio_pin_t eLed = {PI_0, GPIOI, GPIO_PIN_0};
	gpio_pin_t gLed = {PH_6, GPIOH, GPIO_PIN_6};
	gpio_pin_t wLed = {PI_3, GPIOI, GPIO_PIN_3};
	gpio_pin_t pb1 = {PG_7, GPIOG, GPIO_PIN_7};
	
	// Initialise adc for electricity water and gas sensors
	init_adc(eSens);
	init_adc(gSens);
	init_adc(wSens);
	
	// Initialise the pushbuttons and LEDs
	init_gpio(pb1, INPUT);
	init_gpio(eLed, OUTPUT);
	init_gpio(gLed, OUTPUT);
	init_gpio(wLed, OUTPUT);
	
	// variables for storing last values
	uint16_t prev_adc_eSense = 0;
	uint16_t prev_adc_gSense = 0;
	uint16_t prev_adc_wSense = 0;
	
	// resource alarm values
	int eAlarm = 1024;
	int gAlarm = 1024;
	int wAlarm = 1024;
	
	//Create a mode variable (0 = Kw/H, 1 = p/Hr)
	int meterMode = 0;
	//int meterModeState = 0;
	
	int eVal = 0;
	int gVal = 0;
	int wVal = 0;
	
	//char eUnit[12];
	//char gUnit[12];
	//char wUnit[12];
	
	while(1)
	{
		// obtain readings from each sensor
		uint16_t adc_eSens = read_adc(eSens);
		uint16_t adc_gSens = read_adc(gSens);
		uint16_t adc_wSens = read_adc(wSens);
		
		
		if(read_gpio(pb1)== HIGH)
		{
			BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
			switch(meterMode)
			{
				case 0:
				//Convert elecricity sensor reading into KW/Hr (0 = 0kw/h, 4096 = 40kw/h)
				eVal = adc_eSens/10;
				BSP_LCD_DisplayStringAtLine(3, (uint8_t *)" ELECTRICITY USAGE (KW/h):");
				//Convert gas sensor reading into M^3 (0 = 0M^3, 4096 = 40M^3)
				gVal = adc_gSens/10;
				BSP_LCD_DisplayStringAtLine(6, (uint8_t *)" GAS USAGE (M^3): ");
				//Convert water sensor reading into M^3 (0 = 0M^3, 4096 = 40M^3)
				wVal = adc_wSens/10;
				BSP_LCD_DisplayStringAtLine(9, (uint8_t *)" WATER USAGE (M^3): ");
				
				meterMode = 1;
				break;
				
				case 1:
				//Convert elecricity sensor reading into currency reading
				eVal = adc_eSens/100;
				BSP_LCD_DisplayStringAtLine(3, (uint8_t *)" ELECTRICITY USAGE ($/hr):");
				//Convert gas sensor reading into currency reading
				gVal = adc_gSens/100;
				BSP_LCD_DisplayStringAtLine(6, (uint8_t *)" GAS USAGE ($/hr):  ");
				//Convert water sensor reading into currency reading
				wVal = adc_wSens/100;
				BSP_LCD_DisplayStringAtLine(9, (uint8_t *)" WATER USAGE ($/hr):  ");
				
				meterMode = 0;
				break;
			}
		}
		
		
		// format string for electricity reading
		char eStr[28];	
		sprintf(eStr, "                 %4d", eVal);
		
		// format string for gas reading
		char gStr[28];	
		sprintf(gStr, "                 %4d", gVal);
		
		// format string for water reading
		char wStr[28];	
		sprintf(wStr, "                 %4d", wVal);
		
		// update the electricity value on detected change
		if(adc_eSens != prev_adc_eSense)
		{
			// clear previous electricity value
			BSP_LCD_ClearStringLine(4);
			//alarm if value goes over defined threshold
			if(adc_eSens >= eAlarm)
			{				// set text color to red
				BSP_LCD_SetTextColor(LCD_COLOR_RED);
			}
			else
			{				// set text color to white
				BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
			}
			// print the electricity value
			BSP_LCD_DisplayStringAtLine(4, (uint8_t *)eStr);
			prev_adc_eSense = adc_eSens;
		}
		
		// update the gass value on detected change
		if(adc_gSens != prev_adc_gSense)
		{
			// print the message to the lcd
			BSP_LCD_ClearStringLine(7);
			// alarm if value goes over defined threshold
			if(adc_gSens >= gAlarm)
			{				// set text color to red
				BSP_LCD_SetTextColor(LCD_COLOR_RED);
			}
			else
			{				// set text color to white
				BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
			}
			// print the gas value
			BSP_LCD_DisplayStringAtLine(7, (uint8_t *)gStr);
			prev_adc_gSense = adc_gSens;
		}
		
		// update the water value on detected change
		if(adc_wSens != prev_adc_wSense)
		{
			// print the message to the lcd
			BSP_LCD_ClearStringLine(10);
			// alarm if value goes over defined threshold
			if(adc_wSens >= wAlarm)
			{				// set text color to red
				BSP_LCD_SetTextColor(LCD_COLOR_RED);
			}
			else
			{				// set text color to white
				BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
			}
			BSP_LCD_DisplayStringAtLine(10, (uint8_t *)wStr);
			prev_adc_wSense = adc_wSens;
		}
		
		// leds flash when usage above threshold
		if(adc_eSens >= eAlarm) toggle_gpio(eLed);
		else write_gpio(eLed, HIGH);
		
		if(adc_gSens >= gAlarm) toggle_gpio(gLed);
		else write_gpio(gLed, HIGH);

		if(adc_wSens >= wAlarm) toggle_gpio(wLed);
		else write_gpio(wLed, HIGH);
		
		HAL_Delay(1000);
	}
}
