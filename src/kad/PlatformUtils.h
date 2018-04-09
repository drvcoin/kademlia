/**
 *
 * MIT License
 * 
 * Copyright (c) 2018 drvcoin
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 * =============================================================================
 */

#pragma once

#include <string>

namespace kad
{
#ifndef TCHAR
  #define TCHAR char
  #define TEXT(s) s
  #define _T(s) TEXT(s)
  #define _tfopen fopen
  #define _tmkdir(s) mkdir(s, 0755)
  #define _stprintf sprintf
  #define _tstat stat
  #define _tunlink unlink
#endif

using TSTRING = std::basic_string<TCHAR>;

#if defined(WIN32) || defined(_WIN32)
  #define PATH_SEPERATOR '\\'
  #define PATH_SEPERATOR_STR "\\"
#else
  #define PATH_SEPERATOR '/'
  #define PATH_SEPERATOR_STR "/"
#endif

#ifndef UNICODE
#define _TS(s) s
#else
inline TSTRING _TS_IMPL(std::string origin)
{
  return TSTRING(origin.begin(), origin.end());
}
#define _TS(s) _TS_IMPL(s)
#endif


#define UNUSED_RESULT(expr) do { auto _v = (expr); (void)_v; } while(false)
#define UNUSED(expr) (void)(expr)


#if defined(__clang__) || defined(__GNUC__) || defined(__GNUG__)
  #ifndef likely
    #define likely(x) __builtin_expect(!!(x), 1)
  #endif
  #ifndef unlikely
    #define unlikely(x) __builtin_expect(!!(x), 0)
  #endif
#else
  #ifndef likely
    #define likely(x) (x)
  #endif
  #ifndef unlikely
    #define unlikely(x) (x)
  #endif
#endif


#ifndef PATH_MAX
#define PATH_MAX 4096
#endif
}