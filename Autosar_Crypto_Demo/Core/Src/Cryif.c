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

static bool CryIf_ServiceUsesKey(Crypto_ServiceType service)
{
    switch (service)
    {
        case CRYPTO_SERVICE_KEYGENERATE:
        case CRYPTO_SERVICE_AES_ECB_ENCRYPT:
        case CRYPTO_SERVICE_AES_ECB_DECRYPT:
        case CRYPTO_SERVICE_AES_CBC_ENCRYPT:
        case CRYPTO_SERVICE_AES_CBC_DECRYPT:
        case CRYPTO_SERVICE_CMAC_GENERATE:
        case CRYPTO_SERVICE_CMAC_VERIFY:
            return true;

        case CRYPTO_SERVICE_HASH:
        default:
            return false;
    }
}
void CryIf_Init(void)
{
    /* Nothing to initialize in this small learning example. */
}

Std_ReturnType CryIf_ProcessJob(Crypto_JobType *job)
{
    const CryIf_ChannelConfigType *channelCfg;
    uint32_t cryptoKeyId = job->keyId;

    if (job == NULL)
    {
        return E_NOT_OK;
    }

    channelCfg = CryIf_FindChannel(job->channelId);
    if (channelCfg == NULL)
    {
        return E_NOT_OK;
    }

    if (CryIf_ServiceUsesKey(job->service) == true)
    {
        if (CryIf_MapKey(job->keyId, &cryptoKeyId) != E_OK)
        {
            return E_NOT_OK;
        }

        job->keyId = cryptoKeyId;
    }

    job->cryptoObjectId = channelCfg->cryptoObjectId;
    return Crypto_Hw_ProcessJob(job->cryptoObjectId, job);
}

Std_ReturnType CryIf_RandomGenerate(Crypto_JobType *job)
{
    if ((job == NULL) || (job->service != CRYPTO_SERVICE_RANDOMGENERATE))
    {
        return E_NOT_OK;
    }
    return CryIf_ProcessJob(job);
}

Std_ReturnType CryIf_KeyElementGet(
    uint32_t csmKeyId,
    uint32_t keyElementId,
    uint8_t *keyElementPtr,
    uint32_t *keyElementLengthPtr)
{
    uint32_t cryptoKeyId;

    if ((keyElementPtr == NULL) ||
        (keyElementLengthPtr == NULL))
    {
        return E_NOT_OK;
    }

    /*
     * Map virtual CSM KeyId
     * to Crypto Driver KeyId
     */
    if (CryIf_MapKey(csmKeyId,
                     &cryptoKeyId) != E_OK)
    {
        return E_NOT_OK;
    }

    return Crypto_Hw_KeyElementGet(
                cryptoKeyId,
                keyElementId,
                keyElementPtr,
                keyElementLengthPtr);
}

Std_ReturnType CryIf_KeyElementSet(uint32_t csmKeyId,
                                   uint32_t keyElementId,
                                   const uint8_t *keyElementPtr,
                                   uint32_t keyElementLength)
{
    uint32_t cryptoKeyId;

    if ((keyElementPtr == NULL) || (keyElementLength == 0u))
    {
        return E_NOT_OK;
    }

    if (CryIf_MapKey(csmKeyId, &cryptoKeyId) != E_OK)
    {
        return E_NOT_OK;
    }

    return Crypto_Hw_KeyElementSet(cryptoKeyId,
                                   keyElementId,
                                   keyElementPtr,
                                   keyElementLength);
}

Std_ReturnType CryIf_RandomSeed(Crypto_JobType *job)
{
    if ((job == NULL) || (job->service != CRYPTO_SERVICE_RANDOMSEED))
    {
        return E_NOT_OK;
    }
    return CryIf_ProcessJob(job);
}
