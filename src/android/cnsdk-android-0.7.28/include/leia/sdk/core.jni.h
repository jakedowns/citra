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

#include "leia/sdk/api.h"
#include "leia/sdk/core.fwd.h"
#include "leia/common/jniTypes.h"

BEGIN_CAPI_DECL

LEIA_NODISCARD
LEIASDK_API
leia_core* leia_core_from_java(JNIEnv* jniEnv, jobject leiaSDK);

END_CAPI_DECL
