#pragma once
#ifdef WIN32
/* Windows time functions thread-safe. */
#define gmtime_r(t, s) gmtime_s((s), (t))
#define localtime_r(t, s) localtime_s((s), (t))

struct timezone;
struct timeval;
/* implemented in compat/win32/gettimeofday.c */
int gettimeofday(struct timeval* tp, struct timezone* tzp);
#endif
