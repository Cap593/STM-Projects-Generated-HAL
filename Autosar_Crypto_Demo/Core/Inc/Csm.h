#ifndef CSM_H
#define CSM_H

#include "Crypto_Common.h"
#include "Csm_cfg.h"

#ifdef __cplusplus
extern "C" {
#endif

void Csm_Init(void);

/* RNG services */
Std_ReturnType Csm_RandomGenerate(uint32_t jobId,
                                  uint8_t *resultPtr,
                                  uint32_t *resultLengthPtr);

Std_ReturnType Csm_RandomSeed(uint32_t jobId,
                              const uint8_t *seedPtr,
                              uint32_t seedLength);

/* Simple scheduler for asynchronous jobs */
void Csm_MainFunction(void);

Std_ReturnType CsmJobKeyGenerate(uint32_t jobId,uint8_t *resultPtr,uint32_t *resultLengthPtr);
Std_ReturnType Csm_KeyElementGet(uint32_t keyId,uint32_t keyElementId,uint8_t *keyElementPtr,uint32_t *keyElementLengthPtr);
Std_ReturnType Csm_KeyElementSet(uint32_t keyId,uint32_t keyElementId,const uint8_t *keyElementPtr,uint32_t keyElementLength);

#ifdef __cplusplus
}
#endif

#endif /* CSM_H */
