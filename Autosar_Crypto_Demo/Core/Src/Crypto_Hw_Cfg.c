#include "Crypto_Hw_Cfg.h"

static const Crypto_Hw_ObjectConfigType Crypto_Objects[] =
{
    {
        .objectId               = CRYPTO_HW_OBJECT_RNG,
        .path                   = CRYPTO_PATH_HW,
        .supportsRandomGenerate = true,
        .supportsRandomSeed     = true,
        .name                   = "STM_HW_RNG"
    },
    {
        .objectId               = CRYPTO_SW_OBJECT_RNG,
        .path                   = CRYPTO_PATH_SW,
        .supportsRandomGenerate = true,
        .supportsRandomSeed     = true,
        .name                   = "SOFTWARE_RNG_CALLBACK"
    }
};

const Crypto_Hw_ConfigType Crypto_Hw_Config =
{
    .objects   = Crypto_Objects,
    .numObjects = (uint32_t)(sizeof(Crypto_Objects) / sizeof(Crypto_Objects[0]))
};
