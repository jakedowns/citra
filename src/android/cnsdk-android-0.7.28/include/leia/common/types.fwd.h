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
#ifndef CNSDK_LEIA_COMMON_TYPES_FWD_H
#define CNSDK_LEIA_COMMON_TYPES_FWD_H

#include "leia/common/defines.h"

BEGIN_CAPI_DECL

/// Defines an interface for an external user to notify CNSDK of camera events.
struct leia_shared_camera_sink;

struct leia_event_listener;
struct leia_event_center;
struct leia_platform_configuration;
struct leia_const_float_slice;
struct leia_float_slice;
struct leia_slice;
struct leia_asset_manager;

END_CAPI_DECL

#endif // CNSDK_LEIA_COMMON_TYPES_FWD_H
