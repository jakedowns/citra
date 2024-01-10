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
#include "leia/common/config.h"

#ifdef LEIA_COMMON_EXPORTS
#    define LEIA_COMMON_API LEIA_EXPORT
#    define LEIA_COMMON_CLASS_API LEIA_CLASS_EXPORT
#else
#    ifdef LEIA_COMMON_STATIC
#        define LEIA_COMMON_API
#        define LEIA_COMMON_CLASS_API
#    else
#        define LEIA_COMMON_API LEIA_IMPORT
#        define LEIA_COMMON_CLASS_API LEIA_CLASS_IMPORT
#    endif
#endif
