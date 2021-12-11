#pragma once

#ifdef WIN32

/* Needed for struct timeval */
#include <WinSock2.h>
/* windows defines it, but we want it defined in astrometry/errors.h */
#ifdef ERROR
#undef ERROR
#endif

/* Compatibility fake calls for stellarsolver astrometry.net compatibility */
typedef long rlim_t;

struct rlimit {
    rlim_t rlim_cur;
    rlim_t rlim_max;
};
struct rusage {
    struct timeval ru_utime; /* user CPU time used */
    struct timeval ru_stime; /* system CPU time used */
};

inline int getrlimit(int resource, struct rlimit *rlim) {
    rlim->rlim_cur = rlim->rlim_max = -1;
    return -1;
}

inline int setrlimit(int resource, const struct rlimit *rlim)
{
    return -1;
}

inline int getrusage(int who, struct rusage* usage)
{
    return -1;
}

#define RLIMIT_NOFILE 0
#define RUSAGE_SELF 1

#endif
