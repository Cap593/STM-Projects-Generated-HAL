#ifndef CRYPTO_HW_CFG_H
#define CRYPTO_HW_CFG_H

#include "Crypto_Common.h"

/*
 * Crypto driver object = "what the driver can execute".
 * In a real Renesas integration, each object may map to a HW peripheral,
 * hardware accelerator, or a software fallback implementation.
 */
typedef struct
{
    uint32_t        objectId;
    Crypto_PathType  path;              /* HW or SW */
    bool             supportsRandomGenerate;
    bool             supportsRandomSeed;
    const char     *name;
} Crypto_Hw_ObjectConfigType;

typedef struct
{
    Crypto_KeyElementIdType elementId;
    uint8_t  data[32];
    uint32_t length;
} Crypto_KeyElementType;

typedef struct
{
    uint32_t keyId;
    Crypto_KeyStatusType status;
    Crypto_KeyElementType element;
    Crypto_KeyElementType ivElement;
} Crypto_KeySlotType;

typedef struct
{
    const Crypto_Hw_ObjectConfigType *objects;
    uint32_t                          numObjects;
} Crypto_Hw_ConfigType;

#define CRYPTO_HW_OBJECT_RNG   (0u)
#define CRYPTO_SW_OBJECT_RNG   (1u)

extern const Crypto_Hw_ConfigType Crypto_Hw_Config;

#endif /* CRYPTO_HW_CFG_H */
