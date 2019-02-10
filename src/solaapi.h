/*--------------------------------------------------------------------------
    FILE                :   solaapi.h

    PURPOSE             :   Interface for SOLA API

    INITIAL CODING      :   Stephane Rheaume (SR)
    (March 20th, 2016)

        Copyright (c) Stephane Rheaume 2016, All rights reserved.
 --------------------------------------------------------------------------*/

#ifndef __SOLAAPI_H                             /* Prevent multiple includes */
#define __SOLAAPI_H

#include "typedef.h"

#ifdef __cplusplus
extern "C" {                                    /* Assume C declarations for C++ */
#endif /* __cplusplus */

/*--------------------------------------------------------------------------
    Prototypes
 --------------------------------------------------------------------------*/

int SOLA_SetFrameSize(uint16_t frameSize);
uint16_t SOLA_GetFrameSize(void);
int SOLA_TSM(int16_t x[], uint32_t xSize, int16_t *y[], uint32_t *ySize, float alpha);

#ifdef __cplusplus
}                                               /* End of extern "C" { */
#endif /* __cplusplus */

#endif /* __SOLAAPI_H */
