/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

//控制黄色led灯亮度与呼吸的变量
uint16_t breath_ccr = 0;//ccr的值
uint8_t breath_direction = 1;//亮度变化方向，1变亮，0变暗

uint8_t key_state = 0;//记录按键的状态，未按下为0，按下为1
uint32_t last_time = 0;//上一次中断时间
uint32_t press_time = 0;//记录按下时间
uint32_t lift_time = 0;//记录抬起时间
uint8_t led_state = 0;//记录LED的状态，不呼吸为0，呼吸为1
uint8_t led_bright = 0;//不呼吸时led的亮度，分别为0，1，2，3,4五档

//控制舵机变量
uint16_t sg90_ccr = 50;//sg90的占空比
uint16_t sg90_time= 0;//sg90的时间参数

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_TIM3_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
// 启动 TIM3 的 PWM 输出（通道1）
HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1 );
HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2 );
HAL_TIM_Base_Start_IT(&htim2);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/*判断按键按下时间来决定led灯变化*/
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == KEY_Pin) {
        uint32_t current_time = HAL_GetTick();//获取中断时间

		if(current_time -last_time >20)	{
            if (key_state == 0) { //按键未按下
                key_state = 1; //按键按下
                press_time = current_time; //记录按下时间
            }
            else if (key_state == 1) { //按键已按下
                lift_time = current_time; //记录抬起时间
                if (lift_time - press_time >= 1000) { //长按时间为1000ms
                    if (led_state == 0) {//led不呼吸
                        HAL_TIM_Base_Start_IT(&htim3); //开启定时器中断，LED开始呼吸
                        led_state = 1;
                    }
                    else if (led_state == 1) {//led呼吸
                        HAL_TIM_Base_Stop_IT(&htim3); //关闭定时器中断，LED停止呼吸
                        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0);//将LED灯亮度调为不呼吸时的0档
                        led_state = 0;
                    }
                }
                else if (lift_time - press_time < 1000) {//判断短按
					if (led_bright < 4) {
						led_bright += 1;
					}
                    else if (led_bright == 4) {
                        led_bright = 0;
                    }
					breath_ccr = led_bright * 250;
                    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, breath_ccr);
                }
                key_state = 0; //按键状态复位
            }
        }
        last_time = current_time;//记录本次中断时间
    }
}




// 定时器中断回调函数控制呼吸灯与舵机
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim) {
    /*对呼吸灯控制*/
    if (htim->Instance == TIM3) {
        // 根据当前方向进行加减
        if (breath_direction == 1) {
            breath_ccr += 1;
        }
        else if (breath_direction == 0){
            breath_ccr -= 1;
        }
		// 改变方向
        if (breath_ccr >= 1000) {
            breath_ccr = 1000; 
            breath_direction = 0;   // 准备变暗
        }
        else if (breath_ccr <= 0) {
            breath_ccr = 0;
            breath_direction = 1;   // 准备变亮
        }

        // 3. 输出 PWM
        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, breath_ccr);
    }

    /*对舵机控制*/
    else if (htim->Instance == TIM2) {

            if (sg90_ccr >= 250) {
                sg90_ccr = 50;
            }
            else {
                sg90_ccr += 2;
            }
            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, sg90_ccr);
            sg90_time=0;
        
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
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
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
