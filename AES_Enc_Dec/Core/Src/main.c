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
#include "mbedtls/aes.h"

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
RNG_HandleTypeDef hrng;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

uint8_t aes_key[16] =
{
    0x2B, 0x7E, 0x15, 0x16,
    0x28, 0xAE, 0xD2, 0xA6,
    0xAB, 0xF7, 0x15, 0x88,
    0x09, 0xCF, 0x4F, 0x3C
};

uint8_t plaintext[16] =
{
    0x32, 0x43, 0xF6, 0xA8,
    0x88, 0x5A, 0x30, 0x8D,
    0x31, 0x31, 0x98, 0xA2,
    0xE0, 0x37, 0x07, 0x34
};

uint8_t ciphertext[16];
uint8_t decrypted[16];

typedef uint8_t Std_ReturnType;

#ifndef E_OK
#define E_OK        ((Std_ReturnType)0u)
#endif

#ifndef E_NOT_OK
#define E_NOT_OK    ((Std_ReturnType)1u)
#endif

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

static void print_hex(const char *label, const uint8_t *buf, uint32_t len)
{
    char msg[8];

    UART_Print((char *)label);
    UART_Print(": ");

    for (uint32_t i = 0; i < len; i++)
    {
        sprintf(msg, "%02X", buf[i]);
        UART_Print(msg);
    }

    UART_Print("\r\n");
}

static Std_ReturnType aes_ecb_encrypt_128(const uint8_t key[16],
                                          const uint8_t input[16],
                                          uint8_t output[16])
{
    mbedtls_aes_context aes;
    int ret;

    mbedtls_aes_init(&aes);

    ret = mbedtls_aes_setkey_enc(&aes, key, 128);
    if (ret != 0)
    {
        mbedtls_aes_free(&aes);
        return E_NOT_OK;
    }

    ret = mbedtls_aes_crypt_ecb(&aes,
                                MBEDTLS_AES_ENCRYPT,
                                input,
                                output);

    mbedtls_aes_free(&aes);

    return (ret == 0) ? E_OK : E_NOT_OK;
}

static Std_ReturnType aes_ecb_decrypt_128(const uint8_t key[16],
                                          const uint8_t input[16],
                                          uint8_t output[16])
{
    mbedtls_aes_context aes;
    int ret;

    mbedtls_aes_init(&aes);

    ret = mbedtls_aes_setkey_dec(&aes, key, 128);
    if (ret != 0)
    {
        mbedtls_aes_free(&aes);
        return E_NOT_OK;
    }

    ret = mbedtls_aes_crypt_ecb(&aes,
                                MBEDTLS_AES_DECRYPT,
                                input,
                                output);

    mbedtls_aes_free(&aes);

    return (ret == 0) ? E_OK : E_NOT_OK;
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

  UART_Print("UART Initialized\r\n");
  UART_Print("\r\n=== AES-128 ECB TEST (mbedTLS) ===\r\n");

  memset(ciphertext, 0, sizeof(ciphertext));
  memset(decrypted, 0, sizeof(decrypted));

  print_hex("AES KEY", aes_key, sizeof(aes_key));
  print_hex("PLAINTEXT", plaintext, sizeof(plaintext));


  if (aes_ecb_encrypt_128(aes_key, plaintext, ciphertext) == E_OK)
  {
      UART_Print("AES ECB ENCRYPT SUCCESS\r\n");
      print_hex("CIPHERTEXT", ciphertext, sizeof(ciphertext));
  }
  else
  {
      UART_Print("AES ECB ENCRYPT FAILED\r\n");
  }

  if (aes_ecb_decrypt_128(aes_key, ciphertext, decrypted) == E_OK)
  {
      UART_Print("AES ECB DECRYPT SUCCESS\r\n");
      print_hex("DECRYPTED", decrypted, sizeof(decrypted));
  }
  else
  {
      UART_Print("AES ECB DECRYPT FAILED\r\n");
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
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 96;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
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
