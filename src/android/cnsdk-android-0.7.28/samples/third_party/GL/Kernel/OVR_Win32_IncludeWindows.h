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

#ifndef OVR_Win32_IncludeWindows_h
#define OVR_Win32_IncludeWindows_h

#include "OVR_Types.h"

// Automatically avoid including the Windows header on non-Windows platforms.
#ifdef OVR_OS_MS

// It is common practice to define WIN32_LEAN_AND_MEAN to reduce compile times.
// However this then requires us to define our own NTSTATUS data type and other
// irritations throughout our code-base.
#ifdef WIN32_LEAN_AND_MEAN
#undef WIN32_LEAN_AND_MEAN
#endif

// Prevents <Windows.h> from #including <Winsock.h>, as we use <Winsock2.h> instead.
#ifndef _WINSOCKAPI_
#define DID_DEFINE_WINSOCKAPI
#define _WINSOCKAPI_
#endif

// Prevents <Windows.h> from defining min() and max() macro symbols.
#ifndef NOMINMAX
#define NOMINMAX
#endif

OVR_DISABLE_ALL_MSVC_WARNINGS()
// d3dkmthk.h requires an NTSTATUS type, but WIN32_LEAN_AND_MEAN will prevent.
#define WIN32_NO_STATUS
#include <Windows.h>
#undef WIN32_NO_STATUS
//#include <ntstatus.h>
OVR_RESTORE_ALL_MSVC_WARNINGS()

#ifdef DID_DEFINE_WINSOCKAPI
#undef _WINSOCKAPI_
#undef DID_DEFINE_WINSOCKAPI
#endif

#endif // OVR_OS_MS

#endif // OVR_Win32_IncludeWindows_h
