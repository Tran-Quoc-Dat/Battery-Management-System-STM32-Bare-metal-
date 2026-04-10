/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "fonts.h"
#include "ssd1306.h"
#include "INA219.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
INA219_HandleTypeDef ina219;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void takeCurPow();
void balanceVoltageCheck();
void takeMeasurements();
void Charge();
void operate();
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
volatile uint8_t button_pressed = 0;
char mode_oled[26];
char line1[26], line2[26], line3[26], line4[26], line5[26];
int count_button = 1, mode = 0, adc_data_ready = 0;
const float protectVal = 4.4, ampCutoff = 2000;
float voltage = 0, voltage2 = 0, ref_vol = 0, current = 0, temperature = 0, pack_vol = 0, pack_cur = 0, power_supply = 0, power = 0;
uint16_t adc_scan[4];
int start = 0, first_start = 0, check = 0;
char bms_status[4] = "Nor";
float calib_CP = 0, calib_VP = 0, calib_V2 = 0, calib_PW = 0, calib_VS = 0;

//Ham hien thi OLED
void OLED(){
	sprintf(line1,"Mode: %s   ", mode_oled);
	sprintf(line2, "V1: %.02f| VP: %.02f  ", voltage-voltage2, voltage);
	if (current < 0){
		sprintf(line3, "V2: %.02f| CP:%.02f  ", voltage2, current / 1000);
	}
	else{
		sprintf(line3, "V2: %.02f| CP: %.02f  ", voltage2, current / 1000);
	}
	sprintf(line4,"VS: %.02f| PW: %.02f  ", power_supply, power);
	sprintf(line5,"ST: %s | PT: %.02f  ", bms_status, temperature);

	SSD1306_GotoXY (0,0);
  SSD1306_Puts (line1, &Font_7x10, 1);
	
	SSD1306_GotoXY (0,13);
	SSD1306_Puts (line2, &Font_7x10, 1);
	
	SSD1306_GotoXY (0,26);
	SSD1306_Puts (line3, &Font_7x10, 1);
	
	SSD1306_GotoXY (0,39);
	SSD1306_Puts (line4, &Font_7x10, 1);

	SSD1306_GotoXY (0,52);
	SSD1306_Puts (line5, &Font_7x10, 1);
	SSD1306_UpdateScreen();
}
/////////////////////////////////////////////////////
//Ham nut nhan
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){

	UNUSED(GPIO_Pin);
	if (GPIO_Pin == Mode_Button_Pin){
			first_start = 1;
			if (button_pressed == 0){
				button_pressed = 1;
				HAL_TIM_Base_Start_IT(&htim2);
				start = 1;
				if (count_button == 0){
					mode = 1;
				}
				else if (count_button == 1){
					mode = 2;
					check = 1;
					HAL_TIM_Base_Stop_IT(&htim3);
					HAL_TIM_Base_Start_IT(&htim3);
				}
				else{
					mode =3;
					check = 1;
					HAL_TIM_Base_Stop_IT(&htim3);
					HAL_TIM_Base_Start_IT(&htim3);
				}
				count_button++;
				if (count_button == 3)
				{
					count_button = 0;
				}
			}
	}
	__HAL_GPIO_EXTI_CLEAR_FLAG(GPIO_PIN_0);
}
/////////////////////////////////////////////////////
// Ham callback Timer de chong doi dong nut nhan
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
		UNUSED(htim);
    if (htim->Instance == TIM2)
    { 			
				if (button_pressed==1)
				{
					button_pressed = 0;
					HAL_TIM_Base_Stop_IT(&htim2);
				}
    }
		else if (htim->Instance == TIM3)
    { 			
				if (check==1)
				{
					check = 0;
					HAL_TIM_Base_Stop_IT(&htim3);
				}
    }
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

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
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_I2C1_Init();
  MX_I2C2_Init();
  MX_TIM2_Init();
  MX_TIM1_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
	//Start man hinh OLED
	SSD1306_Init();
	
	//Start cam bien do Voltage, Current
	INA219_Init(&ina219, &hi2c2);
	
	//Start DMA de doc cac chan IO ADC
	HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_scan,4);
	
	HAL_TIM_Base_Start_IT(&htim2);
	HAL_TIM_Base_Start_IT(&htim3);
	
	//Start xung pwm, che do mac dinh = 100% chu ky xung
	uint8_t duty_cycle = 50;
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
	__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1, duty_cycle);
	HAL_Delay(50);
	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	float supplyV = 0, calV = 0, cell1V = 0;
	int a ;
	uint8_t led_state = 0;
	int status=0;
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
			//Kiem soat nhiet do - ngat relay neu nhiet do vuot qua 45
			if (temperature >= 45){
				HAL_GPIO_WritePin(GPIOB, Relay_Pin, GPIO_PIN_RESET);
				
			//Treo chuong trinh, can reset lai stm32 de chay lai tu dau
				while(1)
				{}
			}
			else
			{
				if(HAL_GPIO_ReadPin(GPIOB, Relay_Pin) == GPIO_PIN_RESET){
					HAL_GPIO_WritePin(GPIOB, Relay_Pin, GPIO_PIN_SET);
					HAL_Delay(50);
				}
			}
			
			//Nhay led khi nhan nut
			if (start == 1){
				HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_2);
				HAL_Delay(300);
				HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_2);
				start = 0;
			}
			
			//Sac CC - chinh voltage max
			//duty_cycle chay tu 0 -> 50 tuong ung voi 0 -> 100% chu ky xung
			if(mode ==2){
				if (voltage2 >= 3.7  && voltage - voltage2 >= 3.7)
				{
					if(power_supply >8.45){
						duty_cycle -= 1;
					}
					else if (power_supply < 8.35){	
						duty_cycle += 1;
					}
					else{
						duty_cycle = 50;
					}
					
					if (duty_cycle > 50){
						duty_cycle = 50;
					}
					else if (duty_cycle < 0){
						duty_cycle = 0;
					}
					
					__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1, duty_cycle);
				}
			}
			
			//Ham chay chinh
			operate();
			
			//Hien thong tin len OLED
			OLED();
			
			HAL_Delay(100);
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
//////////////////////////////////////////////////////////////////
//Ham check thong tin ADC
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc->Instance == ADC1) {
				//Raise da doc xong data
        adc_data_ready = 1;
    }
}
//////////////////////////////////////////////////////////////////
//Hàm balancing
void balanceVoltageCheck()
{
    if (voltage - voltage2 >= 3.7)
    {
        HAL_GPIO_WritePin(GPIOA, Bal1_Pin, GPIO_PIN_SET); // Balance cell 1
    }
    else
    {	
        HAL_GPIO_WritePin(GPIOA, Bal1_Pin, GPIO_PIN_RESET);
    }

    if (voltage2 >= 3.7)
    {
        HAL_GPIO_WritePin(GPIOA, Bal2_Pin, GPIO_PIN_SET); // Balance cell 2
    }
    else
    {
        HAL_GPIO_WritePin(GPIOA, Bal2_Pin, GPIO_PIN_RESET);
    }
		
		takeCurPow();
		
		if (check == 0){
			HAL_GPIO_WritePin(GPIOA, Bal1_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOA, Bal2_Pin, GPIO_PIN_RESET);
			HAL_Delay(100);
			takeMeasurements();
			check = 1;
			HAL_TIM_Base_Start_IT(&htim3);
		}
}
//////////////////////////////////////////////////////////////////
//Ham sac pin
void Charge()
{
		float Overtemperature = 40;
    if (voltage  - voltage2 >= protectVal || voltage2 >= protectVal)
    {
			if(HAL_GPIO_ReadPin(GPIOA,Switch_Pin) != GPIO_PIN_RESET){
        HAL_GPIO_WritePin(GPIOA, Switch_Pin, GPIO_PIN_RESET);
				strcpy(bms_status, "Err");
				strcpy(mode_oled, "OverVoltage");
			}
    }
    else if (current > ampCutoff)
    {
			if(HAL_GPIO_ReadPin(GPIOA,Switch_Pin) != GPIO_PIN_RESET){
        HAL_GPIO_WritePin(GPIOA, Switch_Pin, GPIO_PIN_RESET);
				strcpy(bms_status, "Err");
        strcpy(mode_oled, "OverCurrent");
			}
    }
		else if (temperature > Overtemperature)
		{
			if(HAL_GPIO_ReadPin(GPIOA,Switch_Pin) != GPIO_PIN_RESET){
        HAL_GPIO_WritePin(GPIOA, Switch_Pin, GPIO_PIN_RESET);
				strcpy(bms_status, "Err");
        strcpy(mode_oled, "OverTemp   ");
			}
		}
    else
    {
			strcpy(bms_status, "Nor");
			if(HAL_GPIO_ReadPin(GPIOA,Switch_Pin) != GPIO_PIN_SET){
					HAL_GPIO_WritePin(GPIOA,Switch_Pin,GPIO_PIN_SET);
					HAL_GPIO_WritePin(GPIOA,Bal1_Pin,GPIO_PIN_RESET);
					HAL_GPIO_WritePin(GPIOA,Bal2_Pin,GPIO_PIN_RESET);
			}
    }	
		takeCurPow();
		
		if (check == 0){
			HAL_GPIO_WritePin(GPIOA, Switch_Pin, GPIO_PIN_RESET);
			HAL_Delay(100);
			takeMeasurements();
			check = 1;
			HAL_TIM_Base_Start_IT(&htim3);
		}
}
//////////////////////////////////////////////////////////////////
//Ham do nhiet do 
float calculate_temperature(uint16_t adc_value, float vref) {
		float R25 = 100000.0, R_FIXED = 92000.0, BETA = 3950.0, T0 = 298.15;
    float output_voltage, thermistor_resistance, temperature;

    output_voltage = ((float)adc_value * vref) / 4095.0;

    thermistor_resistance = R_FIXED * ((vref / output_voltage) - 1.0);

    temperature = 1.0 / ((log(thermistor_resistance / R25) / BETA) + (1.0 / T0));

    temperature -= 273.15;

    return temperature;
}
//////////////////////////////////////////////////////////////////
//Hàm do thong so pin o mode normal va charge
void takeMeasurements()
{
    voltage = 0;
    voltage2 = 0;
    current = 0;
		power_supply = 0;
		temperature = 0;
		power = 0;
	
		float cell1AverageVal = 0, cell2AverageVal = 0, averageAmps = 0, averagePows = 0, averagePower = 0, averageTemp =0 ;
		float supplyV = 0, calV = 0;
		float averages = 100;
	
		//He so calib - su dung VOM do va dien sai so vao day
		calib_VS = 0.8, calib_V2 = 0.2, calib_VP = 0.1;
    for (int i = 0; i < averages; i++)
    {
				while (adc_data_ready == 0){
					//Cho data ADC duoc lay xong
				}
				
				ref_vol = (2.48 * 4095)/(float)adc_scan[2];
				//Dieu kien de neu ref voltage do bi loi thi dung gia tri mac dinh = 3.3
				if (ref_vol <=3.2 || ref_vol >= 3.4){ref_vol = 3.3;}
				//Tinh sai so giua Vref thuc te và ly thuyet
				calV = (ref_vol - 3.3) * 10.00;	
			
				power_supply = (adc_scan[0] * (33 + calV)) / 4095 *5 / 10 + calib_VS;
				voltage2 =(adc_scan[1] * (33 + calV)) / 4095 *5 / 10 + calib_V2;
				temperature = calculate_temperature(adc_scan[3], 3.3);
				
				voltage = INA219_ReadBusVoltage(&ina219) + calib_VP;
				current = INA219_ReadCurrent(&ina219);
				power = INA219_ReadPower(&ina219);	
				
        cell1AverageVal += voltage;
        cell2AverageVal += voltage2;
        averageAmps += current;
				averageTemp += temperature;
				averagePower += power;
				averagePows += power_supply;
    }

    voltage = cell1AverageVal / averages;

		
    voltage2 = (cell2AverageVal / averages);
		
    current = averageAmps / averages;
		
		temperature = averageTemp / averages;
		
		power_supply = averagePows / averages;
		
		power = averagePower / averages;

		if (-1<current && current < 1) current = 0;
		
		adc_data_ready = 0;
}
//////////////////////////////////////////////////////////////////
//Ham do thong tin o che do balance
//Ham do current va power
void takeCurPow()
{
	  power = 0;
    current = 0;
		temperature = 0;
		float averageAmps = 0, averagePower = 0, averageTemp =0 ;
		float supplyV = 0, calV = 0;
		float averages = 100;
	
    for (int i = 0; i < averages; i++)
    {
				current = INA219_ReadCurrent(&ina219);
				power = INA219_ReadPower(&ina219);	
				temperature = calculate_temperature(adc_scan[3], 3.3);

        averageAmps += current;
				averagePower += power;
				averageTemp += temperature;
    }
    current = averageAmps / averages;
		temperature = averageTemp / averages;
		power = averagePower / averages;

		if (-1<current && current < 1) current = 0;
}
//////////////////////////////////////////////////////////////////
//Ham hoat dong chinh
void operate()
{
		if (mode == 2){
					strcpy(mode_oled, "Charging........");
					Charge();
					//balanceVoltageCheck();
		}
		else if (mode == 3){
					strcpy(mode_oled, "Balancing.......");
					if(HAL_GPIO_ReadPin(GPIOA,Switch_Pin) == GPIO_PIN_SET){
						HAL_GPIO_WritePin(GPIOA, Switch_Pin, GPIO_PIN_RESET);
					}
					balanceVoltageCheck();
		}
		else{
					strcpy(mode_oled, "Information");
					takeMeasurements();
					power_supply = power_supply + 0.15;
					if(HAL_GPIO_ReadPin(GPIOA,Switch_Pin) == GPIO_PIN_SET){
						HAL_GPIO_WritePin(GPIOA, Switch_Pin, GPIO_PIN_RESET);
					}
					if(HAL_GPIO_ReadPin(GPIOA,Bal1_Pin) == GPIO_PIN_SET){
						HAL_GPIO_WritePin(GPIOA, Bal1_Pin, GPIO_PIN_RESET);
					}
					if(HAL_GPIO_ReadPin(GPIOA,Bal2_Pin) == GPIO_PIN_SET){
						HAL_GPIO_WritePin(GPIOA, Bal2_Pin, GPIO_PIN_RESET);
					}
		}
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
		HAL_GPIO_WritePin(GPIOB, Relay_Pin, GPIO_PIN_RESET);
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
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
