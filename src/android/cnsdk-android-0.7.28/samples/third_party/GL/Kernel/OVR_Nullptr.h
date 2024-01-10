/*
 * Copyright (c) 2023 Leia Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef OVR_Nullptr_h
#define OVR_Nullptr_h

#pragma once

#include "OVR_Types.h"

//-----------------------------------------------------------------------------------
// ***** OVR_HAVE_std_nullptr_t
//
// Identifies if <cstddef.h> includes std::nullptr_t.
//
#if !defined(OVR_HAVE_std_nullptr_t) && defined(OVR_CPP11_ENABLED)
#if defined(OVR_STDLIB_LIBCPP)
#define OVR_HAVE_std_nullptr_t 1
#elif defined(OVR_STDLIB_LIBSTDCPP)
#if (__GLIBCXX__ >= 20110325) && (__GLIBCXX__ != 20110428) && (__GLIBCXX__ != 20120702)
#define OVR_HAVE_std_nullptr_t 1
#endif
#elif defined(_MSC_VER) && (_MSC_VER >= 1600) // VS2010+
#define OVR_HAVE_std_nullptr_t 1
#elif defined(__clang__)
#define OVR_HAVE_std_nullptr_t 1
#elif defined(OVR_CPP_GNUC) && (OVR_CC_VERSION >= 406) // GCC 4.6+
#define OVR_HAVE_std_nullptr_t 1
#endif
#endif

//-----------------------------------------------------------------------------------
// ***** nullptr / std::nullptr_t
//
// Declares and defines nullptr and related types.
//
#if defined(OVR_CPP_NO_NULLPTR)
namespace std {
class nullptr_t {
 public:
  template <typename T>
  operator T*() const {
    return 0;
  }

  template <typename C, typename T>
  operator T C::*() const {
    return 0;
  }

#if OVR_CPP_NO_EXPLICIT_CONVERSION_OPERATORS
  typedef void* (nullptr_t::*bool_)()
      const; // 4.12,p1. We can't portably use operator bool(){ return false; } because bool
  operator bool_() const // is convertable to int which breaks other required functionality.
  {
    return false;
  }
#else
  operator bool() const {
    return false;
  }
#endif

 private:
  void operator&() const; // 5.2.10,p9
};

inline nullptr_t nullptr_get() {
  nullptr_t n = {};
  return n;
}

#if !defined(nullptr)
#define nullptr nullptr_get()
#endif

} // namespace std

// 5.9,p2 p4
// 13.6, p13
template <typename T>
inline bool operator==(T* pT, const std::nullptr_t) {
  return pT == 0;
}

template <typename T>
inline bool operator==(const std::nullptr_t, T* pT) {
  return pT == 0;
}

template <typename T, typename U>
inline bool operator==(const std::nullptr_t, T U::*pU) {
  return pU == 0;
}

template <typename T, typename U>
inline bool operator==(T U::*pTU, const std::nullptr_t) {
  return pTU == 0;
}

inline bool operator==(const std::nullptr_t, const std::nullptr_t) {
  return true;
}

inline bool operator!=(const std::nullptr_t, const std::nullptr_t) {
  return false;
}

inline bool operator<(const std::nullptr_t, const std::nullptr_t) {
  return false;
}

inline bool operator<=(const std::nullptr_t, const std::nullptr_t) {
  return true;
}

inline bool operator>(const std::nullptr_t, const std::nullptr_t) {
  return false;
}

inline bool operator>=(const std::nullptr_t, const std::nullptr_t) {
  return true;
}

using std::nullptr_get;
using std::nullptr_t;

// Some compilers natively support C++11 nullptr but the standard library being used
// doesn't declare std::nullptr_t, in which case we provide one ourselves.
#elif !defined(OVR_HAVE_std_nullptr_t) && !defined(OVR_CPP_NO_DECLTYPE)
namespace std {
typedef decltype(nullptr) nullptr_t;
}
#endif

#endif
