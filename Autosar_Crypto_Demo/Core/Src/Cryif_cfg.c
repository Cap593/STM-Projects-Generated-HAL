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

const CryIf_ConfigType CryIf_Config =
{
    .channels   = CryIf_Channels,
    .numChannels = (uint32_t)(sizeof(CryIf_Channels) / sizeof(CryIf_Channels[0]))
};
