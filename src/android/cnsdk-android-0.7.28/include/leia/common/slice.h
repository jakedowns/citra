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
#ifndef CNSDK_LEIA_COMMON_SLICE_H
#define CNSDK_LEIA_COMMON_SLICE_H

#include "leia/common/types.fwd.h"

#include <stdint.h>

BEGIN_CAPI_DECL

#pragma pack(push, 8)

struct leia_float_slice {
    float*  data;
    int32_t length;
};

struct leia_const_float_slice {
    float const* data;
    int32_t      length;
};

struct leia_slice {
    void*   data;
    int32_t length;
};

#define LEIA_FLOAT_SLICE(size) struct leia_float_slice
#define LEIA_CONST_FLOAT_SLICE(size) struct leia_const_float_slice

#pragma pack(pop)

END_CAPI_DECL

#endif // CNSDK_LEIA_COMMON_SLICE_H
