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
#ifndef CNSDK_LEIA_COMMON_EVENT_CENTER_H
#define CNSDK_LEIA_COMMON_EVENT_CENTER_H

#include "leia/common/types.fwd.h"
#include "leia/common/log.h"
#include "leia/common/api.h"
#include "leia/common/slice.h"

#include <stdint.h>

BEGIN_CAPI_DECL

#pragma pack(push, 8)

enum leia_event_type {
    LEIA_EVENT_TYPE_LOG              = 0,
    LEIA_EVENT_TYPE_ERROR            = 1,
    LEIA_EVENT_TYPE_COMPONENT        = 2,
    _LEIA_EVENT_TYPE_MAKE_ENUM_32BIT = 0x7FFFFFFF
};

struct leia_event_log {
    const char*    message;
    leia_log_level level;
};

struct leia_event_error {
    const char* message;
    int64_t     code;
};

enum leia_component_id {
    LEIA_COMPONENT_ID_UNDETERMINED     = 0,
    LEIA_COMPONENT_ID_CORE             = 1,
    LEIA_COMPONENT_ID_FACE_TRACKING    = 2,
    _LEIA_COMPONENT_ID_MAKE_ENUM_32BIT = 0x7FFFFFFF
};

struct leia_event_component {
    enum leia_component_id id; ///< The component id
    int32_t                code; ///< The component-specific code
    void const*            payload; ///< The component-specific payload, if any
};

enum leia_error_type {
    LEIA_ERROR_INVALID_USE           = -1,
    LEIA_ERROR_INTERNAL_ERROR        = -2,
    LEIA_ERROR_OUT_OF_SYSTEM_MEMORY  = -3,
    LEIA_ERROR_ASSERTION             = -4,
    _LEIA_ERROR_TYPE_MAKE_ENUM_32BIT = 0x7FFFFFFF
};

struct leia_event {
    enum leia_event_type type;
    union {
        struct leia_event_log       log;
        struct leia_event_error     error;
        struct leia_event_component component;
    } v;
};

typedef void (*leia_event_listener_callback)(void* userData, struct leia_event const*);

LEIA_NODISCARD
LEIA_COMMON_API
struct leia_event_listener* leia_event_listener_alloc();
/// callback must be thread-safe.
LEIA_COMMON_API
void leia_event_listener_set_callback(struct leia_event_listener*, leia_event_listener_callback callback, void* userData);
LEIA_COMMON_API
void leia_event_listener_free(struct leia_event_listener*);

/// The listener should stay valid until it's not removed.
LEIA_COMMON_API
void leia_event_center_add_listener(NONOWNING(struct leia_event_listener*) listener);
LEIA_COMMON_API
void leia_event_center_remove_listener(NONOWNING(struct leia_event_listener*) listener);
LEIA_COMMON_API
void leia_default_event_handler(struct leia_event const*);

#pragma pack(pop)

END_CAPI_DECL

#endif // CNSDK_LEIA_COMMON_EVENT_CENTER_H
