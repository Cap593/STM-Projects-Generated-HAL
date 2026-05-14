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

/* Generic Crypto service model */
typedef enum
{
    CRYPTO_SERVICE_RANDOMGENERATE  = 0u,
    CRYPTO_SERVICE_RANDOMSEED      = 1u,
	CRYPTO_SERVICE_KEYGENERATE     = 2u,
	CRYPTO_SERVICE_AES_ECB_ENCRYPT = 3u,
	CRYPTO_SERVICE_AES_ECB_DECRYPT = 4u,
	CRYPTO_SERVICE_AES_CBC_ENCRYPT = 5u,
	CRYPTO_SERVICE_AES_CBC_DECRYPT = 6u
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
    uint32_t                jobId;          /* CSM job ID */
    uint32_t                channelId;      /* CryIf channel selected by CSM config */
    uint32_t                cryptoObjectId; /* Crypto driver object selected by CryIf */
    uint32_t                keyId;          /* Key reference used by key-management jobs. This is a handle/reference, not the actual key bytes */
    uint32_t                targetKeyId;	/* for key copy / key derive flows, one job may need a second key reference. */
    Crypto_ServiceType      service;
    Crypto_OperationModeType opMode;

    /* For RNG */
    uint8_t                *resultPtr;
    uint32_t               *resultLengthPtr;

    /* Common input/output for crypto services like AES */
    const uint8_t          *inputPtr;
    uint32_t                inputLength;
    uint8_t                *outputPtr;
    uint32_t               *outputLengthPtr;

    const uint8_t          *seedPtr;
    uint32_t                seedLength;

    /* Used by key generation jobs */
    uint32_t                keyLength;

} Crypto_JobType;


/* Small helper limits for this learning scaffold */
#define CRYPTO_MAX_RESULT_SIZE   (32u)
#define CRYPTO_MAX_SEED_SIZE     (32u)

#define CRYPTO_AES_BLOCK_SIZE   (16u)

#define CSM_JOB_QUEUE_SIZE       (4u)

#define CRYIF_CHANNEL_HW   (0u)
#define CRYIF_CHANNEL_SW   (1u)

#endif /* CRYPTO_COMMON_H */
