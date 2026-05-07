#ifndef CRYIF_H
#define CRYIF_H

#include "Crypto_Common.h"
#include "Cryif_cfg.h"
#include "Crypto_Hw.h"

#ifdef __cplusplus
extern "C" {
#endif

void CryIf_Init(void);

Std_ReturnType CryIf_ProcessJob(const Crypto_JobType *job);
Std_ReturnType CryIf_RandomGenerate(const Crypto_JobType *job);
Std_ReturnType CryIf_RandomSeed(const Crypto_JobType *job);

#ifdef __cplusplus
}
#endif

#endif /* CRYIF_H */
