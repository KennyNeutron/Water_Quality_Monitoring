/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdio.h>                   
#include "stm32f4xx_hal_adc.h"        

extern UART_HandleTypeDef huart6;
extern UART_HandleTypeDef huart2;
extern ADC_HandleTypeDef hadc1;
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
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
static void PumpCycleOnce(void){
  HAL_UART_Transmit(&huart2, (uint8_t*)"Turning On Pumps\r\n", strlen("Turning On Pumps\r\n"), HAL_MAX_DELAY);

  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_RESET); //ON Pump 1 (active low)
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_RESET); //ON Pump 2 (active low)
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET); //ON Pump3 (active high)

  vTaskDelay(pdMS_TO_TICKS(3000)); //delay 3s for now

  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_SET); //OFF Pump 1 (active low)
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_SET); //OFF Pump 2 (active low)
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_SET); //OFF Pump3 (active high)

  HAL_UART_Transmit(&huart2, (uint8_t*)"Done with Pumps\r\n", strlen("Done with Pumps\r\n"), HAL_MAX_DELAY);
}

static void phSensor_Main(void){
  char msg[64];
  uint32_t adc_val = 0;

  HAL_ADC_Start(&hadc1);
  HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
  adc_val = HAL_ADC_GetValue(&hadc1);

  // Optional: convert to voltage: V = (adc_val / 4095.0) * 3.3

  snprintf(msg, sizeof(msg), "pH ADC Raw: %lu\r\n", adc_val);
  HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY); //Change to huart6
}

static void doSensor_Main(void) {
  char msg[64];
  uint32_t adc_val = 0;
  uint32_t millivolts = 0;

  // Switch ADC channel to A5 (ADC_CHANNEL_5)
  ADC_ChannelConfTypeDef sConfig = {0};
  sConfig.Channel = ADC_CHANNEL_5;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  HAL_ADC_ConfigChannel(&hadc1, &sConfig);

  // Read raw ADC value
  HAL_ADC_Start(&hadc1);
  HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
  adc_val = HAL_ADC_GetValue(&hadc1);

  // Convert to millivolts: (adc_val / 4095) * 3300
  millivolts = (adc_val * 3300) / 4095;

  //some error
//  snprintf(msg, sizeof(msg), "DO Raw: %lu | DO Voltage: %lu mV", adc_val, do_millisvolts);
//  HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

  snprintf(msg, sizeof(msg), "DO Raw: %lu | \t", adc_val);
  HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

  snprintf(msg, sizeof(msg), "DO Voltage: %lu mV\r\n", millivolts);
  HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

}


void StartDefaultTask(void *argument){

  HAL_UART_Transmit(&huart2, (uint8_t*)"System Start\r\n", strlen("System Start\r\n"), HAL_MAX_DELAY); //Change to huart6

  PumpCycleOnce();



  for(;;)
  {
    phSensor_Main();
    vTaskDelay(pdMS_TO_TICKS(1000));
    doSensor_Main();
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

/* USER CODE END Application */
