#pragma once

#ifdef WIN32

struct dirent {
	char *d_name;
};

typedef struct dirent DIR;

inline DIR *opendir(const char *path)
{
	return NULL;
}

inline struct dirent readdir(DIR *d)
{
	return NULL;
}

/* Define mkdir with mode parameter */
#define mkdir(path, mode) _mkdir((path))
#endif
