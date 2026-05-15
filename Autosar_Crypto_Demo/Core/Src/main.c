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
#define TEST_JOB_HW 				CSM_JOB_ID_HW_RNG
#define TEST_JOB_SW 				CSM_JOB_ID_SW_RNG
#define TEST_JOB_HW_KEYGEN   		CSM_JOB_ID_HW_KEYGEN
#define TEST_JOB_SW_KEYGEN   		CSM_JOB_ID_SW_KEYGEN
#define TEST_JOB_BOOT_MAC_KEYGEN    CSM_JOB_ID_BOOT_MAC_KEYGEN
#define TEST_JOB_AES_KEYGEN			CSM_JOB_ID_AES_KEYGEN
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


uint8_t keyReadBuffer[32];
uint32_t keyReadLength = sizeof(keyReadBuffer);
uint8_t test_keylifecycle;
uint8_t flag_powercycle;

uint8_t plain[16] =
{
    0x32, 0x43, 0xF6, 0xA8,
    0x88, 0x5A, 0x30, 0x8D,
    0x31, 0x31, 0x98, 0xA2,
    0xE0, 0x37, 0x07, 0x34
};

/*
 * KEY_1 material
 */
uint8_t key1Material[16] =
{
    0xD1,0xA6,0xF8,0x27,
    0xDC,0x66,0xDF,0x54,
    0x65,0xA5,0x88,0x57,
    0xFE,0xCA,0x97,0x68
};

/*
 * CBC IV
 */
uint8_t key1Iv[16] =
{
    0x00,0x11,0x22,0x33,
    0x44,0x55,0x66,0x77,
    0x88,0x99,0xAA,0xBB,
    0xCC,0xDD,0xEE,0xFF
};

uint8_t cipher[16];
uint8_t decrypted[16];
uint32_t cipherLen = sizeof(cipher);
uint32_t decryptedLen = sizeof(decrypted);

uint8_t key1Material[16];
uint8_t key1Iv[16];

uint8_t readKeyBuf[32];
uint8_t readIvBuf[16];

uint32_t readKeyLen;
uint32_t readIvLen;

/*CBC test*/
uint8_t cbcPlain[32] =
{
    0x32,0x43,0xF6,0xA8,
    0x88,0x5A,0x30,0x8D,
    0x31,0x31,0x98,0xA2,
    0xE0,0x37,0x07,0x34,

    0x11,0x22,0x33,0x44,
    0x55,0x66,0x77,0x88,
    0x99,0xAA,0xBB,0xCC,
    0xDD,0xEE,0xFF,0x00
};

uint8_t cbcCipher[32];
uint8_t cbcDecrypt[32];

uint32_t cipherLencbc = sizeof(cbcCipher);
uint32_t decryptLen = sizeof(cbcDecrypt);

//CMAC

uint8_t cmacMsg[] =
{
    0x32, 0x43, 0xF6, 0xA8,
    0x88, 0x5A, 0x30, 0x8D,
    0x31, 0x31, 0x98, 0xA2,
    0xE0, 0x37, 0x07, 0x34
};

uint8_t cmacTag[16];
uint32_t cmacTagLen = sizeof(cmacTag);

//HASH
uint8_t msg[] = "Hello SHA256";
uint8_t hash[32];
uint32_t hashLen = sizeof(hash);


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
  /*UART_Print("=== AUTOSAR RNG FLOW TEST ===\r\n");
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

  for (int i = 0; i < 5; i++)
  {
	  Csm_MainFunction();
  }

  UART_Print("[SW RNG] After processing:\r\n");
  UART_Print("RNG : ");
  print_buffer(rngBuffer, rngLength);

  UART_Print("\r\n");

  UART_Print("== 🔐 TEST 3: GENERATE KEY using HW RNG : KEY_SLOT1 ==\r\n");
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

  UART_Print("== 🔐 TEST 4: GENERATE KEY using SW RNG : MASTER KEY ==\r\n");
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

  memset(keyReadBuffer,
         0,
         sizeof(keyReadBuffer));

  keyReadLength = sizeof(keyReadBuffer);

  if (Csm_KeyElementGet(
          1u,
          CRYPTO_KE_KEY_MATERIAL,
          keyReadBuffer,
          &keyReadLength) == E_OK)
  {
      UART_Print("TEST 5: [HW KEY READBACK] SUCCESS\r\n");
      UART_Print("Stored Key : ");

      print_buffer(keyReadBuffer,
                   keyReadLength);
      UART_Print("\r\n");
  }
  else
  {
      UART_Print("[HW KEY READBACK] FAILED\r\n");
  }

  memset(keyReadBuffer,
         0,
         sizeof(keyReadBuffer));

  keyReadLength = sizeof(keyReadBuffer);

  if (Csm_KeyElementGet(
          2u,
          CRYPTO_KE_KEY_MATERIAL,
          keyReadBuffer,
          &keyReadLength) == E_OK)
  {
      UART_Print("TEST 6: [SW KEY READBACK] SUCCESS\r\n");
      UART_Print("Stored Key : ");

      print_buffer(keyReadBuffer,
                   keyReadLength);
      UART_Print("\r\n");
  }
  else
  {
      UART_Print("[SW KEY READBACK] FAILED\r\n");
  }

  UART_Print("== 🔐 TEST 7: GENERATE KEY SLOT 3 : BOOTMAC_KEY ==\r\n");
  hwKeyLength = 16;

  memset(hwKeyBuffer, 0, sizeof(hwKeyBuffer));

  if (CsmJobKeyGenerate(TEST_JOB_BOOT_MAC_KEYGEN,
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

  UART_Print("== 🔐 TEST 8: GENERATE KEY SLOT 4 : AES KEY ==\r\n");
  hwKeyLength = 16;

  memset(hwKeyBuffer, 0, sizeof(hwKeyBuffer));

  if (CsmJobKeyGenerate(TEST_JOB_AES_KEYGEN,
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

  UART_Print("\r\n");*/

  /*UART_Print("== 🔐 TEST 9: Key Lifecycle Test ==\r\n");
  if(test_keylifecycle == 1)
  {
	  UART_Print("== Before Power cycle ==\r\n");
	  swKeyLength = 16;
	  flag_powercycle = 1;

	  if (CsmJobKeyGenerate(TEST_JOB_SW_KEYGEN, swKeyBuffer, &swKeyLength) == E_OK)
	  {
		  if (Csm_KeyElementSet(2u, CRYPTO_KE_KEY_MATERIAL, swKeyBuffer, swKeyLength) == E_OK)
		  {
			  UART_Print("KEY 2 SET OK\r\n");

			  memset(keyReadBuffer,0,sizeof(keyReadBuffer));
			  keyReadLength = sizeof(keyReadBuffer);
			  if (Csm_KeyElementGet(2u,CRYPTO_KE_KEY_MATERIAL,keyReadBuffer,&keyReadLength) == E_OK)
			  {
				  UART_Print("[KEY 2 READBACK] SUCCESS\r\n");
				  UART_Print("Stored Key : ");

				  print_buffer(keyReadBuffer,keyReadLength);
				  UART_Print("\r\n");
			  }
			  else
			  {
				  UART_Print("[SW KEY READBACK] FAILED\r\n");
			  }
		  }
	  }
  }
  else if(flag_powercycle == 1)
  {
	  if (Csm_KeyElementGet(2u,CRYPTO_KE_KEY_MATERIAL,keyReadBuffer,&keyReadLength) == E_OK)
	  {
		  UART_Print("== After Power cycle ==\r\n");
		  UART_Print("[KEY 2 READBACK] SUCCESS\r\n");
		  UART_Print("Stored Key : ");

		  print_buffer(keyReadBuffer,keyReadLength);
		  UART_Print("\r\n");
	  }
  }*/

  /*UART_Print("== 🔐 TEST : AES ECB Encryption ==\r\n");

  print_hex("Plaintext",plain,sizeof(plain));

  memset(cipher, 0, sizeof(cipher));
  memset(decrypted, 0, sizeof(decrypted));

  if (Csm_Encrypt(CSM_JOB_ID_AES_ECB_ENC,
                  CRYPTO_OPERATIONMODE_SINGLECALL,
                  plain,
                  sizeof(plain),
                  cipher,
                  &cipherLen) == E_OK)
  {
      UART_Print("AES ECB ENCRYPT OK\r\n");
	  print_hex("Ciphertext",cipher,cipherLen);
  }

  if (Csm_Decrypt(CSM_JOB_ID_AES_ECB_DEC,
                  CRYPTO_OPERATIONMODE_SINGLECALL,
                  cipher,
                  cipherLen,
                  decrypted,
                  &decryptedLen) == E_OK)
  {
      UART_Print("AES ECB DECRYPT OK\r\n");
	  print_hex("Decrypted Hex",decrypted,decryptedLen);
  }

  UART_Print("\r\n");*/

  /*if(test_keylifecycle == 1)
  {

	  UART_Print("\r\n");
	  UART_Print("=== KEY_1 MATERIAL SET ===\r\n");

	  if (Csm_KeyElementSet(
			  4u,
			  CRYPTO_KE_KEY_MATERIAL,
			  key1Material,
			  sizeof(key1Material)) == E_OK)
	  {
		  UART_Print("KEY_1 MATERIAL SET SUCCESS\r\n");
	  }
	  else
	  {
		  UART_Print("KEY_1 MATERIAL SET FAILED\r\n");
	  }

	  UART_Print("\r\n");
	  UART_Print("=== KEY_1 IV SET ===\r\n");

	  if (Csm_KeyElementSet(
			  4u,
			  CRYPTO_KE_IV,
			  key1Iv,
			  sizeof(key1Iv)) == E_OK)
	  {
		  UART_Print("KEY_1 IV SET SUCCESS\r\n");
	  }
	  else
	  {
		  UART_Print("KEY_1 IV SET FAILED\r\n");
	  }

	  UART_Print("\r\n");
	  UART_Print("=== READ KEY_1 MATERIAL ===\r\n");

	  memset(readKeyBuf,
			 0,
			 sizeof(readKeyBuf));

	  readKeyLen = sizeof(readKeyBuf);

	  if (Csm_KeyElementGet(
			  4u,
			  CRYPTO_KE_KEY_MATERIAL,
			  readKeyBuf,
			  &readKeyLen) == E_OK)
	  {
		  UART_Print("KEY_1 MATERIAL READ SUCCESS\r\n");
		  print_buffer(readKeyBuf,
					   readKeyLen);
	  }
	  else
	  {
		  UART_Print("KEY_1 MATERIAL READ FAILED\r\n");
	  }

	  UART_Print("\r\n");
	  UART_Print("=== READ KEY_1 IV ===\r\n");

	  memset(readIvBuf,
			 0,
			 sizeof(readIvBuf));

	  readIvLen = sizeof(readIvBuf);

	  if (Csm_KeyElementGet(
			  4u,
			  CRYPTO_KE_IV,
			  readIvBuf,
			  &readIvLen) == E_OK)
	  {
		  UART_Print("KEY_1 IV READ SUCCESS\r\n");
		  print_buffer(readIvBuf,
					   readIvLen);
	  }
	  else
	  {
		  UART_Print("KEY_1 IV READ FAILED\r\n");
	  }
  }

  UART_Print("\r\n");
  UART_Print("=== 🔐 AES CBC TEST ===\r\n");

  memset(cbcCipher, 0, sizeof(cbcCipher));
  memset(cbcDecrypt, 0, sizeof(cbcDecrypt));

  UART_Print("CBC Plaintext : ");
  print_buffer(cbcPlain, sizeof(cbcPlain));

  if (Csm_Encrypt(
          CSM_JOB_ID_AES_CBC_ENC,
          CRYPTO_OPERATIONMODE_SINGLECALL,
          cbcPlain,
          sizeof(cbcPlain),
          cbcCipher,
          &cipherLencbc) == E_OK)
  {
      UART_Print("AES CBC ENCRYPT OK\r\n");
      UART_Print("CBC Ciphertext : ");
      print_buffer(cbcCipher, cipherLencbc);
  }

  if (Csm_Decrypt(
          CSM_JOB_ID_AES_CBC_DEC,
          CRYPTO_OPERATIONMODE_SINGLECALL,
          cbcCipher,
		  cipherLencbc,
          cbcDecrypt,
          &decryptLen) == E_OK)
  {
      UART_Print("AES CBC DECRYPT OK\r\n");
      UART_Print("CBC Decrypted : ");
      print_buffer(cbcDecrypt, decryptLen);
  }*/

  /*UART_Print("\r\n=== AUTOSAR CMAC TEST ===\r\n");

  memset(cmacTag, 0, sizeof(cmacTag));

  if (Csm_MacGenerate(CSM_JOB_ID_CMAC_GEN,
                      CRYPTO_OPERATIONMODE_SINGLECALL,
                      cmacMsg,
                      sizeof(cmacMsg),
                      cmacTag,
                      &cmacTagLen) == E_OK)
  {
      UART_Print("CMAC GENERATE OK\r\n");
      print_hex("CMAC Tag",cmacTag, cmacTagLen);
  }
  else
  {
      UART_Print("CMAC GENERATE FAILED\r\n");
  }

  if (Csm_MacVerify(CSM_JOB_ID_CMAC_VER,
                    CRYPTO_OPERATIONMODE_SINGLECALL,
                    cmacMsg,
                    sizeof(cmacMsg),
                    cmacTag,
                    cmacTagLen) == E_OK)
  {
	  print_hex("Verified CMAC Tag",cmacTag, cmacTagLen);
      UART_Print("CMAC VERIFY OK\r\n");
  }
  else
  {
      UART_Print("CMAC VERIFY FAILED\r\n");
  }*/

  UART_Print("\r\n=== AUTOSAR SHA-256 TEST ===\r\n");
  UART_Print("MSG: ");
  UART_Print((char *)msg);
  UART_Print("\r\n");

  memset(hash, 0, sizeof(hash));

  if (Csm_Hash(CSM_JOB_ID_HASH,
               CRYPTO_OPERATIONMODE_SINGLECALL,
               msg,
               strlen((char *)msg),
               hash,
               &hashLen) == E_OK)
  {
      UART_Print("SHA256 OK\r\n");
      print_hex("HASH",hash,hashLen);
  }
  else
  {
      UART_Print("SHA256 FAILED\r\n");
  }


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
