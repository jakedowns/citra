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

#include <stdint.h>

BEGIN_CAPI_DECL

enum // leia_log_level
{
    kLeiaLogLevelDefault = 0,
    kLeiaLogLevelTrace,
    kLeiaLogLevelDebug,
    kLeiaLogLevelInfo,
    kLeiaLogLevelWarn,
    kLeiaLogLevelError,
    kLeiaLogLevelCritical,
    kLeiaLogLevelOff,
    kNumLeiaLogLevels
};

typedef int32_t leia_log_level;

LEIA_NODISCARD
LEIA_COMMON_API
const char* leia_log_level_to_string(leia_log_level);

END_CAPI_DECL
