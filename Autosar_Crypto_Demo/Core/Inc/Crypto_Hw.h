#ifndef CRYPTO_HW_H
#define CRYPTO_HW_H

#include "Crypto_Common.h"
#include "Crypto_Hw_Cfg.h"
#include "stm32f4xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

void Crypto_Hw_Init(void);

Std_ReturnType Crypto_Hw_ProcessJob(uint32_t cryptoObjectId,const Crypto_JobType *job);
Std_ReturnType Crypto_Hw_KeyElementGet(uint32_t cryptoKeyId,uint32_t keyElementId,uint8_t *keyElementPtr,uint32_t *keyElementLengthPtr);

#ifdef __cplusplus
}
#endif

#endif /* CRYPTO_HW_H */
