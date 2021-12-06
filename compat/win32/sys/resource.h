#pragma once

#ifdef WIN32
/* Compatibility fake calls for stellarsolver astrometry.net compatibility */
typedef long rlim_t;

struct rlimit {
    rlim_t rlim_cur;
    rlim_t rlim_max;
};

inline int getrlimit(int resource, struct rlimit *rlim) {
    rlim->rlim_cur = rlim->rlim_max = -1;
    return -1;
}

inline int setrlimit(int resource, const struct rlimit *rlim)
{
    return -1;
}

#endif
