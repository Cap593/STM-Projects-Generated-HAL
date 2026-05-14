/*
 * Crypto_FlashStore.h
 *
 *  Created on: 12-May-2026
 *      Author: HP
 */

#ifndef CRYPTO_FLASHSTORE_H
#define CRYPTO_FLASHSTORE_H

#include "Crypto_Common.h"
#include "Crypto_Hw_Cfg.h"
#include "stm32f4xx_hal.h"
#include <stdbool.h>
#include <string.h>

#define CRYPTO_FLASH_IV_SIZE    (16u)

/*
 * Minimal flash-backed record for one key slot.
 * One slot = one record.
 */

typedef struct
{
    uint32_t magic;        /* record validity marker */
    uint32_t slotId;       /* 1..4 */
    uint32_t keyId;        /* 101..104 */
    uint32_t keyType;      /* SECRET / MASTER / BOOT_MAC / AES */
    uint32_t status;       /* VALID / INVALID */

    uint32_t keyLength;
    uint8_t  keyMaterial[32];

    uint32_t ivLength;
    uint8_t  iv[CRYPTO_FLASH_IV_SIZE];

    uint32_t crc32;        /* simple integrity check */
} Crypto_FlashKeyRecordType;

/* Fixed 4-slot demo */
#define CRYPTO_FLASH_SLOT_COUNT     (4u)

/*
 * Reserve one flash sector for the 4 records.
 * Move this later if your linker/map uses a different sector.
 */
#define CRYPTO_FLASH_BASE_ADDR      (0x080E0000u)
#define CRYPTO_FLASH_SECTOR         (FLASH_SECTOR_11)
#define CRYPTO_FLASH_RECORD_SIZE    (sizeof(Crypto_FlashKeyRecordType))

#define CRYPTO_FLASH_SLOT_ADDR(slotId) \
    (CRYPTO_FLASH_BASE_ADDR + (((slotId) - 1u) * CRYPTO_FLASH_RECORD_SIZE))

/* Record validity marker */
#define CRYPTO_FLASH_MAGIC          (0x4B534C31u)  /* "KSL1" */
#define CRYPTO_FLASH_VERSION        (0x00010001u)

/* Flash store API */
void Crypto_FlashStore_Init(void);

Std_ReturnType Crypto_Flash_ReadSlot(uint32_t slotId,
                                     Crypto_FlashKeyRecordType *outRec);

Std_ReturnType Crypto_Flash_SaveSlot(uint32_t keyId,
                                     const Crypto_KeySlotType *ramSlots,
                                     uint32_t ramSlotCount);

Std_ReturnType Crypto_Flash_LoadAll(Crypto_KeySlotType *ramSlots,
                                    uint32_t ramSlotCount);

Std_ReturnType Crypto_Flash_SaveAll(const Crypto_KeySlotType *ramSlots,
                                    uint32_t ramSlotCount);

uint32_t Crypto_Flash_CalcCrc32(const uint8_t *data, uint32_t len);

#endif /* CRYPTO_FLASHSTORE_H */
