#include "Cryif_cfg.h"
#include "Crypto_Hw_Cfg.h"

static const CryIf_ChannelConfigType CryIf_Channels[] =
{
    {
        .channelId      = CRYIF_CHANNEL_HW,
        .path           = CRYPTO_PATH_HW,
        .cryptoObjectId = CRYPTO_HW_OBJECT_RNG,
        .name           = "HW_RNG_CHANNEL"
    },
    {
        .channelId      = CRYIF_CHANNEL_SW,
        .path           = CRYPTO_PATH_SW,
        .cryptoObjectId = CRYPTO_SW_OBJECT_RNG,
        .name           = "SW_RNG_CHANNEL"
    }
};

static const CryIf_KeyMapType CryIf_KeyMap[] =
{
    {
        .csmKeyId     = 1u,
        .cryptoKeyId  = 101u
    },
    {
        .csmKeyId     = 2u,
        .cryptoKeyId  = 102u
    },
    {
        .csmKeyId     = 3u,
        .cryptoKeyId  = 103u
    },
    {
        .csmKeyId     = 4u,
        .cryptoKeyId  = 104u
    }
};

const CryIf_ConfigType CryIf_Config =
{
    .channels   = CryIf_Channels,
    .numChannels = (uint32_t)(sizeof(CryIf_Channels) / sizeof(CryIf_Channels[0])),
    .keyMap     = CryIf_KeyMap,
    .numKeys    = (uint32_t)(sizeof(CryIf_KeyMap) / sizeof(CryIf_KeyMap[0]))
};


