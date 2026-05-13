#include "Crypto_Hw.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"
#include "mbedtls/aes.h"
#include "Crypto_FlashStore.h"

/*
 * This file is intentionally written as a learning scaffold.
 * The HW implementation below is a placeholder for the Renesas RH850-specific API.
 * Replace the two generator/seed functions with the vendor driver calls.
 */

extern RNG_HandleTypeDef hrng;
static mbedtls_entropy_context entropy;
static mbedtls_ctr_drbg_context ctr_drbg;
static uint8_t drbg_initialized = 0;

typedef struct
{
    bool     initialized;
    uint32_t state;
} Crypto_RngStateType;

static Crypto_RngStateType Crypto_State[2] =
{
    { true, 0x13572468u },
    { true, 0x24681357u }
};

static const Crypto_Hw_ObjectConfigType *Crypto_Hw_FindObject(uint32_t objectId)
{
    for (uint32_t i = 0u; i < Crypto_Hw_Config.numObjects; ++i)
    {
        if (Crypto_Hw_Config.objects[i].objectId == objectId)
        {
            return &Crypto_Hw_Config.objects[i];
        }
    }
    return NULL;
}

Crypto_KeySlotType Crypto_KeySlots[] =
{
    {
/* SLOT 1 --> SECRET_KEY */
        .keyId = 101u,
        .status = CRYPTO_KEY_INVALID,
        .element =
        {
            .elementId = CRYPTO_KE_KEY_MATERIAL,
			.data = {0u},
            .length = 0u
        }
    },
    {
/* SLOT 2 --> MASTER_KEY */
        .keyId = 102u,
        .status = CRYPTO_KEY_INVALID,
        .element =
        {
            .elementId = CRYPTO_KE_KEY_MATERIAL,
			.data = {0u},
            .length = 0u
        }
    },
    {
/* SLOT 3 --> BOOT_MAC_KEY */
        .keyId = 103u,
        .status = CRYPTO_KEY_INVALID,
        .element =
        {
            .elementId = CRYPTO_KE_KEY_MATERIAL,
			.data = {0u},
            .length = 0u
        }
    },
    {
/* SLOT 4 --> KEY_1 */
        .keyId = 104u,
        .status = CRYPTO_KEY_INVALID,
        .element =
        {
            .elementId = CRYPTO_KE_KEY_MATERIAL,
			.data = {0u},
            .length = 0u
        }
    }
};

static Crypto_KeySlotType* Crypto_FindKeySlot(uint32_t keyId)
{
    uint32_t i;

    for (i = 0; i < (sizeof(Crypto_KeySlots) /
                      sizeof(Crypto_KeySlots[0])); i++)
    {
        if (Crypto_KeySlots[i].keyId == keyId)
        {
            return &Crypto_KeySlots[i];
        }
    }

    return NULL;
}

static uint32_t Crypto_XorShift32(uint32_t x)
{
    x ^= (x << 13);
    x ^= (x >> 17);
    x ^= (x << 5);
    return x;
}

static Std_ReturnType Crypto_GenerateBytes(Crypto_RngStateType *state,
                                           uint8_t *out,
                                           uint32_t *outLen,
                                           uint8_t salt)
{
    if ((state == NULL) || (out == NULL) || (outLen == NULL))
    {
        return E_NOT_OK;
    }

    uint32_t n = *outLen;
    if (n > CRYPTO_MAX_RESULT_SIZE)
    {
        return E_NOT_OK;
    }

    uint32_t s = state->state;
    for (uint32_t i = 0u; i < n; ++i)
    {
        s = Crypto_XorShift32(s + (uint32_t)salt + i);
        out[i] = (uint8_t)(s & 0xFFu);
    }

    state->state = s;
    return E_OK;
}

static Std_ReturnType Crypto_SeedState(Crypto_RngStateType *state,
                                       const uint8_t *seed,
                                       uint32_t seedLen,
                                       uint32_t mix)
{
    if ((state == NULL) || (seed == NULL) || (seedLen == 0u))
    {
        return E_NOT_OK;
    }

    uint32_t s = state->state ^ mix;
    for (uint32_t i = 0u; i < seedLen; ++i)
    {
        s ^= ((uint32_t)seed[i] << ((i % 4u) * 8u));
        s = Crypto_XorShift32(s);
    }

    state->state = s;
    return E_OK;
}

/* Placeholder for the real RH850F1KMS1 hardware call */
static Std_ReturnType Crypto_GenerateRandom_HW(uint8_t *out, uint32_t *outLen)
{
    if ((out == NULL) || (outLen == NULL))
        return E_NOT_OK;

    uint32_t remaining = *outLen;
    uint32_t index = 0;

    while (remaining > 0)
    {
        uint32_t rand;

        if (HAL_RNG_GenerateRandomNumber(&hrng, &rand) != HAL_OK)
        {
            return E_NOT_OK;
        }

        uint32_t copySize = (remaining >= 4) ? 4 : remaining;

        memcpy(&out[index], &rand, copySize);

        index += copySize;
        remaining -= copySize;
    }

    return E_OK;
}

/* Placeholder for software fallback */
static Std_ReturnType Crypto_GenerateRandom_SW(uint8_t *out, uint32_t *outLen)
{
    if ((out == NULL) || (outLen == NULL) || (drbg_initialized == 0))
    {
        return E_NOT_OK;
    }

    if (mbedtls_ctr_drbg_random(&ctr_drbg, out, *outLen) != 0)
    {
        return E_NOT_OK;
    }

    return E_OK;
}

static Std_ReturnType Crypto_SeedRandom_HW(const uint8_t *seed, uint32_t seedLen)
{
    return Crypto_SeedState(&Crypto_State[0], seed, seedLen, 0x11111111u);
}

static Std_ReturnType Crypto_SeedRandom_SW(const uint8_t *seed, uint32_t seedLen)
{
    return Crypto_SeedState(&Crypto_State[1], seed, seedLen, 0x22222222u);
}

/* Crypto HW Key Generation */
static Std_ReturnType Crypto_GenerateKey_HW(Crypto_KeySlotType *slot,uint32_t keyLength)
{
    uint32_t i;
    uint32_t rand;

    if (slot == NULL)
    {
        return E_NOT_OK;
    }

    for (i = 0; i < keyLength; i += 4)
    {
        if (HAL_RNG_GenerateRandomNumber(&hrng, &rand) != HAL_OK)
        {
            return E_NOT_OK;
        }

        memcpy(&slot->element.data[i],
               &rand,
               ((keyLength - i) >= 4u) ? 4u :
                                         (keyLength - i));
    }

    slot->element.length = keyLength;

    return E_OK;
}

/* Crypto SW Key Generation */
static Std_ReturnType Crypto_GenerateKey_SW(Crypto_KeySlotType *slot,uint32_t keyLength)
{
    if (slot == NULL)
    {
        return E_NOT_OK;
    }

    if (mbedtls_ctr_drbg_random(&ctr_drbg,
                                slot->element.data,
                                keyLength) != 0)
    {
        return E_NOT_OK;
    }

    slot->element.length = keyLength;

    return E_OK;
}

void Crypto_Hw_Init(void)
{
    Crypto_State[0].initialized = true;
    Crypto_State[1].initialized = true;

    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_init(&ctr_drbg);

    if (mbedtls_ctr_drbg_seed(&ctr_drbg,
                             mbedtls_entropy_func,
                             &entropy,
                             NULL, 0) == 0)
    {
        drbg_initialized = 1;
    }
    else
    {
        drbg_initialized = 0;
    }

    /*
    * Initialize flash store module
    */
    Crypto_FlashStore_Init();

    /*
    * Load persistent flash slots
    * into RAM key slots
    */
    (void)Crypto_Flash_LoadAll(Crypto_KeySlots,CRYPTO_FLASH_SLOT_COUNT);

    /*
     * Minimal factory SECRET_KEY
     */
    if (Crypto_KeySlots[0].status != CRYPTO_KEY_VALID)
    {
        uint8_t secretKey[16] =
        {
            0x11,0x22,0x33,0x44,
            0x55,0x66,0x77,0x88,
            0x99,0xAA,0xBB,0xCC,
            0xDD,0xEE,0xFF,0x10
        };

        memcpy(Crypto_KeySlots[0].element.data,
               secretKey,
               sizeof(secretKey));

        Crypto_KeySlots[0].element.length =
            sizeof(secretKey);

        Crypto_KeySlots[0].status =
            CRYPTO_KEY_VALID;
    }
}

Std_ReturnType Crypto_Hw_KeyElementGet(
    uint32_t cryptoKeyId,
    uint32_t keyElementId,
    uint8_t *keyElementPtr,
    uint32_t *keyElementLengthPtr)
{
    Crypto_KeySlotType *slot;

    if ((keyElementPtr == NULL) ||
        (keyElementLengthPtr == NULL))
    {
        return E_NOT_OK;
    }

    slot = Crypto_FindKeySlot(cryptoKeyId);

    if (slot == NULL)
    {
        return E_NOT_OK;
    }

    /*
     * Key must be VALID
     */
    if (slot->status != CRYPTO_KEY_VALID)
    {
        return E_NOT_OK;
    }

    /*
     * Verify requested element
     */
    if (slot->element.elementId != keyElementId)
    {
        return E_NOT_OK;
    }

    /*
     * Verify output buffer size
     */
    if (*keyElementLengthPtr <
         slot->element.length)
    {
        return E_NOT_OK;
    }

    memcpy(keyElementPtr,
           slot->element.data,
           slot->element.length);

    *keyElementLengthPtr =
        slot->element.length;

    return E_OK;
}

Std_ReturnType Crypto_Hw_KeyElementSet(uint32_t cryptoKeyId,
                                       uint32_t keyElementId,
                                       const uint8_t *keyElementPtr,
                                       uint32_t keyElementLength)
{
    Crypto_KeySlotType *slot;

    if ((keyElementPtr == NULL) || (keyElementLength == 0u))
    {
        return E_NOT_OK;
    }

    /* Keep SECRET_KEY fixed/provisioned */
    if (cryptoKeyId == 101u)
    {
        return E_NOT_OK;
    }

    if (keyElementId != CRYPTO_KE_KEY_MATERIAL)
    {
        return E_NOT_OK;
    }

    slot = Crypto_FindKeySlot(cryptoKeyId);
    if (slot == NULL)
    {
        return E_NOT_OK;
    }

    if (keyElementLength > sizeof(slot->element.data))
    {
        return E_NOT_OK;
    }

    slot->element.elementId = keyElementId;
    memset(slot->element.data, 0, sizeof(slot->element.data));
    memcpy(slot->element.data, keyElementPtr, keyElementLength);
    slot->element.length = keyElementLength;
    slot->status = CRYPTO_KEY_VALID;

    /* Persist the updated slot table to flash */
    if (Crypto_Flash_SaveSlot(cryptoKeyId,Crypto_KeySlots,CRYPTO_FLASH_SLOT_COUNT) != E_OK)
    {
        return E_NOT_OK;
    }

    return E_OK;
}

static Std_ReturnType Crypto_AesEcb_ProcessBuffer(const uint8_t *key,
                                                  uint32_t keyBits,
                                                  const uint8_t *input,
                                                  uint32_t inputLength,
                                                  uint8_t *output,
                                                  bool encrypt)
{
    mbedtls_aes_context aes;
    int ret;

    if ((key == NULL) || (input == NULL) || (output == NULL))
    {
        return E_NOT_OK;
    }

    if ((inputLength == 0u) || ((inputLength % CRYPTO_AES_BLOCK_SIZE) != 0u))
    {
        return E_NOT_OK;
    }

    mbedtls_aes_init(&aes);

    ret = encrypt ? mbedtls_aes_setkey_enc(&aes, key, keyBits)
                  : mbedtls_aes_setkey_dec(&aes, key, keyBits);

    if (ret != 0)
    {
        mbedtls_aes_free(&aes);
        return E_NOT_OK;
    }

    for (uint32_t offset = 0u; offset < inputLength; offset += CRYPTO_AES_BLOCK_SIZE)
    {
        ret = mbedtls_aes_crypt_ecb(&aes,
                                    encrypt ? MBEDTLS_AES_ENCRYPT : MBEDTLS_AES_DECRYPT,
                                    input + offset,
                                    output + offset);

        if (ret != 0)
        {
            mbedtls_aes_free(&aes);
            return E_NOT_OK;
        }
    }

    mbedtls_aes_free(&aes);
    return E_OK;
}

Std_ReturnType Crypto_Hw_ProcessJob(uint32_t cryptoObjectId,const Crypto_JobType *job)
{
    const Crypto_Hw_ObjectConfigType *obj = Crypto_Hw_FindObject(cryptoObjectId);
    if ((obj == NULL) || (job == NULL))
    {
        return E_NOT_OK;
    }

    if (job->service == CRYPTO_SERVICE_RANDOMGENERATE)
    {
        /* This example keeps RNG as SINGLECALL, but we still show the mode check. */
        if (job->opMode != CRYPTO_OPERATIONMODE_SINGLECALL)
        {
            return E_NOT_OK;
        }

        if (obj->supportsRandomGenerate == false)
        {
            return E_NOT_OK;
        }

        if (job->resultPtr == NULL)
        {
            return E_NOT_OK;
        }

        if (obj->path == CRYPTO_PATH_HW)
        {
            return Crypto_GenerateRandom_HW(job->resultPtr, job->resultLengthPtr);
        }
        else
        {
            return Crypto_GenerateRandom_SW(job->resultPtr, job->resultLengthPtr);
        }
    }
    else if (job->service == CRYPTO_SERVICE_RANDOMSEED)
    {
        if (obj->supportsRandomSeed == false)
        {
            return E_NOT_OK;
        }

        if ((job->seedPtr == NULL) || (job->seedLength == 0u))
        {
            return E_NOT_OK;
        }

        if (obj->path == CRYPTO_PATH_HW)
        {
            return Crypto_SeedRandom_HW(job->seedPtr, job->seedLength);
        }
        else
        {
            return Crypto_SeedRandom_SW(job->seedPtr, job->seedLength);
        }
    }
    else if (job->service == CRYPTO_SERVICE_KEYGENERATE)
    {
    	Crypto_KeySlotType *slot;
    	Std_ReturnType ret = E_NOT_OK;

    	slot = Crypto_FindKeySlot(job->keyId);

    	if (slot == NULL)
    	{
    		return E_NOT_OK;
    	}

    	if (obj->path == CRYPTO_PATH_HW)
    	{
    		ret = Crypto_GenerateKey_HW
    				(
    						slot,
							job->keyLength
    				);
    	}
    	else
    	{
    		ret = Crypto_GenerateKey_SW
    				(
    						slot,
							job->keyLength
    				);
    	}

    	if (ret == E_OK)
    	{
        slot->status = CRYPTO_KEY_VALID;

        if ((job->resultPtr == NULL) ||
            (job->resultLengthPtr == NULL))
        {
            return E_NOT_OK;
        }

        if (*(job->resultLengthPtr) < slot->element.length)
        {
            return E_NOT_OK;
        }

        memcpy(job->resultPtr,
               slot->element.data,
               slot->element.length);

        *(job->resultLengthPtr) =
            slot->element.length;
    	}

    	return ret;
    }
    else if ((job->service == CRYPTO_SERVICE_AES_ECB_ENCRYPT) ||
             (job->service == CRYPTO_SERVICE_AES_ECB_DECRYPT))
    {
        Crypto_KeySlotType *slot;
        bool encrypt = (job->service == CRYPTO_SERVICE_AES_ECB_ENCRYPT);

        if ((job->inputPtr == NULL) ||
            (job->outputPtr == NULL) ||
            (job->outputLengthPtr == NULL))
        {
            return E_NOT_OK;
        }

        if (obj->path != CRYPTO_PATH_SW)
        {
            return E_NOT_OK;
        }

        if ((job->inputLength == 0u) ||
            ((job->inputLength % CRYPTO_AES_BLOCK_SIZE) != 0u))
        {
            return E_NOT_OK;
        }

        if (*(job->outputLengthPtr) < job->inputLength)
        {
            return E_NOT_OK;
        }

        slot = Crypto_FindKeySlot(job->keyId);
        if ((slot == NULL) ||
            (slot->status != CRYPTO_KEY_VALID) ||
            (slot->element.length != 16u) ||
            (slot->element.elementId != CRYPTO_KE_KEY_MATERIAL))
        {
            return E_NOT_OK;
        }

        if (Crypto_AesEcb_ProcessBuffer(slot->element.data,
                                        128u,
                                        job->inputPtr,
                                        job->inputLength,
                                        job->outputPtr,
                                        encrypt) != E_OK)
        {
            return E_NOT_OK;
        }

        *(job->outputLengthPtr) = job->inputLength;
        return E_OK;
    }
    else
    {
        return E_NOT_OK;
    }

}
