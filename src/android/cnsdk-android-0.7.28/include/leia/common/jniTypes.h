/*
 * Copyright 2023 (c) Leia Inc.  All rights reserved.
 *
 * NOTICE:  All information contained herein is, and remains
 * the property of Leia Inc. and its suppliers, if any.  The
 * intellectual and technical concepts contained herein are
 * proprietary to Leia Inc. and its suppliers and may be covered
 * by U.S. and Foreign Patents, patents in process, and are
 * protected by trade secret or copyright law.  Dissemination of
 * this information or reproduction of this materials strictly
 * forbidden unless prior written permission is obtained from
 * Leia Inc.
 */
#pragma once

#include "leia/common/defines.h"

#include <stdint.h>

#if defined(LEIA_OS_ANDROID)
#    include <jni.h>
#else

#    if defined(__cplusplus)
class _jobject {};
typedef _jobject* jobject;
#    else // !__cplusplus
typedef void*                            jobject;
#    endif // __cplusplus

typedef int32_t jint;

struct _JNIEnv;
struct _JavaVM;
typedef const struct JNINativeInterface* C_JNIEnv;

#    if defined(__cplusplus)
typedef _JNIEnv                          JNIEnv;
typedef _JavaVM                          JavaVM;
#    else // !__cplusplus
typedef const struct JNINativeInterface* JNIEnv;
typedef const struct JNIInvokeInterface* JavaVM;
#    endif // __cplusplus

#endif // LEIA_OS_ANDROID
