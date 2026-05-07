#include "Cryif.h"

static const CryIf_ChannelConfigType *CryIf_FindChannel(uint32_t channelId)
{
    for (uint32_t i = 0u; i < CryIf_Config.numChannels; ++i)
    {
        if (CryIf_Config.channels[i].channelId == channelId)
        {
            return &CryIf_Config.channels[i];
        }
    }
    return NULL;
}

void CryIf_Init(void)
{
    /* Nothing to initialize in this small learning example. */
}

Std_ReturnType CryIf_ProcessJob(const Crypto_JobType *job)
{
    if (job == NULL)
    {
        return E_NOT_OK;
    }

    const CryIf_ChannelConfigType *ch = CryIf_FindChannel(job->channelId);
    if (ch == NULL)
    {
        return E_NOT_OK;
    }

    /* Channel select happens here */
    return Crypto_Hw_ProcessJob(ch->cryptoObjectId, job);
}

Std_ReturnType CryIf_RandomGenerate(const Crypto_JobType *job)
{
    if ((job == NULL) || (job->service != CRYPTO_SERVICE_RANDOMGENERATE))
    {
        return E_NOT_OK;
    }
    return CryIf_ProcessJob(job);
}

Std_ReturnType CryIf_RandomSeed(const Crypto_JobType *job)
{
    if ((job == NULL) || (job->service != CRYPTO_SERVICE_RANDOMSEED))
    {
        return E_NOT_OK;
    }
    return CryIf_ProcessJob(job);
}
