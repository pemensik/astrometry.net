#pragma once

/* Ignore mode setting. */
#define fchmod(handle, mode) (void)0

#include <process.h>

#ifndef O_SYNC
#define O_SYNC 0
#endif

#ifndef fsync
#define fsync fsyncdata
#endif

#define getpagesize() 4096