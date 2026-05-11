#ifndef CRYIF_H
#define CRYIF_H

#include "Crypto_Common.h"
#include "Cryif_cfg.h"
#include "Crypto_Hw.h"

#ifdef __cplusplus
extern "C" {
#endif

void CryIf_Init(void);

Std_ReturnType CryIf_ProcessJob(Crypto_JobType *job);
Std_ReturnType CryIf_RandomGenerate(Crypto_JobType *job);
Std_ReturnType CryIf_RandomSeed(Crypto_JobType *job);
Std_ReturnType CryIf_KeyElementGet(uint32_t csmKeyId,uint32_t keyElementId,uint8_t *keyElementPtr,uint32_t *keyElementLengthPtr);

#ifdef __cplusplus
}
#endif

#endif /* CRYIF_H */
