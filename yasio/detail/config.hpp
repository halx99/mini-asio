//////////////////////////////////////////////////////////////////////////////////////////
// A cross platform socket APIs, support ios & android & wp8 & window store
// universal app
//////////////////////////////////////////////////////////////////////////////////////////
/*
The MIT License (MIT)

Copyright (c) 2012-2019 halx99

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef YASIO__CONFIG_HPP
#define YASIO__CONFIG_HPP

/*
** Uncomment or add -DYASIO_HEADER_ONLY=1 to enable yasio core implementation header only
*/
// #define YASIO_HEADER_ONLY 1

/*
** Uncomment or add -DYASIO_VERBOS_LOG=1 to enable verbos log
*/
// #define YASIO_VERBOS_LOG 1

/*
** Uncomment or add -DYASIO_DISABLE_OBJECT_POOL to disable object_pool for allocating protocol data
*unit
*/
// #define YASIO_DISABLE_OBJECT_POOL 1

/*
** Uncomment or add -DYASIO_ENABLE_KCP=1 to enable kcp support
** Remember, before thus, please ensure:
** 1. Execute: `git submodule update --init --recursive` to clone  kcp sources.
** 2. Add yasio/kcp/ikcp.c to your build system, even through the `YASIO_HEADER_ONLY` was defined.
** pitfall: yasio kcp support is experimental currently.
*/
// #define YASIO_ENABLE_KCP 1

#if defined(YASIO_HEADER_ONLY)
#  define YASIO__DECL inline
#else
#  define YASIO__DECL
#endif

#if defined(_MSC_VER) && _MSC_VER <= 1800
#  define YASIO__HAVE_NS_INLINE 0
#  define YASIO__NS_INLINE
#else
#  define YASIO__HAVE_NS_INLINE 1
#  define YASIO__NS_INLINE inline
#endif

#if defined(_WIN32)
#  define YASIO_LOG(format, ...)                                                                   \
    OutputDebugStringA(::yasio::strfmt(127, ("%s" format "\n"), "[yasio]", ##__VA_ARGS__).c_str())
#elif defined(ANDROID) || defined(__ANDROID__)
#  include <android/log.h>
#  include <jni.h>
#  define YASIO_LOG(format, ...)                                                                   \
    __android_log_print(ANDROID_LOG_INFO, "yasio", ("%s" format), "[yasio]", ##__VA_ARGS__)
#else
#  define YASIO_LOG(format, ...) printf(("%s" format "\n"), "[yasio]", ##__VA_ARGS__)
#endif

#if !defined(YASIO_VERBOS_LOG)
#  define YASIO_LOGV(fmt, ...) (void)0
#else
#  define YASIO_LOGV YASIO_LOG
#endif

#include "strfmt.hpp"

#endif
