/*
 * Crypto_FlashStore.c
 *
 *  Created on: 12-May-2026
 *      Author: HP
 */

#include <stddef.h>
#include "Crypto_FlashStore.h"


static uint32_t s_FlashSequence = 0u;

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

static void Crypto_Flash_FillBlankRecord(Crypto_FlashKeyRecordType *rec, uint32_t slotId)
{
    memset(rec, 0xFF, sizeof(*rec));
    rec->slotId = slotId;
    rec->magic  = 0xFFFFFFFFu;
    rec->keyId  = 0xFFFFFFFFu;
}

static void Crypto_Flash_BuildRecordFromSlot(uint32_t slotId,
                                             const Crypto_KeySlotType *slot,
                                             Crypto_FlashKeyRecordType *rec)
{
    uint32_t copyLen;
    uint32_t ivLen;

    memset(rec, 0, sizeof(*rec));

    rec->magic   = CRYPTO_FLASH_MAGIC;
    rec->slotId  = slotId;
    rec->keyId   = slot->keyId;
    rec->keyType = 0u;
    rec->status  = (slot->status == CRYPTO_KEY_VALID) ? 1u : 0u;

    copyLen = slot->element.length;
    if (copyLen > sizeof(rec->keyMaterial))
    {
        copyLen = sizeof(rec->keyMaterial);
    }
    rec->keyLength = copyLen;
    memcpy(rec->keyMaterial, slot->element.data, copyLen);

    if (slot->keyId == 104u)
    {
        ivLen = slot->ivElement.length;
        if (ivLen > sizeof(rec->iv))
        {
            ivLen = sizeof(rec->iv);
        }
        rec->ivLength = ivLen;
        memcpy(rec->iv, slot->ivElement.data, ivLen);
    }
    else
    {
        rec->ivLength = 0u;
        memset(rec->iv, 0, sizeof(rec->iv));
    }

    rec->crc32 = Crypto_Flash_CalcCrc32((const uint8_t *)rec,
                                        (uint32_t)offsetof(Crypto_FlashKeyRecordType, crc32));
}

static Std_ReturnType Crypto_Flash_WriteTable(const Crypto_FlashKeyRecordType *records)
{
    FLASH_EraseInitTypeDef erase;
    uint32_t sectorError = 0u;
    uint32_t address;

    if (records == NULL)
    {
        return E_NOT_OK;
    }

    HAL_FLASH_Unlock();

    erase.TypeErase = FLASH_TYPEERASE_SECTORS;
    erase.Sector = CRYPTO_FLASH_SECTOR;
    erase.NbSectors = 1u;
    erase.VoltageRange = FLASH_VOLTAGE_RANGE_3;

    if (HAL_FLASHEx_Erase(&erase, &sectorError) != HAL_OK)
    {
        HAL_FLASH_Lock();
        return E_NOT_OK;
    }

    address = CRYPTO_FLASH_BASE_ADDR;

    for (uint32_t i = 0u; i < CRYPTO_FLASH_SLOT_COUNT; i++)
    {
        if (Crypto_Flash_ProgramBuffer(address,
                                       (const uint8_t *)&records[i],
                                       sizeof(Crypto_FlashKeyRecordType)) != E_OK)
        {
            HAL_FLASH_Lock();
            return E_NOT_OK;
        }

        address += sizeof(Crypto_FlashKeyRecordType);
    }

    HAL_FLASH_Lock();
    return E_OK;
}

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

    if (outRec->keyLength > sizeof(outRec->keyMaterial))
    {
        return E_NOT_OK;
    }

    if (outRec->ivLength > sizeof(outRec->iv))
    {
        return E_NOT_OK;
    }

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

Std_ReturnType Crypto_Flash_SaveSlot(uint32_t keyId,
                                     const Crypto_KeySlotType *ramSlots,
                                     uint32_t ramSlotCount)
{
    Crypto_FlashKeyRecordType records[CRYPTO_FLASH_SLOT_COUNT];
    const Crypto_KeySlotType *slot = NULL;
    uint32_t slotIndex = 0xFFFFFFFFu;

    if (ramSlots == NULL)
    {
        return E_NOT_OK;
    }

    for (uint32_t i = 0u; i < ramSlotCount; i++)
    {
        if (ramSlots[i].keyId == keyId)
        {
            slot = &ramSlots[i];
            slotIndex = i;
            break;
        }
    }

    if ((slot == NULL) || (slotIndex >= CRYPTO_FLASH_SLOT_COUNT))
    {
        return E_NOT_OK;
    }

    for (uint32_t i = 0u; i < CRYPTO_FLASH_SLOT_COUNT; i++)
    {
        if (Crypto_Flash_ReadSlot(i + 1u, &records[i]) != E_OK)
        {
            Crypto_Flash_FillBlankRecord(&records[i], i + 1u);
        }
    }

    Crypto_Flash_BuildRecordFromSlot(slotIndex + 1u, slot, &records[slotIndex]);

    return Crypto_Flash_WriteTable(records);
}

/* reads the flash records into the RAM slots*/
Std_ReturnType Crypto_Flash_LoadAll(Crypto_KeySlotType *ramSlots,
                                    uint32_t ramSlotCount)
{
    Crypto_FlashKeyRecordType rec;
    Crypto_KeySlotType *slot;
    uint32_t copyLen;
    uint32_t ivLen;

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

                /* KEY_MATERIAL */
                slot->element.elementId = CRYPTO_KE_KEY_MATERIAL;
                copyLen = rec.keyLength;
                if (copyLen > sizeof(slot->element.data))
                {
                    copyLen = sizeof(slot->element.data);
                }
                memset(slot->element.data, 0, sizeof(slot->element.data));
                memcpy(slot->element.data, rec.keyMaterial, copyLen);
                slot->element.length = copyLen;

                /* IV only for KEY_1 */
                slot->ivElement.elementId = CRYPTO_KE_IV;
                ivLen = rec.ivLength;
                if (ivLen > sizeof(slot->ivElement.data))
                {
                    ivLen = sizeof(slot->ivElement.data);
                }
                memset(slot->ivElement.data, 0, sizeof(slot->ivElement.data));
                memcpy(slot->ivElement.data, rec.iv, ivLen);
                slot->ivElement.length = ivLen;
            }
        }
    }

    return E_OK;
}

Std_ReturnType Crypto_Flash_SaveAll(const Crypto_KeySlotType *ramSlots,
                                    uint32_t ramSlotCount)
{
    Crypto_FlashKeyRecordType records[CRYPTO_FLASH_SLOT_COUNT];

    if ((ramSlots == NULL) || (ramSlotCount < CRYPTO_FLASH_SLOT_COUNT))
    {
        return E_NOT_OK;
    }

    for (uint32_t i = 0u; i < CRYPTO_FLASH_SLOT_COUNT; i++)
    {
        Crypto_Flash_BuildRecordFromSlot(i + 1u, &ramSlots[i], &records[i]);
    }

    return Crypto_Flash_WriteTable(records);
}


