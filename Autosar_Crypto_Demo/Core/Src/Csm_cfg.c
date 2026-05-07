#include "Csm_cfg.h"

/* Two example CSM jobs: one routed to HW RNG, one routed to SW RNG */
static const Csm_JobConfigType Csm_Jobs[] =
{
    {
        .jobId            = CSM_JOB_ID_HW_RNG,
        .service          = CRYPTO_SERVICE_RANDOMGENERATE,
        .cryIfChannelId   = CRYIF_CHANNEL_HW,
        .isAsynchronous   = false,
        .maxResultLength  = CRYPTO_MAX_RESULT_SIZE
    },
    {
        .jobId            = CSM_JOB_ID_SW_RNG,
        .service          = CRYPTO_SERVICE_RANDOMGENERATE,
        .cryIfChannelId   = CRYIF_CHANNEL_SW,
        .isAsynchronous   = true,
        .maxResultLength  = CRYPTO_MAX_RESULT_SIZE
    }
};

const Csm_ConfigType Csm_Config =
{
    .jobs     = Csm_Jobs,
    .numJobs  = (uint32_t)(sizeof(Csm_Jobs) / sizeof(Csm_Jobs[0]))
};
