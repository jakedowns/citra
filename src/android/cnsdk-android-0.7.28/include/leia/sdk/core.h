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
#ifndef CNSDK_LEIA_SDK_CORE_H
#define CNSDK_LEIA_SDK_CORE_H

#include "leia/sdk/api.h"
#include "leia/sdk/core.types.h"
#include "leia/common/log.h"
#include "leia/common/jniTypes.h"
#include "leia/common/types.h"
#include "leia/common/eventCenter.h"
#include "leia/device/config.h"
#include "leia/headTracking/common/frame.h"
#include "leia/headTracking/common/frameListener.h"

#include <stdint.h>

BEGIN_CAPI_DECL

/*!
 * @defgroup core Core
 *
 * @brief CNSDK's core.
 *
 *
 */

/*!
 * @addtogroup core
 * @{
 */

#ifdef LEIA_DOXYGEN

/**
 * @brief The main entry point into CNSDK C API.
 *
 * It's designed to be initialized once at the start of an app using leia_core_init_async(leia_core_init_configuration*).
 *
 * \code{.cpp}
 *     int main() {
 *         struct leia_core_init_configuration* config = leia_core_init_configuration_alloc(CNSDK_VERSION);
 *         leia_core* core = leia_core_init_async(config);
 *         leia_core_init_configuration_free(config);
 *
 *         while (!leia_core_is_initialized(core));
 *
 *         auto interlacerConfig = leia_interlacer_init_configuration_alloc();
 *         leia_interlacer* interlacer = leia_interlacer_opengl_initialize(core, interlacerConfig, glContext);
 *         while (rendering) {
 *             GLuint tilesTexture = getTilesTexture();
 *             attachFrameBuffer();
 *             leia_interlacer_opengl_do_post_process_picture(interlacer, framebufferWidth, framebufferHeight, tilesTexture);
 *             present();
 *         }
 *
 *         leia_core_shutdown(core);
 *     }
 * \endcode
 */
struct leia_core {};

/**
 * @brief leia_core's initial configuration.
 *
 * Used to describe the host environment and configure the initial state of the modules such as face tracking.
 */
struct leia_core_init_configuration {};

#endif

/**
 * @brief Start initialization of a leia_core instance.
 *
 * On some platforms (e.g. Android), initialization process is asynchronous.
 * A part of CNSDK functionality requires leia_core to be fully initialized before use.
 * There are two ways to know whether leia_core is fully initialized:
 *   - poll leia_core_is_initialized(leia_core)
 *   - get notified via the event center (See LEIA_CORE_EVENT_DID_INITIALIZE, leia_event_center_add_listener)
 *
 * Should be called once.
 *
 * @relates leia_core
 */
LEIA_NODISCARD
LEIASDK_API
struct leia_core* leia_core_init_async(struct leia_core_init_configuration*);

/**
 * @param cnsdkVersion Current CNSDK version - CNSDK_VERSION
 * @relates leia_core_init_configuration
 */
LEIA_NODISCARD
LEIASDK_API
struct leia_core_init_configuration* leia_core_init_configuration_alloc(const char* cnsdkVersion);

/// Whether to enable runtime validation.
/// @public @memberof leia_core_init_configuration
LEIASDK_API
void leia_core_init_configuration_set_enable_validation(struct leia_core_init_configuration*, leia_bool enable);

enum leia_core_android_handle_type {
    LEIA_CORE_ANDROID_HANDLE_APP      = 0, ///< android.app.Application.
    LEIA_CORE_ANDROID_HANDLE_ACTIVITY = 1, ///< android.app.Activity.
    LEIA_CORE_ANDROID_HANDLE_CONTEXT  = 2, ///< android.content.Context.

    _LEIA_CORE_ANDROID_HANDLE_MAKE_ENUM_32BIT = 0x7FFFFFFF
};

/// Set one of the android handles. In this priority order: App > Activity > Context.
/// Unity and Unreal applications can leave this unset.
/// @public @memberof leia_core_init_configuration
LEIASDK_API
void leia_core_init_configuration_set_platform_android_handle(struct leia_core_init_configuration*, enum leia_core_android_handle_type, jobject);

/// Set Java VM handle. Must be provided in the purely native android applications when JNI_OnLoad is not called.
/// @public @memberof leia_core_init_configuration
LEIASDK_API
void leia_core_init_configuration_set_platform_android_java_vm(struct leia_core_init_configuration*, JavaVM*);

/// Use any kLeiaLogLevel except the default one to override the logging level.
/// @public @memberof leia_core_init_configuration
LEIASDK_API
void leia_core_init_configuration_set_platform_log_level(struct leia_core_init_configuration*, leia_log_level);

/// Set face tracking shared camera sink.
/// @public @memberof leia_core_init_configuration
LEIASDK_API
void leia_core_init_configuration_set_face_tracking_shared_camera_sink(struct leia_core_init_configuration*, struct leia_shared_camera_sink*);

/// Set face tracking runtime to use.
/// @public @memberof leia_core_init_configuration
LEIASDK_API
void leia_core_init_configuration_set_face_tracking_runtime(struct leia_core_init_configuration*, enum leia_face_tracking_runtime_type);

/// Enable face tracking ASAP.
/// @public @memberof leia_core_init_configuration
LEIASDK_API
void leia_core_init_configuration_set_face_tracking_enable(struct leia_core_init_configuration*, leia_bool);

/// Start face tracking ASAP.
/// @public @memberof leia_core_init_configuration
LEIASDK_API
void leia_core_init_configuration_set_face_tracking_start(struct leia_core_init_configuration*, leia_bool);

/// Do the system-level face tracking permission check.
/// @public @memberof leia_core_init_configuration
LEIASDK_API
void leia_core_init_configuration_set_face_tracking_check_permission(struct leia_core_init_configuration*, leia_bool);

/// Whether CNSDK should kill the process when the user denies face tracking permission.
/// @public @memberof leia_core_init_configuration
LEIASDK_API
void leia_core_init_configuration_set_face_tracking_permission_dialog_kill_process(struct leia_core_init_configuration*, leia_bool);

/// Face tracking service logging level.
/// @public @memberof leia_core_init_configuration
LEIASDK_API
void leia_core_init_configuration_set_face_tracking_server_log_level(struct leia_core_init_configuration*, leia_log_level);

/// Use to fallback to the old renderer.
#define LEIA_CORE_HINT_USE_OLD_RENDERER "useOldRenderer" // Will be removed when done testing the new renderer

/// @public @memberof leia_core_init_configuration
LEIASDK_API
void leia_core_init_configuration_set_hint(struct leia_core_init_configuration*, const char* hint);

/// @public @memberof leia_core_init_configuration
LEIASDK_API
void leia_core_init_configuration_free(struct leia_core_init_configuration*);

/**
 * @brief Check if leia_core is initialized.
 *
 * @see leia_core_init_async(leia_core_init_configuration*)
 *
 * @public @memberof leia_core
 */
LEIA_NODISCARD
LEIASDK_API
leia_bool leia_core_is_initialized(struct leia_core*);

/** @brief Frees resources allocated after leia_core_init_async.
 *
 * @public @memberof leia_core
 */
LEIASDK_API
void leia_core_shutdown(struct leia_core*);

/**
 * @public @memberof leia_core
 */
LEIA_NODISCARD
LEIASDK_API
leia_bool leia_core_is_validation_enabled(struct leia_core*);

/**
 * @brief Get the device config for read/write access.
 *
 * Requires leia_core to be fully initialized (leia_core_is_initialized(leia_core) == true).
 * Should be released via leia_core_release_device_config.
 *
 * @public @memberof leia_core
 */
LEIA_NODISCARD
LEIASDK_API
struct leia_device_config* leia_core_get_device_config(struct leia_core*);

/**
 * @brief Synchronize the internal device config with the provided one.
 *
 * @public @memberof leia_core
 */
LEIASDK_API
void leia_core_sync_device_config(struct leia_core*, struct leia_device_config*);

/**
 * @brief Release the device config allocated with leia_core_get_device_config.
 *
 * @public @memberof leia_core
 */
LEIASDK_API
void leia_core_release_device_config(struct leia_core*, struct leia_device_config*);

/**
 * @brief Get the center view number.
 *
 * Requires leia_core to be fully initialized (leia_core_is_initialized(leia_core) == true).
 *
 * @public @memberof leia_core
 */
LEIASDK_API
float leia_core_get_center_view(struct leia_core*);

/**
 * @brief Set a center view number.
 *
 * Requires leia_core to be fully initialized (leia_core_is_initialized(leia_core) == true).
 *
 * @public @memberof leia_core
 */
LEIASDK_API
void leia_core_set_center_view(struct leia_core*, float centerView);

/**
 * @brief Notify of app resume.
 *
 * @public @memberof leia_core
 */
LEIASDK_API
void leia_core_on_resume(struct leia_core*);
/**
 * @brief Notify of app pause.
 *
 * @public @memberof leia_core
 */
LEIASDK_API
void leia_core_on_pause(struct leia_core*);

/**
 * @public @memberof leia_core
 */
LEIASDK_API
void leia_core_set_backlight(struct leia_core*, leia_bool enable);
/**
 * @public @memberof leia_core
 */
LEIA_NODISCARD
LEIASDK_API
leia_bool leia_core_get_backlight(struct leia_core*);
/**
 * @public @memberof leia_core
 */
LEIA_NODISCARD
LEIASDK_API
leia_bool leia_core_is_overlay_device(struct leia_core*);

/**
 * @brief Initialize/Release all face tracking related resources.
 *
 * Do not call on the main thread to avoid stalls.
 *
 * If face tracking should be temporarily paused,
 * consider using leia_core_start_face_tracking(leia_core, leia_bool) instead.
 *
 * @public @memberof leia_core
 */
LEIASDK_API
leia_bool leia_core_enable_face_tracking(struct leia_core*, leia_bool enable);
/**
 * @public @memberof leia_core
 */
LEIA_NODISCARD
LEIASDK_API
leia_bool leia_core_is_face_tracking_enabled(struct leia_core*);

/**
 * @brief Resume/pause face tracking.
 *
 * It's a lightweight call - can be called on the main thread.
 *
 * @public @memberof leia_core
 */
LEIASDK_API
void leia_core_start_face_tracking(struct leia_core*, leia_bool start);
/**
 * @public @memberof leia_core
 */
LEIA_NODISCARD
LEIASDK_API
leia_bool leia_core_is_face_tracking_started(struct leia_core*);

/**
 * @brief Configure face tracking inference device and the kind of input it should use.
 *
 * @public @memberof leia_core
 */
LEIASDK_API
void leia_core_set_face_detector_config(struct leia_core*, struct leia_face_detector_config);

/**
 * @public @memberof leia_core
 */
LEIA_NODISCARD
LEIASDK_API
struct leia_face_detector_config leia_core_get_face_detector_config(struct leia_core*);

/**
 * @brief Select which eye to track.
 *
 * Used mainly for calibration purposes.
 *
 * @public @memberof leia_core
 */
LEIASDK_API
void leia_core_set_face_tracker_configuration(struct leia_core*, leia_bool trackLeft, leia_bool trackRight);

/**
 * @brief Set face tracking runtime.
 *
 * No-op if runtime is not supported.
 *
 * @see leia_core_is_face_tracking_runtimeSupported.
 *
 * @public @memberof leia_core
 */
LEIASDK_API
void leia_core_set_face_tracking_runtime(struct leia_core*, enum leia_face_tracking_runtime_type);
/**
 * @public @memberof leia_core
 */
LEIA_NODISCARD
LEIASDK_API
enum leia_face_tracking_runtime_type leia_core_get_face_tracking_runtime(struct leia_core*);

/**
 * @brief Configure face tracking to use the shared camera sink.
 *
 * Face tracking must be re-enabled to apply changes.
 * Shared camera sink is supported only by in-app face tracking runtime.
 * leia_core takes ownership over the sink.
 *
 * @public @memberof leia_core
 */
LEIASDK_API
void leia_core_set_face_tracking_shared_camera_sink(struct leia_core*, OWNING(struct leia_shared_camera_sink*) sink);

/**
 * @brief Get the primary face.
 *
 * @param position a predicted face position
 *
 * @returns whether there is a face or none
 *
 * @public @memberof leia_core
 */
LEIA_NODISCARD
LEIASDK_API
leia_bool leia_core_get_primary_face(struct leia_core*, LEIA_FLOAT_SLICE(3) position);

/**
 * @brief Get the primary face.
 *
 * @param position a non-predicted face position
 *
 * @returns whether there is a face or none
 *
 * @public @memberof leia_core
 */
LEIA_NODISCARD
LEIASDK_API
leia_bool leia_core_get_non_predicted_primary_face(struct leia_core*, LEIA_FLOAT_SLICE(3) position);

/**
 * @public @memberof leia_core
 */
LEIASDK_API
void leia_core_set_face_tracking_frame_listener(struct leia_core*, OWNING(struct leia_headtracking_frame_listener*));

/** @brief Toggle face tracking profiling.
 *
 * @public @memberof leia_core
 */
LEIASDK_API
void leia_core_set_face_tracking_profiling(struct leia_core*, leia_bool enable);

/**
 * @brief Get face tracking profiling data.
 *
 * Face tracking profiling must be explicitly enabled.
 *
 * @see Set_face_tracking_profiling.
 *
 * @public @memberof leia_core
 */
LEIA_NODISCARD
LEIASDK_API
leia_bool leia_core_get_face_tracking_profiling(struct leia_core*, struct leia_headtracking_frame_profiling* profling, int32_t profilingSizeof);

/**
 * @brief Toggle no-face mode.
 *
 * No-face mode automatically disables backlight and forces all interlaces
 * into the single view mode when there is no face available for a particular amount of time.
 *
 * @public @memberof leia_core
 */
LEIASDK_API
void leia_core_enable_no_face_mode(struct leia_core*, leia_bool enable);

/**
 * @public @memberof leia_core
 */
LEIA_NODISCARD
LEIASDK_API
struct leia_asset_manager* leia_core_get_asset_manager(struct leia_core*);

/** @brief Check if the current CNSDK build support particular leia_face_tracking_runtime_type.
 *
 * @public @memberof leia_core
 */
LEIA_NODISCARD
LEIASDK_API
leia_bool leia_is_face_tracking_runtime_supported(enum leia_face_tracking_runtime_type);

/*!
 * @}
 */

END_CAPI_DECL

#endif // CNSDK_LEIA_SDK_CORE_H

#if !defined(_LEIA_DUMP_ABI_VERIFICATION) && defined(LEIA_X64)
#include "leia/sdk/core.event.h"
#include "leia/sdk/core.interlacer.h"
#include "leia/common/platform.h"
#include "leia/common/sharedCameraSink.h"
#include <stddef.h>
#ifdef __cplusplus
#    define _LEIA_LINE_STRING LEIA_STRINGIZE(__LINE__)
#    define LEIA_ABI_ASSERT(condition) static_assert(condition, "ABI assert failed on line " _LEIA_LINE_STRING ": " #condition)
#    include <type_traits>
#    define LEIA_ABI_TRIVIALLY_COPYABLE_ASSERT(T) static_assert(std::is_trivially_copyable<T>::value, #T " must be trivially copyable")
#else // !__cplusplus
#    define LEIA_ABI_ASSERT(condition) typedef char static_abi_assertion_failed_at_line##__LINE__[(condition) ? 1 : -1]
#    define LEIA_ABI_TRIVIALLY_COPYABLE_ASSERT(T)
#endif // __cplusplus
#define LEIA_ABI_SIZEOF_ASSERT(T, size) LEIA_ABI_ASSERT(sizeof(T) == size)
#define LEIA_ABI_OFFSETOF_ASSERT(T, F, offset) LEIA_ABI_ASSERT(offsetof(T, F) == offset)
#include "leia/sdk/core.abiVerification.h"
#undef LEIA_ABI_ASSERT
#undef _LEIA_LINE_STRING
#endif // !_LEIA_DUMP_ABI_VERIFICATION
