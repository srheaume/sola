/*--------------------------------------------------------------------------
    FILE                :   SOLAAPI.c

    PURPOSE             :   Implementation of the synchronized overlap add
                            (SOLA) method of TSM.

    INITIAL CODING      :   Stephane Rheaume (SR)
    (March 20th, 2016)

        Copyright (c) Stephane Rheaume 2016, All rights reserved.
 --------------------------------------------------------------------------*/

#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "typedef.h"
#include "solaapi.h"

/*--------------------------------------------------------------------------
    ===> PRIVATE <===
 --------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------
    Local variables
 --------------------------------------------------------------------------*/

static uint16_t N = 160;                        /* Size of the overlapping frames */
static uint32_t lastSampleIndex;

/*--------------------------------------------------------------------------
    SOLA_GetIntervals

    Description:
        Calculates and returns the analysis (Sa) and synthesis (Ss)
        interframe intervals. The choice of Sa and Ss will depend
        on alpha and N.

    Parameters:
        alpha - Time-scale factor
        sa - Analysis interframe interval (pointer)
        ss - Synthesis interframe interval (pointer)
 --------------------------------------------------------------------------*/

static void SOLA_GetIntervals(float alpha, uint16_t *sa, uint16_t *ss)
{
    *sa = (uint16_t) ((alpha > 1.0) ? (N / (2 * alpha)) : (N / 2));
    *ss = (uint16_t) (*sa * alpha);
}

/*--------------------------------------------------------------------------
    SOLA_CrossCorrelation:

    Description:
        Computes the normalized cross-correlation between two signals.

    Parameters:
        x and y - Input and output signals
        points - Number of points used to compute
 --------------------------------------------------------------------------*/

static float SOLA_CrossCorrelation(int16_t x[], int16_t y[], uint16_t points)
{
    double   sum[3] = {0.0, 0.0, 0.0};
    double   denom;
    uint16_t j;

    for (j = 0; j < points; j++) {
        sum[0] += x[j] * y[j];                  /* Numerator */
        sum[1] += x[j] * x[j];                  /* Denominator */
        sum[2] += y[j] * y[j];
    }

    if (0.0 == (denom = sqrt(sum[1] * sum[2]))) {
        return 0.0;
    }

    return (float) (sum[0] / denom);
}

/*--------------------------------------------------------------------------
    SOLA_FindLag:

    Description:
        Find the highest cross-correlation point.

    Parameters:
        x and y - Input and output signals
        sa - Analysis interframe interval
        ss - Synthesis interframe interval
        m - Size of the overlapping frames
 --------------------------------------------------------------------------*/

static int16_t SOLA_FindLag(int16_t x[], int16_t y[], uint16_t sa, uint16_t ss, uint16_t m)
{
    int16_t  k, km = 0;
    uint16_t L;
    float    R, Rm = -1;

    k = -(((m * ss) >= (N / 2)) ? (N / 2) : ss);

    /*
     * Number of points of overlap between y(mSs+k+j) and x(mSa+j).
     */
    L = N;
    if ((uint32_t) (((m * ss) + k) + N) > lastSampleIndex) {
        L = (uint16_t) (lastSampleIndex - ((m * ss) + k));
    }

    for ( ; k <= (N / 2); k++, L--) {
        /*
         * The cross-correlation function as defined will indicate a high
         * correlation between y and x when L is small, which could lead
         * to errant synchronization. To remedy this situation, we restricted
         * L to taking on values greater than N / 8.
         */
        if (L < (N / 8))
            break;

        /*
         * Obtain the aligment by computing the normalized cross-correlation
         * between x(mSa+j) and y(mSs+k+j).
         */
        if ((R = SOLA_CrossCorrelation(&x[m * sa], &y[m * ss + k], L)) > Rm) {
            Rm = R;
            km = k;
        }
    }

    return km;
}

/*--------------------------------------------------------------------------
    SOLA_OverlapFrame

    Description:
        Weights and averages x(mSa+j) with y(mSs+km+j) along their
        points of overlap.

    Parameters:
        x and y - Input and output signals
        sa - Analysis interframe interval
        ss - Synthesis interframe interval
        m - Size of the overlapping frames
        km - Denote the lag at which Rm(k) is maximum
 --------------------------------------------------------------------------*/

static void SOLA_OverlapFrame(int16_t x[], int16_t y[], uint16_t sa, uint16_t ss, uint16_t m, int16_t km)
{
    uint16_t Lm;                                /* Range of overlap */
    uint16_t j;

    Lm = N;
    if ((uint32_t) (((m * ss) + km) + N) > lastSampleIndex) {
        Lm = (uint16_t) (lastSampleIndex - ((m * ss) + km));
    }

    for (j = 0; j < Lm; j++) {
        y[m * ss + km + j] = (int16_t) ((1 - j / Lm) * y[m * ss + km + j] + (j / Lm) * x[m * sa + j]);
    }

    if (Lm < N) {
        memcpy(&y[m * ss + km + Lm], &x[m * sa + Lm], (N - Lm) * sizeof(int16_t));
    }
}

/*--------------------------------------------------------------------------
    ===> PUBLIC <===
 --------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------
    SOLA_SetFrameSize

    Description:
        Call this function to set the size of the overlapping frames.
        The size must be greater than 1 (default = 160 ).

    Return Value:
        Nonzero if the size of the frames was set; otherwise 0.
 --------------------------------------------------------------------------*/

int SOLA_SetFrameSize(uint16_t frameSize)
{
    if (0 == frameSize) {
        return EINVAL;
    }

    N = frameSize;

    return 0;
}

/*--------------------------------------------------------------------------
    SOLA_GetFrameSize

    Description:
        Returns the size of the overlapping frames.
 --------------------------------------------------------------------------*/

uint16_t SOLA_GetFrameSize(void)
{
    return N;
}

/*--------------------------------------------------------------------------
    SOLA_TSM

    Description:
        Time-Scale Modification of speech using SOLA.
 --------------------------------------------------------------------------*/

int SOLA_TSM(int16_t x[], uint32_t xSize, int16_t *y[], uint32_t *ySize, float alpha)
{
    uint16_t sa, ss;                            /* Interframe intervals */
    uint16_t m, maxFrames;
    int16_t  km;

    /*
     * The size of the original signal must be greater than N.
     */
    if (xSize < N) {
        return EINVAL;
    }

    /*
     *  Obtain the interframe intervals (Sa & Ss).
     */
    SOLA_GetIntervals(alpha, &sa, &ss);

    /*
     * Allocate memory for synthetic signal.
     */
    *y = (int16_t *) malloc(((uint32_t) (xSize * alpha) + N) * sizeof(int16_t));
    if (NULL == *y) {
        return ENOMEM;
    }
    memset(*y, 0, ((uint32_t) (xSize * alpha) + N) * sizeof(int16_t));

    /*
     * Copy the first frame to the output signal.
     */
    memcpy(*y, x, N * sizeof(int16_t));

    /*
     * Time-Scale Modification of speech.
     */
    lastSampleIndex = N;
    maxFrames = (uint16_t) ((xSize - N) / sa);

    for (m = 1; m <= maxFrames; m++) {
        km = SOLA_FindLag(x, *y, sa, ss, m);
        SOLA_OverlapFrame(x, *y, sa, ss, m, km);

        lastSampleIndex = (m * ss) + km + N;
    }

    *ySize = lastSampleIndex;

    return 0;
}
