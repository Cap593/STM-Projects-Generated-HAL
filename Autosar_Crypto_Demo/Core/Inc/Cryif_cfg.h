#ifndef CRYIF_CFG_H
#define CRYIF_CFG_H

#include "Crypto_Common.h"

/*
 * CryIf maps a CSM-facing channel to a concrete crypto driver object.
 * Here we keep 2 channels:
 *   0 -> hardware RNG path
 *   1 -> software RNG path
 */
typedef struct
{
    uint32_t        channelId;
    Crypto_PathType  path;
    uint32_t        cryptoObjectId; /* object used by Crypto_Hw layer */
    const char     *name;
} CryIf_ChannelConfigType;

typedef struct
{
    const CryIf_ChannelConfigType *channels;
    uint32_t                       numChannels;
} CryIf_ConfigType;

#define CRYIF_CHANNEL_HW   (0u)
#define CRYIF_CHANNEL_SW   (1u)

extern const CryIf_ConfigType CryIf_Config;

#endif /* CRYIF_CFG_H */
