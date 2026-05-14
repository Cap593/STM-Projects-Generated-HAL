#ifndef CSM_CFG_H
#define CSM_CFG_H

#include "Crypto_Common.h"

/*
 * CSM job configuration.
 * In a real AUTOSAR project, these values are usually generated from the tool.
 */
typedef struct
{
    uint32_t           jobId;
    Crypto_ServiceType service;
    Crypto_OperationModeType opMode;
    uint32_t           cryIfChannelId;   /* 0 = HW channel, 1 = SW channel */
    uint32_t           keyId;
    uint32_t           targetKeyId;
    bool               isAsynchronous;   /* true = queue + Csm_MainFunction */
    uint32_t           keyLength;
    uint32_t           maxResultLength;
} Csm_JobConfigType;

typedef struct
{
    const Csm_JobConfigType *jobs;
    uint32_t                  numJobs;
} Csm_ConfigType;

/* job IDs */
#define CSM_JOB_ID_HW_RNG      			(1001u)
#define CSM_JOB_ID_SW_RNG      			(1002u)

/* New key-generation job IDs */
#define CSM_JOB_ID_HW_KEYGEN    		(3001u)
#define CSM_JOB_ID_SW_KEYGEN    		(3002u)
#define CSM_JOB_ID_BOOT_MAC_KEYGEN		(3003u)
#define CSM_JOB_ID_AES_KEYGEN			(3004u)

#define CSM_JOB_ID_AES_ECB_ENC   		(3005u)
#define CSM_JOB_ID_AES_ECB_DEC   		(3006u)
#define CSM_JOB_ID_AES_CBC_ENC   		(3007u)
#define CSM_JOB_ID_AES_CBC_DEC   		(3008u)

#define CSM_JOB_ID_CMAC_GEN   			(3009u)
#define CSM_JOB_ID_CMAC_VER   			(3010u)

extern const Csm_ConfigType Csm_Config;

#endif /* CSM_CFG_H */
