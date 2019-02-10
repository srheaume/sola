/*--------------------------------------------------------------------------
    FILE                :   typedef.h

    PURPOSE             :   Basic type definitions

    INITIAL CODING      :   Stephane Rheaume (SR)
    (March 20th, 2016)

        Copyright (c) Stephane Rheaume 2016, All rights reserved.
 --------------------------------------------------------------------------*/

#ifndef __TYPEDEF_H                             /* Prevent multiple includes */
#define __TYPEDEF_H

#include <stdint.h>

/*--------------------------------------------------------------------------
    Macro definitions
 --------------------------------------------------------------------------*/

#define MAKEU16(a, b) ((uint16_t)(((uint8_t)(a)) | ((uint16_t)((uint8_t)(b))) << 8))
#define MAKEU32(a, b) ((uint32_t)(((uint16_t)(a)) | ((uint32_t)((uint16_t)(b))) << 16))
#define LOU16(x)      ((uint16_t)(x))
#define HIU16(x)      ((uint16_t)(((uint32_t)(x) >> 16) & 0xFFFF))
#define LOU8(x)       ((uint8_t)(x))
#define HIU8(x)       ((uint8_t)(((uint16_t)(x) >> 8) & 0xFF))

#define SWAPU16(x)    MAKEU16(HIU8(x), LOU8(x))
#define SWAPU32(x)    MAKEU32(SWAPU16(HIU16(x)), SWAPU16(LOU16(x)))

#endif /* __TYPEDEF_H */
