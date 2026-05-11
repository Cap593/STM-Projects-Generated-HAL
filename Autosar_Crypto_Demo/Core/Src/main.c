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
#include "mbedtls.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include <stdio.h>
#include <string.h>

#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"

/* AUTOSAR Crypto stack */
#include "Csm.h"
#include "Cryif.h"
#include "Crypto_Hw.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* Job IDs from config */
#define TEST_JOB_HW 			CSM_JOB_ID_HW_RNG
#define TEST_JOB_SW 			CSM_JOB_ID_SW_RNG
#define TEST_JOB_HW_KEYGEN   	CSM_JOB_ID_HW_KEYGEN
#define TEST_JOB_SW_KEYGEN   	CSM_JOB_ID_SW_KEYGEN
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
RNG_HandleTypeDef hrng;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

uint8_t rngBuffer[16];
uint32_t rngLength = 16;
mbedtls_entropy_context entropy;
mbedtls_ctr_drbg_context ctr_drbg;

//For key generation
uint8_t hwKeyBuffer[32];
uint8_t swKeyBuffer[32];

uint32_t hwKeyLength = 16;
uint32_t swKeyLength = 16;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_RNG_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* Debug print helper */
void UART_Print(char *msg)
{
    HAL_UART_Transmit(&huart2, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
}

void print_buffer(uint8_t *buf, uint32_t len)
{
    char msg[100];

    for (uint32_t i = 0; i < len; i++)
    {
        sprintf(msg, "%02X", buf[i]);
        UART_Print(msg);
    }

    UART_Print("\r\n");
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
  MX_RNG_Init();
  MX_USART2_UART_Init();
  MX_MBEDTLS_Init();
  /* USER CODE BEGIN 2 */

  /* AUTOSAR Crypto layers init */
  Crypto_Hw_Init();
  CryIf_Init();
  Csm_Init();

  UART_Print("UART Initialized\r\n");
  UART_Print("Crypto Modules Initialized\r\n");

  UART_Print("\r\n");
  UART_Print("=== AUTOSAR RNG FLOW TEST ===\r\n");
  UART_Print("\r\n");

  UART_Print("== 🔹 TEST 1: HW RNG (Synchronous) ==\r\n");
  rngLength = 16;
  memset(rngBuffer, 0, sizeof(rngBuffer));
  if (Csm_RandomGenerate(TEST_JOB_HW, rngBuffer, &rngLength) == E_OK)
  {
	  UART_Print("[HW RNG] SUCCESS\r\n");
	  UART_Print("RNG : ");
	  print_buffer(rngBuffer, rngLength);
  }
  else
  {
	  UART_Print("[HW RNG] FAILED\r\n");
  }

  UART_Print("\r\n");

  UART_Print("== 🔹 TEST 2: SW RNG (Asynchronous) ==\r\n");
  rngLength = 16;
  memset(rngBuffer, 0, sizeof(rngBuffer));
  if (Csm_RandomGenerate(TEST_JOB_SW, rngBuffer, &rngLength) == E_OK)
  {
	  UART_Print("[SW RNG] JOB QUEUE SUCCESS\r\n");
  }
  else
  {
	  UART_Print("[SW RNG] FAILED\r\n");
  }

  /* Process async queue */
  for (int i = 0; i < 5; i++)
  {
	  Csm_MainFunction();
  }

  UART_Print("[SW RNG] After processing:\r\n");
  UART_Print("RNG : ");
  print_buffer(rngBuffer, rngLength);

  UART_Print("\r\n");

  UART_Print("== 🔐 TEST 3: HW KEY GENERATE ==\r\n");
  hwKeyLength = 16;

  memset(hwKeyBuffer, 0, sizeof(hwKeyBuffer));

  if (CsmJobKeyGenerate(TEST_JOB_HW_KEYGEN,
                        hwKeyBuffer,
                        &hwKeyLength) == E_OK)
  {
      UART_Print("[HW KEYGEN] SUCCESS\r\n");
      UART_Print("HW Generated Key : ");
      print_buffer(hwKeyBuffer,
                   hwKeyLength);
  }
  else
  {
      UART_Print("[HW KEYGEN] FAILED\r\n");
  }

  UART_Print("\r\n");

  UART_Print("== 🔐 TEST 4: SW KEY GENERATE ==\r\n");
  swKeyLength = 16;

  memset(swKeyBuffer, 0, sizeof(swKeyBuffer));

  if (CsmJobKeyGenerate(TEST_JOB_SW_KEYGEN,
                        swKeyBuffer,
                        &swKeyLength) == E_OK)
  {
      UART_Print("[SW KEYGEN] SUCCESS\r\n");
      UART_Print("SW Generated Key : ");
      print_buffer(swKeyBuffer,
                   swKeyLength);
  }
  else
  {
      UART_Print("[SW KEYGEN] FAILED\r\n");
  }

  UART_Print("\r\n");

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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief RNG Initialization Function
  * @param None
  * @retval None
  */
static void MX_RNG_Init(void)
{

  /* USER CODE BEGIN RNG_Init 0 */

  /* USER CODE END RNG_Init 0 */

  /* USER CODE BEGIN RNG_Init 1 */

  /* USER CODE END RNG_Init 1 */
  hrng.Instance = RNG;
  if (HAL_RNG_Init(&hrng) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RNG_Init 2 */

  /* USER CODE END RNG_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

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
