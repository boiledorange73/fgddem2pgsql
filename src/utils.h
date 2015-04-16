#ifndef _UTILS_HEAD_
#define _UTILS_HEAD_

#if defined(_WIN32) || defined(_WIN64)
#define sscanf sscanf_s
#define fopen fopen_s
#endif

#endif /* utils.h */