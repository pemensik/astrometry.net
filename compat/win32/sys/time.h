#pragma once
#ifdef WIN32
/* Windows time functions thread-safe. */
#define gmtime_r(t, s) gmtime_s((s), (t))
#define localtime_r(t, s) localtime_s((s), (t))
#endif
