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
#ifndef CNSDK_LEIA_HEADTRACKING_COMMON_ANDROID_FRAME_JNI_H
#define CNSDK_LEIA_HEADTRACKING_COMMON_ANDROID_FRAME_JNI_H

#include "leia/common/jniTypes.h"
#include "leia/headTracking/common/api.h"
#include "leia/headTracking/common/types.h"

BEGIN_CAPI_DECL

LHT_COMMON_API
jobject leia_headtracking_frame_to_java(JNIEnv* jniEnv, OWNING(leia_headtracking_frame*));
LHT_COMMON_API
NONOWNING(leia_headtracking_frame*) leia_headtracking_frame_from_java(JNIEnv*, jobject);

END_CAPI_DECL

#endif // CNSDK_LEIA_HEADTRACKING_COMMON_ANDROID_FRAME_JNI_H
