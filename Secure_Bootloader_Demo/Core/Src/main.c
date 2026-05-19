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

#include "mbedtls/pk.h"
#include "mbedtls/sha256.h"
#include <string.h>
#include <stdio.h>
#include "rsa_keys.h"   /* public key only */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define BOOTLOADER_START_ADDR   (0x08000000U)
#define BOOT_HEADER_ADDR        (0x08010000U)
#define APP_START_ADDR          (0x08020000U)
#define BOOT_MAGIC              (0xB007B007U)

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
RNG_HandleTypeDef hrng;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_RNG_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */

static mbedtls_pk_context rsa_pub_ctx;

static void Bootloader_InitCrypto(void);
static int Boot_CalcAppHash(const uint8_t *data, uint32_t len, uint8_t outHash[32]);
static int Boot_VerifySignature(const uint8_t hash[32],
                                const uint8_t *sig,
                                uint32_t sigLen);
static void Boot_JumpToApplication(void);
static void UART_Print(char *msg);   /* your existing UART function */
static void print_hex(const char *label, const uint8_t *buf, uint32_t len);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

static void Bootloader_InitCrypto(void)
{
    mbedtls_pk_init(&rsa_pub_ctx);

    if (mbedtls_pk_parse_public_key(
            &rsa_pub_ctx,
            (const unsigned char *)rsa_public_key_pem,
            strlen(rsa_public_key_pem) + 1) != 0)
    {
        UART_Print("Public key parse failed\r\n");
        Error_Handler();
    }

    UART_Print("Public key parsed\r\n");
}

static int Boot_CalcAppHash(const uint8_t *data, uint32_t len, uint8_t outHash[32])
{
    if (mbedtls_sha256_ret(data, len, outHash, 0) != 0)
    {
        return -1;
    }

    return 0;
}

static int Boot_VerifySignature(const uint8_t hash[32],
                                const uint8_t *sig,
                                uint32_t sigLen)
{
    if (mbedtls_pk_verify(&rsa_pub_ctx,
                          MBEDTLS_MD_SHA256,
                          hash,
                          0,
                          sig,
                          sigLen) != 0)
    {
        return -1;
    }

    return 0;
}

static void Boot_JumpToApplication(void)
{
    uint32_t appStack = *(__IO uint32_t *)APP_START_ADDR;
    uint32_t appReset = *(__IO uint32_t *)(APP_START_ADDR + 4U);
    pFunction appEntry = (pFunction)appReset;

    UART_Print("Jumping to application\r\n");

    __disable_irq();

    /* stop SysTick */
    SysTick->CTRL = 0U;
    SysTick->LOAD = 0U;
    SysTick->VAL  = 0U;

    /* deinit HAL and clocks */
    HAL_RCC_DeInit();
    HAL_DeInit();

    /* disable all interrupts */
    for (uint32_t i = 0U; i < 8U; i++)
    {
        NVIC->ICER[i] = 0xFFFFFFFFU;
        NVIC->ICPR[i] = 0xFFFFFFFFU;
    }

    /* relocate vector table */
    SCB->VTOR = APP_START_ADDR;

    /* set application stack pointer */
    __set_MSP(appStack);

    appEntry();
}

static void UART_Print(char *msg)
{
    HAL_UART_Transmit(&huart2, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
}

static void print_hex(const char *label, const uint8_t *buf, uint32_t len)
{
    char tmp[8];

    UART_Print((char *)label);
    UART_Print(": ");

    for (uint32_t i = 0; i < len; i++)
    {
        sprintf(tmp, "%02X", buf[i]);
        UART_Print(tmp);
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

  const Boot_ImageHeaderType *hdr;
  uint8_t calcHash[32];
  char tmp[32];

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

  UART_Print("\r\n=== BOOTLOADER START ===\r\n");

  Bootloader_InitCrypto();

  hdr = (const Boot_ImageHeaderType *)BOOT_HEADER_ADDR;

  if (hdr->magic != BOOT_MAGIC)
  {
      UART_Print("Header magic invalid\r\n");
      while (1)
      {
          UART_Print("Stay in bootloader\r\n");
          HAL_Delay(1000);
      }
  }

  UART_Print("Header OK\r\n");
  UART_Print("App size: ");
  sprintf(tmp, "%lu\r\n", (unsigned long)hdr->imageSize);
  UART_Print(tmp);

  /* hash the application image */
  if (Boot_CalcAppHash((const uint8_t *)APP_START_ADDR,
                       hdr->imageSize,
                       calcHash) != 0)
  {
      UART_Print("Hash failed\r\n");
      while (1)
      {
      }
  }

  UART_Print("App hash: ");
  print_hex("", calcHash, sizeof(calcHash));

  /* optional debug check: compare stored hash with computed hash */
  if (memcmp(calcHash, hdr->imageHash, 32) != 0)
  {
      UART_Print("Stored hash mismatch\r\n");
      while (1)
      {
          UART_Print("Stay in bootloader\r\n");
          HAL_Delay(1000);
      }
  }

  UART_Print("Hash match\r\n");

  /* verify RSA signature over the hash */
  if (Boot_VerifySignature(calcHash, hdr->signature, sizeof(hdr->signature)) != 0)
  {
      UART_Print("Signature verify failed\r\n");
      while (1)
      {
          UART_Print("Stay in bootloader\r\n");
          HAL_Delay(1000);
      }
  }

  UART_Print("Signature verify OK\r\n");

  Boot_JumpToApplication();

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
