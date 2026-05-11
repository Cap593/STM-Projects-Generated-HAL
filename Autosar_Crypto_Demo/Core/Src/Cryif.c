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

static Std_ReturnType CryIf_MapKey(uint32_t csmKeyId,uint32_t *cryptoKeyId)
{
    uint32_t i;

    for (i = 0; i < CryIf_Config.numKeys; i++)
    {
        if (CryIf_Config.keyMap[i].csmKeyId == csmKeyId)
        {
            *cryptoKeyId =
                CryIf_Config.keyMap[i].cryptoKeyId;

            return E_OK;
        }
    }

    return E_NOT_OK;
}

void CryIf_Init(void)
{
    /* Nothing to initialize in this small learning example. */
}

Std_ReturnType CryIf_ProcessJob(Crypto_JobType *job)
{
    const CryIf_ChannelConfigType *channelCfg;
    uint32_t cryptoKeyId;

    if (job == NULL)
    {
        return E_NOT_OK;
    }

    /* -----------------------------
     * Find channel
     * ----------------------------- */
    channelCfg = CryIf_FindChannel(job->channelId);

    if (channelCfg == NULL)
    {
        return E_NOT_OK;
    }

    /* -----------------------------
     * Map KeyId
     * ----------------------------- */
    if (job->service == CRYPTO_SERVICE_KEYGENERATE)
    {
    	if (CryIf_MapKey(job->keyId, &cryptoKeyId) != E_OK)
    	{
    		return E_NOT_OK;
    	}
    }

    /* -----------------------------
     * Update runtime job
     * ----------------------------- */
    job->cryptoObjectId = channelCfg->cryptoObjectId;

    /*
     * Replace virtual KeyId
     * with Crypto Driver KeyId
     */
    job->keyId = cryptoKeyId;

    return Crypto_Hw_ProcessJob(job->cryptoObjectId,job);
}

Std_ReturnType CryIf_RandomGenerate(Crypto_JobType *job)
{
    if ((job == NULL) || (job->service != CRYPTO_SERVICE_RANDOMGENERATE))
    {
        return E_NOT_OK;
    }
    return CryIf_ProcessJob(job);
}

Std_ReturnType CryIf_RandomSeed(Crypto_JobType *job)
{
    if ((job == NULL) || (job->service != CRYPTO_SERVICE_RANDOMSEED))
    {
        return E_NOT_OK;
    }
    return CryIf_ProcessJob(job);
}
