/*--------------------------------------------------------------------------
    FILE                :   ulawapi.h

    PURPOSE             :   Interface for ulaw API

    INITIAL CODING      :   Stephane Rheaume (SR)
    (March 20th, 2016)

        Copyright (c) Stephane Rheauume 2016, All rights reserved.
 --------------------------------------------------------------------------*/

#ifndef __ULAWAPI_H                             /* Prevent multiple includes */
#define __ULAWAPI_H

#include "typedef.h"

#ifdef __cplusplus
extern "C" {                                    /* Assume C declarations for C++ */
#endif /* __cplusplus */

/*--------------------------------------------------------------------------
    General constants and data types
 --------------------------------------------------------------------------*/

struct audio_file_header {
    uint32_t magic;                             /* Magic number */
    uint32_t dataLocation;                      /* Data location (offset) */
    uint32_t dataSize;                          /* Number of bytes of data */
    uint32_t dataFormat;                        /* Data format */
    uint32_t sampleRate;                        /* Samples per second */
    uint32_t channels;                          /* # of interleaved channels */
    uint32_t info;                              /* Information field */
    uint32_t reserved;
} __attribute__((packed));

typedef struct audio_file_header audio_file_header_t;

#define AUDIO_FILE_MAGIC_NUMBER 0x2e736e64

/*--------------------------------------------------------------------------
    Prototypes
 --------------------------------------------------------------------------*/

int ULAW_ReadFile(uint16_t channel, FILE *stream, int16_t *buffer[], uint32_t *bufferSize, uint32_t *sampleRate);
int ULAW_SaveFile(FILE *stream, int16_t buffer[], uint32_t bufferSize, uint32_t sampleRate);

#ifdef __cplusplus
}                                               /* End of extern "C" { */
#endif /* __cplusplus */

#endif /* __ULAWAPI_H */
