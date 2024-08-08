/*
 * Header for base data types.
 *
 * Maintainer: Zhigang.Kang <Zhigang.Kang@orbita.com>
 *
 * Copyright (C) 2019 Orbita Inc.
 *
 */

#ifndef __BASE_TYPE_H__
#define __BASE_TYPE_H__

#include <stdint.h>
#include <stdio.h>
#ifndef NDEBUG
#include <assert.h>
#endif

#if defined(_MSC_VER)
/* MSVS doesn't define off_t, and uses _f{seek,tell}i64. */
typedef __int64 off_t;
#define fseeko		_fseeki64
#define ftello		_ftelli64
#elif defined(_WIN32)
/*
 * MinGW defines off_t as long and uses f{seek,tell}o64/off64_t for large
 * files.
 */
#define fseeko		fseeko64
#define ftello		ftello64
#define off_t		off64_t
#endif  /* _WIN32 */

typedef int8_t		i8;
typedef uint8_t		u8;
typedef int16_t		i16;
typedef uint16_t	u16;
typedef int32_t		i32;
typedef uint32_t	u32;
typedef int64_t		i64;
typedef uint64_t	u64;
typedef size_t		ptr_t;

#ifndef INLINE
#define INLINE		inline
#endif

#ifndef NULL
#ifdef __cplusplus
#define NULL		0
#else /*  */
#define NULL		((void *)0)
#endif /*  */
#endif

#ifndef __cplusplus

#ifndef bool
typedef enum {
	false = 0,
	true = 1
} bool;
#endif

enum {
	OK = 0,
	NOK = -1
};
#endif

/* ASSERT */
#ifndef NDEBUG
#define ASSERT(x)	assert(x)
#else
#define ASSERT(x)
#endif

#endif
