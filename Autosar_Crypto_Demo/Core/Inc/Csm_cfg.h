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
    uint32_t           cryIfChannelId;   /* 0 = HW channel, 1 = SW channel */
    bool               isAsynchronous;   /* true = queue + Csm_MainFunction */
    uint32_t           maxResultLength;
} Csm_JobConfigType;

typedef struct
{
    const Csm_JobConfigType *jobs;
    uint32_t                  numJobs;
} Csm_ConfigType;

/* Example job IDs */
#define CSM_JOB_ID_HW_RNG   (1001u)
#define CSM_JOB_ID_SW_RNG   (1002u)

extern const Csm_ConfigType Csm_Config;

#endif /* CSM_CFG_H */
