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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "mcp2515.h"
#include "stdio.h"
#include "string.h"

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
CAN_HandleTypeDef hcan1;

SPI_HandleTypeDef hspi1;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

uint8_t txDataNode1[8];
uint8_t txDataNode2[8];
uint16_t id;
uint8_t rxDataNode1[8];
uint8_t rxDataNode2[8];
uint8_t len;
uint8_t val;
uint8_t mcpstatus;
uint8_t eflg;
uint8_t intf;
uint8_t rxStatus;
uint8_t tec;
uint8_t rec;
uint8_t RXB0;
uint8_t RXB1;

CAN_TxHeaderTypeDef txHeader;
uint32_t txMailbox;
HAL_CAN_StateTypeDef CAN_State_Var;
CAN_RxHeaderTypeDef rxHeader;


char msgNode1[100];
char msgNode2[100];
char msgMCP[100];

uint32_t spi_error;
HAL_SPI_StateTypeDef spi_state;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_CAN1_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void CAN_Filter_Config(void)
{
    CAN_FilterTypeDef filter;

    filter.FilterActivation = ENABLE;     // enable this filter
    filter.FilterBank = 0;                // use filter bank 0
    filter.FilterFIFOAssignment = CAN_FILTER_FIFO0; // route to FIFO0

    // Accept all IDs → no filtering
    filter.FilterIdHigh = 0x0000;
    filter.FilterIdLow  = 0x0000;
    filter.FilterMaskIdHigh = 0x0000;
    filter.FilterMaskIdLow  = 0x0000;

    filter.FilterMode = CAN_FILTERMODE_IDMASK;   // mask mode
    filter.FilterScale = CAN_FILTERSCALE_32BIT;  // full 32-bit filter

    filter.SlaveStartFilterBank = 14; // required for STM32F4 dual CAN

    HAL_CAN_ConfigFilter(&hcan1, &filter);
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    // Read received message from FIFO0
    HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rxHeader, rxDataNode1);

    // Convert to readable string
    sprintf(msgNode1, "NodeA ID:0x%03lX Data:%02X %02X %02X\r\n",
            rxHeader.StdId,
			rxDataNode1[0],
			rxDataNode1[1],
			rxDataNode1[2]);

    // Send to UART (Tera Term)
    HAL_UART_Transmit(&huart2, (uint8_t*)msgNode1, strlen(msgNode1), 100);
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
	uint8_t status;
	uint8_t tx[2] = {0xA0, 0x00};
	uint8_t rx[2] = {0};


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
  MX_CAN1_Init();
  MX_SPI1_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

  MCP2515_Init();


  /* Reading MCP2515 status bits */
  MCP2515_CS_LOW();
  HAL_SPI_TransmitReceive(&hspi1, tx, rx, 2, HAL_MAX_DELAY);
  MCP2515_CS_HIGH();

  status = rx[1];

  val = MCP2515_Read(0x0E); // CANSTAT

  sprintf(msgMCP, "MCP Status: 0x%02X\r\n", status);
  HAL_UART_Transmit(&huart2, (uint8_t*)msgMCP, strlen(msgMCP), 100);

  sprintf(msgMCP, "CANSTAT: 0x%02X\r\n", val);
  HAL_UART_Transmit(&huart2, (uint8_t*)msgMCP, strlen(msgMCP), 100);

  CAN_State_Var = HAL_CAN_GetState(&hcan1);

  // Configure filter BEFORE starting CAN
  CAN_Filter_Config();

  // Start CAN peripheral
  if(HAL_CAN_Start(&hcan1) != HAL_OK)
  {
	  Error_Handler();
  }

  CAN_State_Var = HAL_CAN_GetState(&hcan1);

  // Enable RX interrupt
  if(HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
  {
	  Error_Handler();
  }

  // Configure CAN message header
  txHeader.StdId = 0x123;       // Standard ID
  txHeader.IDE   = CAN_ID_STD;  // Standard frame
  txHeader.RTR   = CAN_RTR_DATA;// Data frame
  txHeader.DLC   = 3;           // 3 bytes

  // Data payload
  txDataNode1[0] = 0x11;
  txDataNode1[1] = 0x22;
  txDataNode1[2] = 0x33;

  // Wait until mailbox is free
  while(HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) == 0)
  {}

  /*Node B TX*/
  txDataNode2[0] = 0x44;
  txDataNode2[1] = 0x55;
  txDataNode2[2] = 0x66;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	//Node1 Tx
	while(HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) == 0);
	if(HAL_CAN_AddTxMessage(&hcan1, &txHeader, txDataNode1, &txMailbox) != HAL_OK)
	{
	  Error_Handler();
	}
	HAL_Delay(100);

	//Node2 Tx
	/*MCP2515_Send(0x200, txDataNode2, 3);
	HAL_Delay(100);*/

	/*NodeB RX*/
	if (MCP2515_Receive(&id, rxDataNode2, &len))
	{
		// Convert to readable string
		sprintf(msgNode2, "ID:0x%03X Data:%02X %02X %02X\r\n",
				id,
				rxDataNode2[0],
				rxDataNode2[1],
				rxDataNode2[2]);

		// Print via UART
		HAL_UART_Transmit(&huart2, (uint8_t*)msgNode2, strlen(msgNode2), 100);
	}

	intf   = MCP2515_Read(CANINTF);
	eflg   = MCP2515_Read(EFLG);
	mcpstatus = MCP2515_Read(CANSTAT);
	rxStatus = MCP2515_ReadStatus();
	tec = MCP2515_Read(0x1C);
	rec = MCP2515_Read(0x1D);
	RXB0 = MCP2515_Read(0x61),
	RXB1 = MCP2515_Read(0x71);

	sprintf(msgMCP,
	        "CANINTF:0x%02X EFLG:0x%02X CANSTAT:0x%02X RXSTATUS:0x%02X TEC:0x%02X REC:0x%02X RXB0:0x%02X RXB1:0x%02X\r\n",
	        intf, eflg, mcpstatus, rxStatus, tec, rec, RXB0, RXB1);
	HAL_UART_Transmit(&huart2, (uint8_t*)msgMCP, strlen(msgMCP), 100);



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
  RCC_OscInitStruct.PLL.PLLN = 64;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief CAN1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_CAN1_Init(void)
{

  /* USER CODE BEGIN CAN1_Init 0 */

  /* USER CODE END CAN1_Init 0 */

  /* USER CODE BEGIN CAN1_Init 1 */

  /* USER CODE END CAN1_Init 1 */
  hcan1.Instance = CAN1;
  hcan1.Init.Prescaler = 4;
  hcan1.Init.Mode = CAN_MODE_NORMAL;
  hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan1.Init.TimeSeg1 = CAN_BS1_13TQ;
  hcan1.Init.TimeSeg2 = CAN_BS2_2TQ;
  hcan1.Init.TimeTriggeredMode = DISABLE;
  hcan1.Init.AutoBusOff = DISABLE;
  hcan1.Init.AutoWakeUp = DISABLE;
  hcan1.Init.AutoRetransmission = ENABLE;
  hcan1.Init.ReceiveFifoLocked = DISABLE;
  hcan1.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN1_Init 2 */

  /* USER CODE END CAN1_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

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
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(CS_B_GPIO_Port, CS_B_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : CS_B_Pin */
  GPIO_InitStruct.Pin = CS_B_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(CS_B_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : INT_B_Pin */
  GPIO_InitStruct.Pin = INT_B_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(INT_B_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

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
