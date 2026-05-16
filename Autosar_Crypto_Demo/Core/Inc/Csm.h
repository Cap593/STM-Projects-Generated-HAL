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
Std_ReturnType Csm_Encrypt(uint32_t jobId,
                           Crypto_OperationModeType mode,
                           const uint8_t *dataPtr,
                           uint32_t dataLength,
                           uint8_t *resultPtr,
                           uint32_t *resultLengthPtr);

Std_ReturnType Csm_Decrypt(uint32_t jobId,
                           Crypto_OperationModeType mode,
                           const uint8_t *dataPtr,
                           uint32_t dataLength,
                           uint8_t *resultPtr,
                           uint32_t *resultLengthPtr);

Std_ReturnType Csm_MacGenerate(uint32_t jobId,
                               Crypto_OperationModeType mode,
                               const uint8_t *dataPtr,
                               uint32_t dataLength,
                               uint8_t *macPtr,
                               uint32_t *macLengthPtr);

Std_ReturnType Csm_MacVerify(uint32_t jobId,
                             Crypto_OperationModeType mode,
                             const uint8_t *dataPtr,
                             uint32_t dataLength,
                             const uint8_t *macPtr,
                             uint32_t macLength);

Std_ReturnType Csm_Hash(uint32_t jobId,
                        Crypto_OperationModeType mode,
                        const uint8_t *dataPtr,
                        uint32_t dataLength,
                        uint8_t *resultPtr,
                        uint32_t *resultLengthPtr);

Std_ReturnType Csm_SignatureGenerate(
    uint32_t jobId,
    Crypto_OperationModeType mode,
    const uint8_t *dataPtr,
    uint32_t dataLength,
    uint8_t *signaturePtr,
    uint32_t *signatureLengthPtr);

Std_ReturnType Csm_SignatureVerify(
    uint32_t jobId,
    Crypto_OperationModeType mode,
    const uint8_t *dataPtr,
    uint32_t dataLength,
    const uint8_t *signaturePtr,
    uint32_t signatureLength);

#ifdef __cplusplus
}
#endif

#endif /* CSM_H */
