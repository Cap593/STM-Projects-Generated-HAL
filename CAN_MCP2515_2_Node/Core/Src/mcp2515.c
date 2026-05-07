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


void MCP2515_Reset(void)
{
    uint8_t cmd = MCP_RESET;

    MCP2515_CS_LOW();
    HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
    MCP2515_CS_HIGH();

    HAL_Delay(10);
}

void MCP2515_Write(uint8_t addr, uint8_t data)
{
    uint8_t tx[3];

    tx[0] = MCP_WRITE;
    tx[1] = addr;
    tx[2] = data;

    MCP2515_CS_LOW();
    HAL_SPI_Transmit(&hspi1, tx, 3, HAL_MAX_DELAY);
    MCP2515_CS_HIGH();
}

uint8_t MCP2515_Read(uint8_t addr)
{
    uint8_t tx[3] = {MCP_READ, addr, 0x00};
    uint8_t rx[3] = {0};

    MCP2515_CS_LOW();

    HAL_SPI_TransmitReceive(&hspi1, tx, rx, 3, HAL_MAX_DELAY);

    MCP2515_CS_HIGH();

    return rx[2];
}

uint8_t MCP2515_ReadStatus(void)
{
    uint8_t tx[2] = {0xA0, 0x00};
    uint8_t rx[2] = {0};

    MCP2515_CS_LOW();
    HAL_SPI_TransmitReceive(&hspi1, tx, rx, 2, HAL_MAX_DELAY);
    MCP2515_CS_HIGH();

    return rx[1];
}

void MCP2515_BitModify(uint8_t addr, uint8_t mask, uint8_t data)
{
    uint8_t tx[4];

    tx[0] = MCP_BITMOD;
    tx[1] = addr;
    tx[2] = mask;
    tx[3] = data;

    MCP2515_CS_LOW();
    HAL_SPI_Transmit(&hspi1, tx, 4, HAL_MAX_DELAY);
    MCP2515_CS_HIGH();
}


void MCP2515_Init(void)
{
    MCP2515_Reset();

    // Enter Configuration mode
    // verify reset worked
    while ((MCP2515_Read(CANSTAT) & 0xE0) != 0x80);

    // Bit timing for 500 kbps (8 MHz oscillator)
    MCP2515_Write(CNF1, 0x00); // SJW=1, BRP=0
    MCP2515_Write(CNF2, 0xD1); // BTLMODE=1, SAM=0, PHSEG1=6, PRSEG=2
    MCP2515_Write(CNF3, 0x01); // PHSEG2=2

    //MCP2515_Write(CNF1, 0x00);
    //MCP2515_Write(CNF2, 0x90);
    //MCP2515_Write(CNF3, 0x02);

    //MCP2515_Write(CNF1, 0x00);
    //MCP2515_Write(CNF2, 0xD0);
    //MCP2515_Write(CNF3, 0x82);

    //MCP2515_Write(CNF1, 0x00);
    //MCP2515_Write(CNF2, 0xF0);
    //MCP2515_Write(CNF3, 0x86);

    //MCP2515_Write(CNF1, 0x00);
    //MCP2515_Write(CNF2, 0xB8);
    //MCP2515_Write(CNF3, 0x05);

    // Accept all messages (mask = 0)
    MCP2515_Write(0x20, 0x00); // RXM0SIDH
    MCP2515_Write(0x21, 0x00);
    MCP2515_Write(0x24, 0x00); // RXM1SIDH
    MCP2515_Write(0x25, 0x00);

    // Force accept all (bypass filter)
    MCP2515_Write(0x60, 0x60); // RXB0CTRL
    MCP2515_Write(0x70, 0x60); // RXB1CTRL

    // RX Filters only
    for(uint8_t addr = 0x00; addr <= 0x0B; addr++)
    {
        MCP2515_Write(addr, 0x00);
    }

    //Interrupt Enable
    MCP2515_Write(CANINTE, 0x03);

    // Switch to Normal mode
    MCP2515_Write(CANCTRL, 0x00);

    // Wait for normal mode
    while ((MCP2515_Read(CANSTAT) & 0xE0) != 0x00);
}

void MCP2515_Send(uint16_t id, uint8_t *data, uint8_t len)
{
    uint8_t cmd = MCP_RTS_TX0;

    // Load Standard ID
    MCP2515_Write(TXB0SIDH, (uint8_t)(id >> 3));
    MCP2515_Write(TXB0SIDL, (uint8_t)((id & 0x07) << 5));

    // DLC
    MCP2515_Write(TXB0DLC, len & 0x0F);

    // Data bytes
    for (uint8_t i = 0; i < len; i++)
    {
        MCP2515_Write(TXB0D0 + i, data[i]);
    }

    // Request To Send
    MCP2515_CS_LOW();
    HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
    MCP2515_CS_HIGH();
}

uint8_t MCP2515_Receive(uint16_t *id, uint8_t *data, uint8_t *len)
{
    uint8_t intf = MCP2515_Read(CANINTF);
    uint8_t sidh, sidl;
    uint8_t baseAddr;

    if (intf & 0x01)   // RXB0 has message
    {
        baseAddr = RXB0SIDH;
    }
    else if (intf & 0x02) // RXB1 has message
    {
        baseAddr = RXB1SIDH;
    }
    else
    {
        return 0;
    }

    sidh = MCP2515_Read(baseAddr);
    sidl = MCP2515_Read(baseAddr + 1);

    *id = (sidh << 3) | (sidl >> 5);

    *len = MCP2515_Read(baseAddr + 4) & 0x0F;

    for (uint8_t i = 0; i < *len; i++)
    {
        data[i] = MCP2515_Read(baseAddr + 5 + i);
    }

    // Clear RX interrupt flags
    MCP2515_BitModify(CANINTF, 0x03, 0x00);

    return 1;
}
