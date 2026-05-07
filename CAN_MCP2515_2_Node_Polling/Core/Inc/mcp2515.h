/*
 * mcp2515.h
 *
 *  Created on: 07-Apr-2026
 *      Author: AI
 */

#ifndef INC_MCP2515_H_
#define INC_MCP2515_H_

#include "stm32f4xx_hal.h"

// SPI handle (from CubeMX)
extern SPI_HandleTypeDef hspi1;

typedef struct
{
    SPI_HandleTypeDef *hspi;
    GPIO_TypeDef *CS_Port;
    uint16_t CS_Pin;
} MCP2515_Handle_t;

// CS control
static inline void MCP_CS_LOW(MCP2515_Handle_t *mcp)
{
    HAL_GPIO_WritePin(mcp->CS_Port, mcp->CS_Pin, GPIO_PIN_RESET);
}

static inline void MCP_CS_HIGH(MCP2515_Handle_t *mcp)
{
    HAL_GPIO_WritePin(mcp->CS_Port, mcp->CS_Pin, GPIO_PIN_SET);
}

// MCP2515 Commands
#define MCP_RESET          0xC0
#define MCP_READ           0x03
#define MCP_WRITE          0x02
#define MCP_BITMOD         0x05
#define MCP_RTS_TX0        0x81
#define MCP_READ_STATUS    0xA0

// Registers
#define CANCTRL            0x0F
#define CANSTAT            0x0E
#define CNF1               0x2A
#define CNF2               0x29
#define CNF3               0x28

#define TXB0SIDH           0x31
#define TXB0SIDL           0x32
#define TXB0DLC            0x35
#define TXB0D0             0x36
#define TXB0CTRL		   0x30

#define RXB1SIDH		   0x71
#define RXB0SIDH           0x61
#define RXB0SIDL           0x62
#define RXB0DLC            0x65
#define RXB0D0             0x66

#define CANINTF            0x2C
#define CANINTE            0x2B
#define EFLG 			   0x2D

// Function prototypes
void MCP2515_Init(MCP2515_Handle_t *mcp);
void MCP2515_Reset(MCP2515_Handle_t *mcp);
void MCP2515_Write(MCP2515_Handle_t *mcp, uint8_t addr, uint8_t data);
uint8_t MCP2515_SPI_Read(MCP2515_Handle_t *mcp);
uint8_t MCP2515_Read(MCP2515_Handle_t *mcp, uint8_t addr);
void MCP2515_BitModify(MCP2515_Handle_t *mcp, uint8_t addr, uint8_t mask, uint8_t data);
uint8_t MCP2515_ReadStatus(MCP2515_Handle_t *mcp);
void MCP2515_LoopbackInit(MCP2515_Handle_t *mcp);


void MCP2515_Send(MCP2515_Handle_t *mcp, uint16_t id, uint8_t *data, uint8_t len);
uint8_t MCP2515_Receive(MCP2515_Handle_t *mcp, uint16_t *id, uint8_t *data, uint8_t *len);



#endif /* INC_MCP2515_H_ */
