#pragma once

#define LOGGING_LEVEL LL_VERBOSE

typedef enum loggingLevels
{
    LL_ERROR,
    LL_WARNING,
    LL_INFO,
    LL_VERBOSE,
} loggingLevels_t;

void log_error(char *module, char *message);

void logg(loggingLevels_t level, char * module, char * format, ...);

void logg_track(loggingLevels_t level, int trackingId, char * format, ...);

