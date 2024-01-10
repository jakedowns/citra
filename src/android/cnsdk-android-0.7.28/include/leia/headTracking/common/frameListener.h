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

#include "leia/common/types.h"
#include "leia/headTracking/common/api.h"

BEGIN_CAPI_DECL

struct leia_headtracking_frame;
struct leia_headtracking_frame_listener;

#ifdef LEIA_DOXYGEN
/**
 * @brief Face tracking frame listener.
 */
struct leia_headtracking_frame_listener {};
#endif // LEIA_DOXYGEN

/**
 * @brief Face Tracking frame callback.
 *
 * This callback is invoked from a background thread.
 * And it might still be called after disabling the listener due to asynchronicity.
 * Calling it must be valid until leia_free_user_data_cb has been called.
 *
 * Note: the handler is responsible for releasing the frame (@see leia_headtracking_frame_release)
 */
typedef void (*leia_headtracking_on_frame_cb)(OWNING(struct leia_headtracking_frame*) frame, void* userData);

/**
 * @brief Allocate face tracking frame listener.
 *
 * @param onFrameCb      frame callback
 * @param userData       user-provided pointer passed into callbacks [optional]
 * @param freeUserDataCb callback used to free the userData [optional]
 *
 * @relates leia_headtracking_frame_listener
 */
LEIA_NODISCARD
LHT_COMMON_API
OWNING(struct leia_headtracking_frame_listener*)
leia_headtracking_frame_listener_alloc(leia_headtracking_on_frame_cb onFrameCb, void* userData, leia_free_user_data_cb freeUserDataCb);

/**
 * @brief Free face tracking frame listener.
 *
 * @public @memberof leia_headtracking_frame_listener
 */
LHT_COMMON_API
void leia_headtracking_frame_listener_release(OWNING(struct leia_headtracking_frame_listener*) listener);

END_CAPI_DECL
