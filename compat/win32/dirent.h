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

inline struct dirent *readdir(DIR *d)
{
	return NULL;
}

inline int closedir(DIR* dirp)
{
	return -1;
}
/* Define mkdir with mode parameter */
int _mkdir(const char* dirname);
#define mkdir(path, mode) _mkdir((path))
#endif
