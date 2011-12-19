#ifndef __MCDEFS_H__
#define __MCDEFS_H__

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <sys/types.h>
typedef u_int32_t UINT64;
typedef u_int32_t UINT8;
#endif

#endif