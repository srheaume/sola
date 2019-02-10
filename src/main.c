/*--------------------------------------------------------------------------
    FILE                :   main.c

    INITIAL CODING      :   Stephane Rheaume (SR)
    (March 20th, 2016)

        Copyright (c) Stephane Rheaume 2016, All rights reserved.
 --------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------
    Revision 1.0a   ==>     (SR) compiled using gcc 4.8.4
 --------------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include "typedef.h"
#include "ulawapi.h"
#include "solaapi.h"

/*
 * If defined, prints verbose program progress when it's running ...
 */
#define VERBOSE

/*
 * Define version information and macro.
 */
#define VER_NAME        "SOLA"
#define VER_MAJOR       1
#define VER_MINOR       0
#define VER_REVISION    'a'
#define VER_COPYRIGHT   "Copyright (c) Stephane Rheaume 2016, All Rights Reserved"

#define PRINT_VERSION_INFO() (printf("%s - v%d.%d%c. Built on %s at %s\n%s\n\n", \
                                     VER_NAME, VER_MAJOR, VER_MINOR, \
                                     VER_REVISION, __DATE__, __TIME__, \
                                     VER_COPYRIGHT))

/*--------------------------------------------------------------------------
    Symbolic constants
 --------------------------------------------------------------------------*/

#define MIN_ALPHA     0.5F
#define MAX_ALPHA     2.0F
#define MIN_FRAMESIZE 25U
#define MAX_FRAMESIZE 1000U

/*--------------------------------------------------------------------------
    Error

    Description:
        Displays an error message containing the given formatted
        string.
 --------------------------------------------------------------------------*/

void Error(const char *fmt, ...)
{
    char buff[256];
    va_list args;

    va_start(args, fmt);
    vsprintf(buff, fmt, args);
    va_end(args);

    fprintf(stderr, "\nERROR: %s\n", buff);
    exit(1);
}

/*--------------------------------------------------------------------------
    fcant

    Description:
        An fopen() replacement with error trapping.
 --------------------------------------------------------------------------*/

FILE *fcant(const char *fileName, const char *mode)
{
    FILE *stream;

    if (NULL == (stream = fopen(fileName, mode))) {
        Error("Can't open %s", fileName);
    }

    return stream;
}

/*--------------------------------------------------------------------------
    Usage

    Description:
        Displays a help screen for the program.
 --------------------------------------------------------------------------*/

void Usage(void)
{
    printf("Usage: sola <source> <destination> <alpha> [<framesize>]\n");
    printf("  source       Specifies the file to be time-scale modified\n");
    printf("  destination  Specifies the filename for the new file\n");
    printf("  alpha        Specifies the time-scale factor [%0.1f to %0.1f]\n", MIN_ALPHA, MAX_ALPHA);
    printf("  framesize    Specifies the size of the overlapping frames\n");
    printf("               [%u to %u] {default = 160}\n", MIN_FRAMESIZE, MAX_FRAMESIZE);
}

/*--------------------------------------------------------------------------
    Main program
 --------------------------------------------------------------------------*/

void main(int argc, char *argv[])
{
    FILE     *srcFile;                          /* File to be time-scale modified */
    FILE     *destFile;                         /* Destination file */
    float    alpha;                             /* Time-scale factor */
    uint16_t frameSize;                         /* Size of the overlapping frames */
    uint32_t sampleRate;                        /* Samples per seconds */
    short    *x, *y;
    uint32_t xSize, ySize;

    /*
     * Display version information.
     */
#ifdef VERBOSE
    PRINT_VERSION_INFO();
#endif

    if (argc < 4) {
        Usage();
        Error("Required parameters missing");
    }

    if (argc > 5) {
        Usage();
        Error("Too many parameters");
    }

    /*
     * Parses the command line string and extracts the required
     * information.
     */
    alpha = (float) atof(argv[3]);
    if ((alpha < MIN_ALPHA) || (alpha > MAX_ALPHA)) {
        Error("<alpha> must range from %0.1f to %0.1f", MIN_ALPHA, MAX_ALPHA);
    }

    if (5 == argc) {
        frameSize = atoi(argv[4]);
        if ((frameSize < MIN_FRAMESIZE) || (frameSize > MAX_FRAMESIZE)) {
            Error("<framesize> must range from %u to %u", MIN_FRAMESIZE, MAX_FRAMESIZE);
        }
        SOLA_SetFrameSize(frameSize);
    }

    /* 
     * Perform Time-Scale Modification of speech.
     */
#ifdef VERBOSE
    printf("READING ...\n");
#endif
    srcFile = fcant(argv[1], "rb");
    if (ULAW_ReadFile(1, srcFile, &x, &xSize, &sampleRate)) {
        Error("Problem reading the file");
    }

#ifdef VERBOSE
    printf("PERFORMING Time-Scale Modification (TSM) ...\n");
#endif
    if (SOLA_TSM(x, xSize, &y, &ySize, alpha)) {
        Error("Not enough memory or\nthe size of the original signal is smaller than <framesize = %u>", SOLA_GetFrameSize());
    }

#ifdef VERBOSE
    printf("WRITING ...\n");
#endif
    destFile = fcant(argv[2], "wb");
    if (ULAW_SaveFile(destFile, y, ySize, sampleRate)) {
        free(x);
        Error("Problem writing the file");
    }

    /*
     * Free memory blocks that were previously allocated.
     */
    free(x);
    free(y);

    /*
     * Display the report
     */
    printf("\nSOLA report:\n" );
    printf("  Time-scale factor:       %0.2f\n", alpha);
    printf("  Frame size:              %u\n", SOLA_GetFrameSize());
    printf("  Number of bytes read:    %lu\n", xSize + sizeof(audio_file_header_t));
    printf("  Number of bytes written: %lu\n", ySize + sizeof(audio_file_header_t));

    /*
     * Close all open streams.
     */
    fclose(srcFile);
    fclose(destFile);

    exit(0);
}
