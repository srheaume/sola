/*--------------------------------------------------------------------------
    FILE                :   ulawapi.c

    PURPOSE             :   Source file for ulaw API

    INITIAL CODING      :   Stephane Rheaume (SR)
    (March 20th, 2016)

        Copyright (c) Stephane Rheaume 2016, All rights reserved.
 --------------------------------------------------------------------------*/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include "typedef.h"
#include "ulawapi.h"

/*--------------------------------------------------------------------------
    ===> PRIVATE <===
 --------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------
    ULAW2Linear

    Description:
        This routine converts from ulaw to linear.
 --------------------------------------------------------------------------*/

static int16_t ULAW2Linear(uint8_t ulaw)
{
    static int exp_lut[8] = { 0,132,396,924,1980,4092,8316,16764 };
    int16_t    sign, exponent, mantissa, sample;

    ulaw = ~ulaw;
    sign = (ulaw & 0x80);
    exponent = (ulaw >> 4) & 0x07;
    mantissa = ulaw & 0x0F;
    sample = exp_lut[exponent] + (mantissa << (exponent + 3));
    if (0 != sign) sample = -sample;
    return sample;
}

/*--------------------------------------------------------------------------
    Linear2ULAW

    Description:
        This routine converts from linear to ulaw.
        29 September 1989

    Craig Reese: IDA/Supercomputing Research Center
    Joe Campbell: Department of Defense

    References:
        1) CCITT Recommendation G.711  (very difficult to follow)
        2) "A New Digital Technique for Implementation of Any 
        Continuous PCM Companding Law," Villeret, Michel,
        et al. 1973 IEEE Int. Conf. on Communications, Vol 1,
        1973, pg. 11.12-11.17
        3) MIL-STD-188-113,"Interoperability and Performance Standards
        for Analog-to_Digital Conversion Techniques,"
        17 February 1987

    Input: Signed 16 bit linear sample
    Output: 8 bit ulaw sample
 --------------------------------------------------------------------------*/

#define NOZEROTRAP          /* Turn on the trap as per the MIL-STD */
#define BIAS        0x84    /* Define the add-in bias for 16 bit samples */
#define CLIP        32635

static uint8_t Linear2ULAW(int16_t sample)
{
    static int16_t exp_lut[256] = { 0,0,1,1,2,2,2,2,3,3,3,3,3,3,3,3,
                                    4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
                                    5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
                                    5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
                                    6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
                                    6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
                                    6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
                                    6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
                                    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
                                    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
                                    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
                                    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
                                    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
                                    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
                                    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
                                    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7 };

    int16_t sign, exponent, mantissa;
    uint8_t ulaw;

    /*
     * Get the sample into sign-magnitude.
     */
    sign = (sample >> 8) & 0x80;                /* Set aside the sign */
    if (0 != sign) sample = -sample;            /* Get magnitude */
    if (sample > CLIP) sample = CLIP;           /* Clip the magnitude */

    /*
     * Convert from 16 bit linear to ulaw.
     */
    sample = sample + BIAS;
    exponent = exp_lut[(sample >> 7) & 0xFF];
    mantissa = (sample >> (exponent + 3)) & 0x0F;
    ulaw = ~(sign | (exponent << 4) | mantissa);
    #ifdef ZEROTRAP
    if (0 == ulaw) ulaw = 0x02;                 /* Optional CCITT trap */
    #endif

    /*
     * Return the result.
     */
    return ulaw;
}

#ifdef LITTLE_ENDIAN
/*--------------------------------------------------------------------------
    ByteSwapHeader
 --------------------------------------------------------------------------*/

static void ByteSwapHeader(audio_file_header_t *header)
{
    header->magic = SWAPU32(header->magic);
    header->dataLocation = SWAPU32(header->dataLocation);
    header->dataSize = SWAPU32(header->dataSize);
    header->dataFormat = SWAPU32(header->dataFormat);
    header->sampleRate = SWAPU32(header->sampleRate);
    header->channels = SWAPU32(header->channels);
    header->info = SWAPU32(header->info);
}
#endif

/*--------------------------------------------------------------------------
    ===> PUBLIC <===
 --------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------
    ULAW_ReadFile

    Description:
        Reads ulaw encoded audio file.
 --------------------------------------------------------------------------*/

int ULAW_ReadFile(uint16_t channel, FILE *stream, int16_t *buffer[], uint32_t *bufferSize, uint32_t *sampleRate)
{
    audio_file_header_t header;
    uint8_t             sample;
    uint32_t            i;

    /*
     * Read the audio file header and check if it's valid.
     */
    if (1 != fread(&header, sizeof(audio_file_header_t), 1, stream)) {
        return EINVAL;
    }

#ifdef LITTLE_ENDIAN
    ByteSwapHeader(&header);
#endif

    if (AUDIO_FILE_MAGIC_NUMBER != header.magic)
        return EINVAL;
    if(/* ulaw */ 1 != header.dataFormat)
        return EINVAL;
    if (channel > header.channels)
        return EINVAL;

    *sampleRate = header.sampleRate;

    /*
     * Allocate memory for audio data.
     */
    *bufferSize = header.dataSize;
    if (NULL == (*buffer = (int16_t *) malloc(header.dataSize * sizeof(int16_t)))) {
        return ENOMEM;
    }

    /*
     * Go read the audio file.
     */
    fseek(stream, header.dataLocation, SEEK_SET);
    fseek(stream, channel - 1, SEEK_CUR);
    for (i = 0; (i < header.dataSize) && !feof(stream); i++) {
        if (1 != fread(&sample, sizeof(uint8_t), 1, stream)) {
            free(*buffer);
            fclose(stream);
            return EIO;
        }

        (*buffer)[i] = ULAW2Linear(sample);

        fseek(stream, header.channels - 1, SEEK_CUR);
    }

    return 0;
}

/*--------------------------------------------------------------------------
    ULAW_SaveFile

    Description:
        Writes mu-law encoded audio file.
 --------------------------------------------------------------------------*/

int ULAW_SaveFile(FILE *stream, int16_t buffer[], uint32_t bufferSize, uint32_t sampleRate)
{
    audio_file_header_t header;
    uint8_t             ulaw;
    uint32_t            i;

    header.magic        = AUDIO_FILE_MAGIC_NUMBER;
    header.dataLocation = sizeof(audio_file_header_t);
    header.dataSize     = bufferSize;
    header.dataFormat   = 1;
    header.sampleRate   = sampleRate;
    header.channels     = 1;
    header.info         = 0;
    header.reserved     = 0;

#ifdef LITTLE_ENDIAN
    ByteSwapHeader(&header);
#endif

    /*
     * Write the file header.
     */
    if (1 != fwrite(&header, sizeof(audio_file_header_t), 1, stream)) {
        return EIO;
    }

    for (i = 0; i < bufferSize; i++) {
        ulaw = Linear2ULAW(buffer[i]);
        if (1 != fwrite(&ulaw, sizeof(uint8_t), 1, stream)) {
            return EIO;
        }
    }

    return 0;
}
