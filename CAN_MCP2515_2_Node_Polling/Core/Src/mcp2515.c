/*
 * mcp2515.c
 *
 *  Created on: 07-Apr-2026
 *      Author: HP
 */

#include "mcp2515.h"
#include "string.h"
#include "stdio.h"

extern UART_HandleTypeDef huart2;
char mcp_driver_msg[40];
uint8_t mcpvar;

void MCP2515_Reset(MCP2515_Handle_t *mcp)
{
    uint8_t cmd = MCP_RESET;

    MCP_CS_LOW(mcp);
    HAL_SPI_Transmit(mcp->hspi, &cmd, 1, HAL_MAX_DELAY);
    MCP_CS_HIGH(mcp);

    HAL_Delay(10);
}

void MCP2515_Write(MCP2515_Handle_t *mcp, uint8_t addr, uint8_t data)
{
    uint8_t tx[3];

    tx[0] = MCP_WRITE;
    tx[1] = addr;
    tx[2] = data;

    MCP_CS_LOW(mcp);
    HAL_SPI_Transmit(mcp->hspi, tx, 3, HAL_MAX_DELAY);
    MCP_CS_HIGH(mcp);
}

uint8_t MCP2515_Read(MCP2515_Handle_t *mcp, uint8_t addr)
{
    uint8_t tx[3] = {MCP_READ, addr, 0x00};
    uint8_t rx[3] = {0};

    MCP_CS_LOW(mcp);
    HAL_SPI_TransmitReceive(mcp->hspi, tx, rx, 3, HAL_MAX_DELAY);
    MCP_CS_HIGH(mcp);

    return rx[2];
}

uint8_t MCP2515_ReadStatus(MCP2515_Handle_t *mcp)
{
    uint8_t tx[2] = {0xA0, 0x00};
    uint8_t rx[2] = {0};

    MCP_CS_LOW(mcp);
    HAL_SPI_TransmitReceive(mcp->hspi, tx, rx, 2, HAL_MAX_DELAY);
    MCP_CS_HIGH(mcp);

    return rx[1];
}

void MCP2515_BitModify(MCP2515_Handle_t *mcp, uint8_t addr, uint8_t mask, uint8_t data)
{
    uint8_t tx[4];

    tx[0] = MCP_BITMOD;
    tx[1] = addr;
    tx[2] = mask;
    tx[3] = data;

    MCP_CS_LOW(mcp);
    HAL_SPI_Transmit(mcp->hspi, tx, 4, HAL_MAX_DELAY);
    MCP_CS_HIGH(mcp);
}


void MCP2515_Init(MCP2515_Handle_t *mcp)
{

    MCP2515_Reset(mcp);
    HAL_Delay(200);

    mcpvar = (MCP2515_Read(mcp, CANSTAT) & 0xE0);
    sprintf(mcp_driver_msg,"MCP RESET CANSTAT = 0x%02X\r\n",mcpvar);
    HAL_UART_Transmit(&huart2, (uint8_t *)mcp_driver_msg, sizeof(mcp_driver_msg), HAL_MAX_DELAY);
    //while ((MCP2515_Read(mcp, CANSTAT) & 0xE0) != 0x80);

    // 500 kbps @ 8MHz
    MCP2515_Write(mcp, CNF1, 0x00);
    MCP2515_Write(mcp, CNF2, 0x91);
    MCP2515_Write(mcp, CNF3, 0x01);

    // Accept all
    MCP2515_Write(mcp, 0x20, 0x00);
    MCP2515_Write(mcp, 0x21, 0x00);
    MCP2515_Write(mcp, 0x24, 0x00);
    MCP2515_Write(mcp, 0x25, 0x00);

    MCP2515_Write(mcp, 0x60, 0x60);
    MCP2515_Write(mcp, 0x70, 0x60);

    for(uint8_t addr = 0x00; addr <= 0x0B; addr++)
    {
        MCP2515_Write(mcp, addr, 0x00);
    }

    MCP2515_BitModify(mcp, CANINTF, 0x03, 0x00);

    MCP2515_Write(mcp, CANCTRL, 0x00);

    mcpvar = (MCP2515_Read(mcp, CANSTAT) & 0xE0);
    sprintf(mcp_driver_msg,"MCP MODE CANSTAT = 0x%02X\r\n",mcpvar);
    HAL_UART_Transmit(&huart2, (uint8_t *)mcp_driver_msg, sizeof(mcp_driver_msg), HAL_MAX_DELAY);
    //while ((MCP2515_Read(mcp, CANSTAT) & 0xE0) != 0x00);
}

void MCP2515_LoopbackInit(MCP2515_Handle_t *mcp)
{
    MCP2515_Reset(mcp);
    HAL_Delay(200);

    while ((MCP2515_Read(mcp, CANSTAT) & 0xE0) != 0x80);

    // 500 kbps @ 8MHz
    MCP2515_Write(mcp, CNF1, 0x00);
    MCP2515_Write(mcp, CNF2, 0x91);
    MCP2515_Write(mcp, CNF3, 0x01);

    // Accept all
    MCP2515_Write(mcp, 0x20, 0x00);
    MCP2515_Write(mcp, 0x21, 0x00);
    MCP2515_Write(mcp, 0x24, 0x00);
    MCP2515_Write(mcp, 0x25, 0x00);

    MCP2515_Write(mcp, 0x60, 0x60);
    MCP2515_Write(mcp, 0x70, 0x60);

    for(uint8_t addr = 0x00; addr <= 0x0B; addr++)
    {
        MCP2515_Write(mcp, addr, 0x00);
    }

    MCP2515_BitModify(mcp, CANINTF, 0x03, 0x00);

    // Enter LOOPBACK mode
    MCP2515_Write(mcp, CANCTRL, 0x40);

    while ((MCP2515_Read(mcp, CANSTAT) & 0xE0) != 0x40);
}

/*void MCP2515_Send(MCP2515_Handle_t *mcp, uint16_t id, uint8_t *data, uint8_t len)
{
    uint8_t cmd = MCP_RTS_TX0;

    // Load Standard ID
    MCP2515_Write(mcp, TXB0SIDH, (uint8_t)(id >> 3));
    MCP2515_Write(mcp, TXB0SIDL, (uint8_t)((id & 0x07) << 5));

    // DLC
    MCP2515_Write(mcp, TXB0DLC, len & 0x0F);

    // Data bytes
    for (uint8_t i = 0; i < len; i++)
    {
        MCP2515_Write(mcp, TXB0D0 + i, data[i]);
    }

    // Request To Send
    MCP_CS_LOW(mcp);
    HAL_SPI_Transmit(mcp->hspi, &cmd, 1, HAL_MAX_DELAY);
    MCP_CS_HIGH(mcp);
}*/

void MCP2515_Send(MCP2515_Handle_t *mcp, uint16_t id, uint8_t *data, uint8_t len)
{
uint8_t cmd = MCP_RTS_TX0;

/* Clear previous TX request */
MCP2515_BitModify(mcp, TXB0CTRL, 0x08, 0x00);

/* Load ID */
MCP2515_Write(mcp, TXB0SIDH, (uint8_t)(id >> 3));
MCP2515_Write(mcp, TXB0SIDL, (uint8_t)((id & 0x07) << 5));

/* DLC */
MCP2515_Write(mcp, TXB0DLC, len & 0x0F);

/* Data */
for (uint8_t i = 0; i < len; i++)
{
    MCP2515_Write(mcp, TXB0D0 + i, data[i]);
}

/* Request to send */
MCP_CS_LOW(mcp);
HAL_SPI_Transmit(mcp->hspi, &cmd, 1, HAL_MAX_DELAY);
MCP_CS_HIGH(mcp);

/* Wait for TX complete (timeout protected) */
uint32_t timeout = HAL_GetTick();
while (MCP2515_Read(mcp, TXB0CTRL) & 0x08)
{
    if (HAL_GetTick() - timeout > 100)
        break;
}

}

uint8_t MCP2515_Receive(MCP2515_Handle_t *mcp, uint16_t *id, uint8_t *data, uint8_t *len)
{
    uint8_t intf = MCP2515_Read(mcp, CANINTF);
    uint8_t sidh, sidl;
    uint8_t baseAddr;

    if (intf & 0x01)
    {
        baseAddr = RXB0SIDH;
        MCP2515_BitModify(mcp, CANINTF, 0x01, 0x00);
    }
    else if (intf & 0x02)
    {
        baseAddr = RXB1SIDH;
        MCP2515_BitModify(mcp, CANINTF, 0x02, 0x00);
    }
    else
    {
        return 0;
    }

    sidh = MCP2515_Read(mcp, baseAddr);
    sidl = MCP2515_Read(mcp, baseAddr + 1);

    *id = (sidh << 3) | (sidl >> 5);

    *len = MCP2515_Read(mcp, baseAddr + 4) & 0x0F;

    for (uint8_t i = 0; i < *len; i++)
    {
        data[i] = MCP2515_Read(mcp, baseAddr + 5 + i);
    }
    return 1;
}
