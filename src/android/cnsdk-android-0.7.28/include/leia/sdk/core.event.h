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
#ifndef CNSDK_LEIA_SDK_CORE_EVENT_H
#define CNSDK_LEIA_SDK_CORE_EVENT_H

#include "leia/common/defines.h"

BEGIN_CAPI_DECL

enum leia_core_event_code {
    LEIA_CORE_EVENT_DID_INITIALIZE        = 0,
    _LEIA_CORE_EVENT_CODE_MAKE_ENUM_32BIT = 0x7FFFFFFF
};

END_CAPI_DECL

#endif // CNSDK_LEIA_SDK_CORE_EVENT_H
