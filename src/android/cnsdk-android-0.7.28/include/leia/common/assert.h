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

#define LNK_ASSERT(e) ((e) ? (void)0 : leia_assert(__FILE__, __LINE__, LEIA_FUNCTION, #e))

BEGIN_CAPI_DECL

#ifdef LNK_ENABLE_ASSERT
[[noreturn]]
#endif // LNK_ENABLE_ASSERT
LEIA_COMMON_API
void leia_assert(const char* file, int line, const char* function, const char* msg);

END_CAPI_DECL
