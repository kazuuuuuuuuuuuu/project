#ifndef _GLOBAL_DEFINE_H_
#define _GLOBAL_DEFINE_H_

#include "log/log.h"
//#define LOG_ERROR printf
//#define LOG_DEBUG printf
//#define LOG_WARN  printf

constexpr int MAX_MESSAGE_LEN = 367280;
constexpr int INVALID_U32 = 0xFFFF;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef signed char i8;
typedef signed short i16;
typedef signed int i32;
typedef signed long long i64;

typedef float r32;
typedef double r64;
typedef long double r128;

#endif
