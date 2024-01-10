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

#include "leia/common/api.h"

#define CNSDK_VERSION "0.7.28"
#define CNSDK_MAJOR_VERSION 0
#define CNSDK_MINOR_VERSION 7
#define CNSDK_PATCH_VERSION 28

BEGIN_CAPI_DECL

LEIA_COMMON_API
const char* leia_get_git_refspec();

LEIA_COMMON_API
const char* leia_get_git_sha1();

LEIA_COMMON_API
const char* leia_get_version();

END_CAPI_DECL
