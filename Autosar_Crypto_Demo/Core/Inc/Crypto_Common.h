#ifndef CRYPTO_COMMON_H
#define CRYPTO_COMMON_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* AUTOSAR-like return values */
typedef uint8_t Std_ReturnType;

#ifndef E_OK
#define E_OK        ((Std_ReturnType)0u)
#endif

#ifndef E_NOT_OK
#define E_NOT_OK    ((Std_ReturnType)1u)
#endif

#define CRYPTO_HASH_DIGEST_SIZE   (32u)

/* Generic Crypto service model */

typedef enum
{
    CRYPTO_SERVICE_RANDOMGENERATE     = 0u,
    CRYPTO_SERVICE_RANDOMSEED         = 1u,
    CRYPTO_SERVICE_KEYGENERATE        = 2u,
    CRYPTO_SERVICE_AES_ECB_ENCRYPT    = 3u,
    CRYPTO_SERVICE_AES_ECB_DECRYPT    = 4u,
    CRYPTO_SERVICE_AES_CBC_ENCRYPT    = 5u,
    CRYPTO_SERVICE_AES_CBC_DECRYPT    = 6u,
    CRYPTO_SERVICE_CMAC_GENERATE      = 7u,
    CRYPTO_SERVICE_CMAC_VERIFY        = 8u,
    CRYPTO_SERVICE_HASH               = 9u,
	CRYPTO_SERVICE_SIGNATURE_GENERATE = 10u,
	CRYPTO_SERVICE_SIGNATURE_VERIFY   = 11u
} Crypto_ServiceType;

typedef enum
{
    CRYPTO_PATH_HW = 0u,
    CRYPTO_PATH_SW = 1u
} Crypto_PathType;

typedef enum
{
    CRYPTO_OPERATIONMODE_SINGLECALL = 0u,
    CRYPTO_OPERATIONMODE_START      = 1u,
    CRYPTO_OPERATIONMODE_UPDATE     = 2u,
    CRYPTO_OPERATIONMODE_FINISH     = 3u
} Crypto_OperationModeType;

typedef enum
{
    CRYPTO_KEY_INVALID = 0u,
    CRYPTO_KEY_VALID   = 1u
} Crypto_KeyStatusType;

typedef enum
{
    CRYPTO_KE_KEY_MATERIAL = 0u,
    CRYPTO_KE_IV           = 1u
} Crypto_KeyElementIdType;

/* Common job object passed from CSM -> CryIf -> Crypto Driver */
typedef struct
{
    uint32_t                 jobId;
    uint32_t                 channelId;
    uint32_t                 cryptoObjectId;
    uint32_t                 keyId;
    uint32_t                 targetKeyId;
    Crypto_ServiceType       service;
    Crypto_OperationModeType opMode;

    uint8_t                 *resultPtr;        /* output for encrypt / CMAC generate */
    uint32_t                *resultLengthPtr;

    const uint8_t           *inputPtr;         /* plaintext / message */
    uint32_t                 inputLength;
    uint8_t                 *outputPtr;        /* ciphertext / CMAC output */
    uint32_t                *outputLengthPtr;

    const uint8_t           *macPtr;           /* expected MAC for verify */
    uint32_t                 macLength;

    const uint8_t           *seedPtr;
    uint32_t                 seedLength;

    const uint8_t 			*signaturePtr;
    uint32_t       			signatureLength;

    uint32_t                 keyLength;
} Crypto_JobType;

/* Small helper limits for this learning scaffold */
#define CRYPTO_MAX_RESULT_SIZE   (32u)
#define CRYPTO_MAX_SEED_SIZE     (32u)

#define CRYPTO_AES_BLOCK_SIZE   (16u)

#define CSM_JOB_QUEUE_SIZE       (4u)

#define CRYIF_CHANNEL_HW   (0u)
#define CRYIF_CHANNEL_SW   (1u)

#endif /* CRYPTO_COMMON_H */
