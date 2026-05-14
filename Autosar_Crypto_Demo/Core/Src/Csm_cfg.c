#include "Csm_cfg.h"

/* Two example CSM jobs: one routed to HW RNG, one routed to SW RNG */
static const Csm_JobConfigType Csm_Jobs[] =
{
    {
        .jobId            = CSM_JOB_ID_HW_RNG,
        .service          = CRYPTO_SERVICE_RANDOMGENERATE,
        .cryIfChannelId   = CRYIF_CHANNEL_HW,
        .keyId            = 0u,
        .targetKeyId      = 0u,
        .isAsynchronous   = false,
        .keyLength        = 0u,
        .maxResultLength  = CRYPTO_MAX_RESULT_SIZE
    },
    {
        .jobId            = CSM_JOB_ID_SW_RNG,
        .service          = CRYPTO_SERVICE_RANDOMGENERATE,
        .cryIfChannelId   = CRYIF_CHANNEL_SW,
        .keyId            = 0u,
        .targetKeyId      = 0u,
        .isAsynchronous   = true,
        .keyLength        = 0u,
        .maxResultLength  = CRYPTO_MAX_RESULT_SIZE
    },
    {
        .jobId           = CSM_JOB_ID_HW_KEYGEN,
        .service         = CRYPTO_SERVICE_KEYGENERATE,
        .opMode          = CRYPTO_OPERATIONMODE_SINGLECALL,
        .cryIfChannelId  = CRYIF_CHANNEL_HW,
        .keyId           = 1u,
        .targetKeyId     = 0u,
        .isAsynchronous  = false,
        .keyLength       = 16u,
        .maxResultLength = 16u
    },
    {
        .jobId           = CSM_JOB_ID_SW_KEYGEN,
        .service         = CRYPTO_SERVICE_KEYGENERATE,
        .opMode          = CRYPTO_OPERATIONMODE_SINGLECALL,
        .cryIfChannelId  = CRYIF_CHANNEL_SW,
        .keyId           = 2u,
        .targetKeyId     = 0u,
        .isAsynchronous  = false,
        .keyLength       = 16u,
        .maxResultLength = 16u
    },
    {
        .jobId           = CSM_JOB_ID_BOOT_MAC_KEYGEN,
        .service         = CRYPTO_SERVICE_KEYGENERATE,
        .opMode          = CRYPTO_OPERATIONMODE_SINGLECALL,
        .cryIfChannelId  = CRYIF_CHANNEL_HW,
        .keyId           = 3u,
        .targetKeyId     = 0u,
        .isAsynchronous  = false,
        .keyLength       = 16u,
        .maxResultLength = 16u
    },
    {
        .jobId           = CSM_JOB_ID_AES_KEYGEN,
        .service         = CRYPTO_SERVICE_KEYGENERATE,
        .opMode          = CRYPTO_OPERATIONMODE_SINGLECALL,
        .cryIfChannelId  = CRYIF_CHANNEL_HW,
        .keyId           = 4u,
        .targetKeyId     = 0u,
        .isAsynchronous  = false,
        .keyLength       = 16u,
        .maxResultLength = 16u
    },
    {
        .jobId           = CSM_JOB_ID_AES_ECB_ENC,
        .service         = CRYPTO_SERVICE_AES_ECB_ENCRYPT,
        .opMode          = CRYPTO_OPERATIONMODE_SINGLECALL,
        .cryIfChannelId  = CRYIF_CHANNEL_SW,
        .keyId           = 4u,
        .targetKeyId     = 0u,
        .isAsynchronous  = false,
        .keyLength       = 0u,
        .maxResultLength = 16u
    },
    {
        .jobId           = CSM_JOB_ID_AES_ECB_DEC,
        .service         = CRYPTO_SERVICE_AES_ECB_DECRYPT,
        .opMode          = CRYPTO_OPERATIONMODE_SINGLECALL,
        .cryIfChannelId  = CRYIF_CHANNEL_SW,
        .keyId           = 4u,
        .targetKeyId     = 0u,
        .isAsynchronous  = false,
        .keyLength       = 0u,
        .maxResultLength = 16u
    },
    {
        .jobId           = CSM_JOB_ID_AES_CBC_ENC,
        .service         = CRYPTO_SERVICE_AES_CBC_ENCRYPT,
        .opMode          = CRYPTO_OPERATIONMODE_SINGLECALL,
        .cryIfChannelId  = CRYIF_CHANNEL_SW,
        .keyId           = 4u,
        .targetKeyId     = 0u,
        .isAsynchronous  = false,
        .keyLength       = 0u,
        .maxResultLength = 32u
    },
    {
        .jobId           = CSM_JOB_ID_AES_CBC_DEC,
        .service         = CRYPTO_SERVICE_AES_CBC_DECRYPT,
        .opMode          = CRYPTO_OPERATIONMODE_SINGLECALL,
        .cryIfChannelId  = CRYIF_CHANNEL_SW,
        .keyId           = 4u,
        .targetKeyId     = 0u,
        .isAsynchronous  = false,
        .keyLength       = 0u,
        .maxResultLength = 32u
    },
	{
	    .jobId           = CSM_JOB_ID_CMAC_GEN,
	    .service         = CRYPTO_SERVICE_CMAC_GENERATE,
	    .cryIfChannelId  = CRYIF_CHANNEL_SW,
	    .keyId           = 4u,
	    .targetKeyId     = 0u,
	    .isAsynchronous  = false,
	    .keyLength       = 0u,
	    .maxResultLength = 16u
	},
	{
	    .jobId           = CSM_JOB_ID_CMAC_VER,
	    .service         = CRYPTO_SERVICE_CMAC_VERIFY,
	    .cryIfChannelId  = CRYIF_CHANNEL_SW,
	    .keyId           = 4u,
	    .targetKeyId     = 0u,
	    .isAsynchronous  = false,
	    .keyLength       = 0u,
	    .maxResultLength = 16u
	}
};

const Csm_ConfigType Csm_Config =
{
    .jobs     = Csm_Jobs,
    .numJobs  = (uint32_t)(sizeof(Csm_Jobs) / sizeof(Csm_Jobs[0]))
};
