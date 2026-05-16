#include "Csm.h"
#include "Cryif.h"

typedef struct
{
    bool            used;
    Crypto_JobType  job;
} Csm_QueuedJobType;

static Csm_QueuedJobType Csm_Queue[CSM_JOB_QUEUE_SIZE];

static const Csm_JobConfigType *Csm_FindJobConfig(uint32_t jobId)
{
    for (uint32_t i = 0u; i < Csm_Config.numJobs; ++i)
    {
        if (Csm_Config.jobs[i].jobId == jobId)
        {
            return &Csm_Config.jobs[i];
        }
    }
    return NULL;
}

static Std_ReturnType Csm_EnqueueJob(const Crypto_JobType *job)
{
    for (uint32_t i = 0u; i < CSM_JOB_QUEUE_SIZE; ++i)
    {
        if (Csm_Queue[i].used == false)
        {
            Csm_Queue[i].used = true;
            Csm_Queue[i].job  = *job;
            return E_OK;
        }
    }
    return E_NOT_OK; /* queue full */
}

static Std_ReturnType Csm_SubmitToCryIf(Crypto_JobType *job)
{
    return CryIf_ProcessJob(job);
}

void Csm_Init(void)
{
    for (uint32_t i = 0u; i < CSM_JOB_QUEUE_SIZE; ++i)
    {
        Csm_Queue[i].used = false;
        (void)memset(&Csm_Queue[i].job, 0, sizeof(Csm_Queue[i].job));
    }
}

Std_ReturnType Csm_RandomGenerate(uint32_t jobId,
                                  uint8_t *resultPtr,
                                  uint32_t *resultLengthPtr)
{
    const Csm_JobConfigType *cfg = Csm_FindJobConfig(jobId);
    if ((cfg == NULL) || (resultPtr == NULL) || (resultLengthPtr == NULL))
    {
        return E_NOT_OK;
    }

    if (*resultLengthPtr > cfg->maxResultLength)
    {
        return E_NOT_OK;
    }

    Crypto_JobType job;
    (void)memset(&job, 0, sizeof(job));

    job.jobId           = cfg->jobId;
    job.channelId       = cfg->cryIfChannelId;
    job.service         = CRYPTO_SERVICE_RANDOMGENERATE;
    job.opMode          = CRYPTO_OPERATIONMODE_SINGLECALL;
    job.resultPtr       = resultPtr;
    job.resultLengthPtr = resultLengthPtr;

    if (cfg->isAsynchronous == true)
    {
        return Csm_EnqueueJob(&job);
    }

    return Csm_SubmitToCryIf(&job);
}

Std_ReturnType Csm_RandomSeed(uint32_t jobId,
                              const uint8_t *seedPtr,
                              uint32_t seedLength)
{
    const Csm_JobConfigType *cfg = Csm_FindJobConfig(jobId);
    if ((cfg == NULL) || (seedPtr == NULL) || (seedLength == 0u))
    {
        return E_NOT_OK;
    }

    Crypto_JobType job;
    (void)memset(&job, 0, sizeof(job));

    job.jobId     = cfg->jobId;
    job.channelId = cfg->cryIfChannelId;
    job.service   = CRYPTO_SERVICE_RANDOMSEED;
    job.opMode    = CRYPTO_OPERATIONMODE_SINGLECALL;
    job.seedPtr   = seedPtr;
    job.seedLength = seedLength;

    return Csm_SubmitToCryIf(&job);
}

Std_ReturnType CsmJobKeyGenerate
(
    uint32_t jobId,
    uint8_t *resultPtr,
    uint32_t *resultLengthPtr
)
{
    Crypto_JobType job;
    const Csm_JobConfigType *cfg;

    if ((resultPtr == NULL) ||
        (resultLengthPtr == NULL))
    {
        return E_NOT_OK;
    }

    /* -------------------------------
     * Find configured job
     * ------------------------------- */
    cfg = Csm_FindJobConfig(jobId);

    if (cfg == NULL)
    {
        return E_NOT_OK;
    }

    if (cfg->service != CRYPTO_SERVICE_KEYGENERATE)
    {
        return E_NOT_OK;
    }

    if (*resultLengthPtr < cfg->keyLength)
    {
        return E_NOT_OK;
    }

    /* -------------------------------
     * Build runtime job object
     * ------------------------------- */
    memset(&job, 0, sizeof(Crypto_JobType));

    job.jobId              = cfg->jobId;
    job.channelId          = cfg->cryIfChannelId;

    job.keyId              = cfg->keyId;
    job.targetKeyId        = cfg->targetKeyId;

    job.service            = cfg->service;
    job.opMode             = cfg->opMode;

    job.resultPtr          = resultPtr;
    job.resultLengthPtr    = resultLengthPtr;

    job.keyLength          = cfg->keyLength;

    /* -------------------------------
     * Sync vs Async
     * ------------------------------- */
    if (cfg->isAsynchronous == true)
    {
        return Csm_EnqueueJob(&job);
    }
    else
    {
        return Csm_SubmitToCryIf(&job);
    }
}

Std_ReturnType Csm_KeyElementGet(
    uint32_t keyId,
    uint32_t keyElementId,
    uint8_t *keyElementPtr,
    uint32_t *keyElementLengthPtr)
{
    if ((keyElementPtr == NULL) ||
        (keyElementLengthPtr == NULL))
    {
        return E_NOT_OK;
    }

    return CryIf_KeyElementGet(
                keyId,
                keyElementId,
                keyElementPtr,
                keyElementLengthPtr);
}

Std_ReturnType Csm_KeyElementSet(uint32_t keyId,uint32_t keyElementId,const uint8_t *keyElementPtr,uint32_t keyElementLength)
{
    if ((keyElementPtr == NULL) || (keyElementLength == 0u))
    {
        return E_NOT_OK;
    }

    return CryIf_KeyElementSet(keyId,
                               keyElementId,
                               keyElementPtr,
                               keyElementLength);
}

Std_ReturnType Csm_Encrypt(
    uint32_t jobId,
    Crypto_OperationModeType mode,
    const uint8_t *dataPtr,
    uint32_t dataLength,
    uint8_t *resultPtr,
    uint32_t *resultLengthPtr)
{
    const Csm_JobConfigType *cfg;
    Crypto_JobType job;

    /*
     * Validate pointers
     */
    if ((dataPtr == NULL) ||
        (resultPtr == NULL) ||
        (resultLengthPtr == NULL))
    {
        return E_NOT_OK;
    }

    /*
     * ECB first version:
     * SINGLECALL only
     */
    if (mode != CRYPTO_OPERATIONMODE_SINGLECALL)
    {
        return E_NOT_OK;
    }

    /*
     * ECB requires 16-byte aligned blocks
     */
    if ((dataLength == 0u) || ((dataLength % CRYPTO_AES_BLOCK_SIZE) != 0u))
    {
        return E_NOT_OK;
    }

    if (*resultLengthPtr < dataLength)
    {
        return E_NOT_OK;
    }

    /*
     * Find configured job
     */
    cfg = Csm_FindJobConfig(jobId);

    if (cfg == NULL)
    {
        return E_NOT_OK;
    }

    if ((cfg->service != CRYPTO_SERVICE_AES_ECB_ENCRYPT) && (cfg->service != CRYPTO_SERVICE_AES_CBC_ENCRYPT))
    {
        return E_NOT_OK;
    }

    /*
     * Build runtime job
     */
    memset(&job,0,sizeof(Crypto_JobType));

    job.jobId = cfg->jobId;
    job.channelId = cfg->cryIfChannelId;
    job.keyId = cfg->keyId;
    job.service = cfg->service;
    job.opMode = mode;
    job.inputPtr = dataPtr;
    job.inputLength = dataLength;
    job.outputPtr = resultPtr;
    job.outputLengthPtr = resultLengthPtr;

    /*
     * Submit to CryIf
     */
    return Csm_SubmitToCryIf(&job);
}

Std_ReturnType Csm_Decrypt(
    uint32_t jobId,
    Crypto_OperationModeType mode,
    const uint8_t *dataPtr,
    uint32_t dataLength,
    uint8_t *resultPtr,
    uint32_t *resultLengthPtr)
{
    const Csm_JobConfigType *cfg;

    Crypto_JobType job;

    /*
     * Validate pointers
     */
    if ((dataPtr == NULL) || (resultPtr == NULL) || (resultLengthPtr == NULL))
    {
        return E_NOT_OK;
    }

    /*
     * ECB first version:
     * SINGLECALL only
     */
    if (mode != CRYPTO_OPERATIONMODE_SINGLECALL)
    {
        return E_NOT_OK;
    }

    /*
     * ECB requires block-aligned input
     */
    if ((dataLength == 0u) ||
        ((dataLength % CRYPTO_AES_BLOCK_SIZE) != 0u))
    {
        return E_NOT_OK;
    }

    /*
     * Output buffer size check
     */
    if (*resultLengthPtr < dataLength)
    {
        return E_NOT_OK;
    }

    /*
     * Find configured job
     */
    cfg = Csm_FindJobConfig(jobId);

    if (cfg == NULL)
    {
        return E_NOT_OK;
    }

    /*
     * Validate service type
     */
    if ((cfg->service != CRYPTO_SERVICE_AES_ECB_DECRYPT) && (cfg->service != CRYPTO_SERVICE_AES_CBC_DECRYPT))
    {
        return E_NOT_OK;
    }

    /*
     * Build runtime job
     */
    memset(&job,0,sizeof(Crypto_JobType));

    job.jobId = cfg->jobId;
    job.channelId =cfg->cryIfChannelId;
    job.keyId = cfg->keyId;
    job.service = cfg->service;
    job.opMode = mode;
    job.inputPtr = dataPtr;
    job.inputLength = dataLength;
    job.outputPtr = resultPtr;
    job.outputLengthPtr = resultLengthPtr;


    return Csm_SubmitToCryIf(&job);
}

Std_ReturnType Csm_MacGenerate(uint32_t jobId,
                               Crypto_OperationModeType mode,
                               const uint8_t *dataPtr,
                               uint32_t dataLength,
                               uint8_t *macPtr,
                               uint32_t *macLengthPtr)
{
    const Csm_JobConfigType *cfg;
    Crypto_JobType job;

    if ((dataPtr == NULL) || (macPtr == NULL) || (macLengthPtr == NULL))
    {
        return E_NOT_OK;
    }

    if (mode != CRYPTO_OPERATIONMODE_SINGLECALL)
    {
        return E_NOT_OK;
    }

    if (dataLength == 0u)
    {
        return E_NOT_OK;
    }

    if (*macLengthPtr < 16u)
    {
        return E_NOT_OK;
    }

    cfg = Csm_FindJobConfig(jobId);
    if ((cfg == NULL) || (cfg->service != CRYPTO_SERVICE_CMAC_GENERATE))
    {
        return E_NOT_OK;
    }

    memset(&job, 0, sizeof(job));
    job.jobId           = cfg->jobId;
    job.channelId       = cfg->cryIfChannelId;
    job.keyId           = cfg->keyId;
    job.service         = cfg->service;
    job.opMode          = mode;
    job.inputPtr        = dataPtr;
    job.inputLength     = dataLength;
    job.outputPtr       = macPtr;
    job.outputLengthPtr = macLengthPtr;

    return Csm_SubmitToCryIf(&job);
}

Std_ReturnType Csm_MacVerify(uint32_t jobId,
                             Crypto_OperationModeType mode,
                             const uint8_t *dataPtr,
                             uint32_t dataLength,
                             const uint8_t *macPtr,
                             uint32_t macLength)
{
    const Csm_JobConfigType *cfg;
    Crypto_JobType job;

    if ((dataPtr == NULL) || (macPtr == NULL))
    {
        return E_NOT_OK;
    }

    if (mode != CRYPTO_OPERATIONMODE_SINGLECALL)
    {
        return E_NOT_OK;
    }

    if ((dataLength == 0u) || (macLength != 16u))
    {
        return E_NOT_OK;
    }

    cfg = Csm_FindJobConfig(jobId);
    if ((cfg == NULL) || (cfg->service != CRYPTO_SERVICE_CMAC_VERIFY))
    {
        return E_NOT_OK;
    }

    memset(&job, 0, sizeof(job));
    job.jobId       = cfg->jobId;
    job.channelId   = cfg->cryIfChannelId;
    job.keyId       = cfg->keyId;
    job.service     = cfg->service;
    job.opMode      = mode;
    job.inputPtr    = dataPtr;
    job.inputLength = dataLength;
    job.macPtr      = macPtr;
    job.macLength   = macLength;

    return Csm_SubmitToCryIf(&job);
}

Std_ReturnType Csm_Hash(uint32_t jobId,
                        Crypto_OperationModeType mode,
                        const uint8_t *dataPtr,
                        uint32_t dataLength,
                        uint8_t *resultPtr,
                        uint32_t *resultLengthPtr)
{
    const Csm_JobConfigType *cfg;
    Crypto_JobType job;

    if ((resultPtr == NULL) || (resultLengthPtr == NULL))
    {
        return E_NOT_OK;
    }

    /* Allow empty message if you want SHA-256("") support */
    if ((dataLength > 0u) && (dataPtr == NULL))
    {
        return E_NOT_OK;
    }

    if (mode != CRYPTO_OPERATIONMODE_SINGLECALL)
    {
        return E_NOT_OK;
    }

    if (*resultLengthPtr < CRYPTO_HASH_DIGEST_SIZE)
    {
        return E_NOT_OK;
    }

    cfg = Csm_FindJobConfig(jobId);
    if ((cfg == NULL) || (cfg->service != CRYPTO_SERVICE_HASH))
    {
        return E_NOT_OK;
    }

    memset(&job, 0, sizeof(job));
    job.jobId           = cfg->jobId;
    job.channelId       = cfg->cryIfChannelId;
    job.keyId           = cfg->keyId;   /* 0 for hash */
    job.service         = cfg->service;
    job.opMode          = mode;
    job.inputPtr        = dataPtr;
    job.inputLength     = dataLength;
    job.outputPtr       = resultPtr;
    job.outputLengthPtr = resultLengthPtr;

    return Csm_SubmitToCryIf(&job);
}

Std_ReturnType Csm_SignatureGenerate(
    uint32_t jobId,
    Crypto_OperationModeType mode,
    const uint8_t *dataPtr,
    uint32_t dataLength,
    uint8_t *signaturePtr,
    uint32_t *signatureLengthPtr)
{
    const Csm_JobConfigType *cfg;

    Crypto_JobType job;

    if ((dataPtr == NULL) ||
        (signaturePtr == NULL) ||
        (signatureLengthPtr == NULL))
    {
        return E_NOT_OK;
    }

    if (mode != CRYPTO_OPERATIONMODE_SINGLECALL)
    {
        return E_NOT_OK;
    }

    cfg = Csm_FindJobConfig(jobId);

    if ((cfg == NULL) ||
        (cfg->service !=
         CRYPTO_SERVICE_SIGNATURE_GENERATE))
    {
        return E_NOT_OK;
    }

    memset(&job,
           0,
           sizeof(Crypto_JobType));

    job.jobId = cfg->jobId;
    job.channelId = cfg->cryIfChannelId;
    job.service = cfg->service;
    job.opMode = mode;

    job.inputPtr = dataPtr;
    job.inputLength = dataLength;

    job.outputPtr = signaturePtr;
    job.outputLengthPtr =
        signatureLengthPtr;

    return Csm_SubmitToCryIf(&job);
}

Std_ReturnType Csm_SignatureVerify(
    uint32_t jobId,
    Crypto_OperationModeType mode,
    const uint8_t *dataPtr,
    uint32_t dataLength,
    const uint8_t *signaturePtr,
    uint32_t signatureLength)
{
    const Csm_JobConfigType *cfg;

    Crypto_JobType job;

    if ((dataPtr == NULL) ||
        (signaturePtr == NULL))
    {
        return E_NOT_OK;
    }

    if (mode != CRYPTO_OPERATIONMODE_SINGLECALL)
    {
        return E_NOT_OK;
    }

    cfg = Csm_FindJobConfig(jobId);

    if ((cfg == NULL) ||
        (cfg->service !=
         CRYPTO_SERVICE_SIGNATURE_VERIFY))
    {
        return E_NOT_OK;
    }

    memset(&job,
           0,
           sizeof(Crypto_JobType));

    job.jobId = cfg->jobId;
    job.channelId = cfg->cryIfChannelId;
    job.service = cfg->service;
    job.opMode = mode;

    job.inputPtr = dataPtr;
    job.inputLength = dataLength;

    job.signaturePtr = signaturePtr;
    job.signatureLength = signatureLength;

    return Csm_SubmitToCryIf(&job);
}

void Csm_MainFunction(void)
{
    for (uint32_t i = 0u; i < CSM_JOB_QUEUE_SIZE; ++i)
    {
        if (Csm_Queue[i].used == true)
        {
            (void)Csm_SubmitToCryIf(&Csm_Queue[i].job);
            Csm_Queue[i].used = false;
        }
    }
}
