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
