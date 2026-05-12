/*
 * Crypto_FlashStore.c
 *
 *  Created on: 12-May-2026
 *      Author: HP
 */

#include <stddef.h>
#include "Crypto_FlashStore.h"


static uint32_t s_FlashSequence = 0u;

uint32_t Crypto_Flash_CalcCrc32(const uint8_t *data, uint32_t len)
{
    uint32_t crc = 0xFFFFFFFFu;

    if (data == NULL)
    {
        return 0u;
    }

    for (uint32_t i = 0u; i < len; i++)
    {
        crc ^= (uint32_t)data[i];

        for (uint32_t bit = 0u; bit < 8u; bit++)
        {
            if ((crc & 1u) != 0u)
            {
                crc = (crc >> 1) ^ 0xEDB88320u;
            }
            else
            {
                crc >>= 1;
            }
        }
    }

    return ~crc;
}

void Crypto_FlashStore_Init(void)
{
    s_FlashSequence = 0u;
}

static Std_ReturnType Crypto_Flash_ProgramBuffer(uint32_t address,
                                                 const uint8_t *data,
                                                 uint32_t length)
{
    uint32_t word = 0xFFFFFFFFu;

    if ((data == NULL) || (length == 0u))
    {
        return E_NOT_OK;
    }

    for (uint32_t i = 0u; i < length; i += 4u)
    {
        word = 0xFFFFFFFFu;
        memcpy(&word, &data[i], ((length - i) >= 4u) ? 4u : (length - i));

        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address + i, word) != HAL_OK)
        {
            return E_NOT_OK;
        }
    }

    return E_OK;
}

Std_ReturnType Crypto_Flash_ReadSlot(uint32_t slotId,
                                     Crypto_FlashKeyRecordType *outRec)
{
    const volatile Crypto_FlashKeyRecordType *rec;

    if ((slotId < 1u) || (slotId > CRYPTO_FLASH_SLOT_COUNT) || (outRec == NULL))
    {
        return E_NOT_OK;
    }

    rec = (const volatile Crypto_FlashKeyRecordType *)CRYPTO_FLASH_SLOT_ADDR(slotId);

    memcpy(outRec, (const void *)rec, sizeof(Crypto_FlashKeyRecordType));

    if (outRec->magic != CRYPTO_FLASH_MAGIC)
    {
        return E_NOT_OK;
    }

    if (outRec->slotId != slotId)
    {
        return E_NOT_OK;
    }

    if (outRec->length > sizeof(outRec->keyMaterial))
    {
        return E_NOT_OK;
    }

    /* Validate CRC */
    {
        uint32_t crc = Crypto_Flash_CalcCrc32((const uint8_t *)outRec,
                                             (uint32_t)offsetof(Crypto_FlashKeyRecordType, crc32));
        if (crc != outRec->crc32)
        {
            return E_NOT_OK;
        }
    }

    return E_OK;
}

Std_ReturnType Crypto_Flash_WriteSlot(
    uint32_t slotId,
    const Crypto_FlashKeyRecordType *inRec)
{
    Crypto_FlashKeyRecordType temp;

    uint32_t address;

    if ((slotId < 1u) ||
        (slotId > CRYPTO_FLASH_SLOT_COUNT) ||
        (inRec == NULL))
    {
        return E_NOT_OK;
    }

    memcpy(&temp,
           inRec,
           sizeof(Crypto_FlashKeyRecordType));

    /*
     * Ensure valid record fields
     */
    temp.magic = CRYPTO_FLASH_MAGIC;

    temp.crc32 =
        Crypto_Flash_CalcCrc32(
            (const uint8_t *)&temp,
            (uint32_t)offsetof(
                Crypto_FlashKeyRecordType,
                crc32));

    /*
     * Write ONLY this slot record.
     * No erase here anymore.
     */
    address = CRYPTO_FLASH_SLOT_ADDR(slotId);

    if (Crypto_Flash_ProgramBuffer(
            address,
            (const uint8_t *)&temp,
            sizeof(Crypto_FlashKeyRecordType)) != E_OK)
    {
        return E_NOT_OK;
    }

    return E_OK;
}

/* reads the flash records into the RAM slots*/
Std_ReturnType Crypto_Flash_LoadAll(Crypto_KeySlotType *ramSlots,
                                    uint32_t ramSlotCount)
{
    Crypto_FlashKeyRecordType rec;
    Crypto_KeySlotType *slot;
    uint32_t copyLen;

    if (ramSlots == NULL)
    {
        return E_NOT_OK;
    }

    for (uint32_t i = 1u; i <= CRYPTO_FLASH_SLOT_COUNT; i++)
    {
        if (Crypto_Flash_ReadSlot(i, &rec) == E_OK)
        {
            if (i <= ramSlotCount)
            {
                slot = &ramSlots[i - 1u];

                slot->keyId = rec.keyId;
                slot->status = (rec.status != 0u) ? CRYPTO_KEY_VALID : CRYPTO_KEY_INVALID;

                /* Minimal version: one key material element only */
                slot->element.elementId = CRYPTO_KE_KEY_MATERIAL;
                copyLen = rec.length;
                if (copyLen > sizeof(slot->element.data))
                {
                    copyLen = sizeof(slot->element.data);
                }

                memset(slot->element.data, 0, sizeof(slot->element.data));
                memcpy(slot->element.data, rec.keyMaterial, copyLen);
                slot->element.length = copyLen;
            }
        }
    }

    return E_OK;
}

Std_ReturnType Crypto_Flash_SaveAll(
    const Crypto_KeySlotType *ramSlots,
    uint32_t ramSlotCount)
{
    FLASH_EraseInitTypeDef erase;

    uint32_t sectorError = 0u;

    Crypto_FlashKeyRecordType rec;

    uint32_t copyLen;

    if (ramSlots == NULL)
    {
        return E_NOT_OK;
    }

    /*
     * STEP 1
     * Erase flash sector ONCE
     */
    HAL_FLASH_Unlock();

    erase.TypeErase = FLASH_TYPEERASE_SECTORS;
    erase.Sector = CRYPTO_FLASH_SECTOR;
    erase.NbSectors = 1u;
    erase.VoltageRange = FLASH_VOLTAGE_RANGE_3;

    if (HAL_FLASHEx_Erase(
            &erase,
            &sectorError) != HAL_OK)
    {
        HAL_FLASH_Lock();
        return E_NOT_OK;
    }

    /*
     * STEP 2
     * Write ALL slot records
     */
    for (uint32_t i = 0u;
         i < ramSlotCount;
         i++)
    {
        memset(&rec,
               0xFF,
               sizeof(Crypto_FlashKeyRecordType));

        rec.slotId = i + 1u;

        /*
         * IMPORTANT:
         * Keep real Crypto Driver key ID
         */
        rec.keyId = ramSlots[i].keyId;

        rec.keyType = 0u;

        rec.status =
            (ramSlots[i].status ==
             CRYPTO_KEY_VALID)
             ? 1u
             : 0u;

        copyLen =
            ramSlots[i].element.length;

        if (copyLen >
            sizeof(rec.keyMaterial))
        {
            copyLen =
                sizeof(rec.keyMaterial);
        }

        rec.length = copyLen;

        memcpy(rec.keyMaterial,
               ramSlots[i].element.data,
               copyLen);

        /*
         * Write this slot record
         */
        if (Crypto_Flash_WriteSlot(
                rec.slotId,
                &rec) != E_OK)
        {
            HAL_FLASH_Lock();
            return E_NOT_OK;
        }
    }

    HAL_FLASH_Lock();

    return E_OK;
}


