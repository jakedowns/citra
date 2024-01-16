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
#ifndef CNSDK_LEIA_SDK_CORE_TYPES_HPP
#define CNSDK_LEIA_SDK_CORE_TYPES_HPP

#include "leia/sdk/core.types.h"

namespace leia {

using FaceTrackingRuntimeType = leia_face_tracking_runtime_type;

using FitMode            = leia_fit_mode;
using GraphicsApi        = leia_graphics_api;
using InterlaceMode      = leia_interlace_mode;
using ShaderDebugMode    = leia_shader_debug_mode;
using CalibrationPattern = leia_calibration_pattern;

using InterlacerGuiInputState          = leia_interlacer_gui_input_state;
using InterlacerGuiSurface             = leia_interlacer_gui_surface;
using InterlacerGuiConfiguration       = leia_interlacer_gui_configuration;
using InterlacerDebugMenuConfiguration = leia_interlacer_debug_menu_configuration;

LEIA_NODISCARD
LEIA_FORCE_INLINE const char* ToUiStr(FaceTrackingRuntimeType faceTrackingRuntimeType)
{
    return leia_face_tracking_runtime_type_to_ui_str(faceTrackingRuntimeType);
}
LEIA_NODISCARD
LEIA_FORCE_INLINE const char* ToUiStr(InterlaceMode interlaceMode)
{
    return leia_interlace_mode_to_ui_str(interlaceMode);
}
LEIA_NODISCARD
LEIA_FORCE_INLINE const char* ToUiStr(ShaderDebugMode shaderDebugMode)
{
    return leia_shader_debug_mode_to_ui_str(shaderDebugMode);
}

} // namespace leia

#endif // CNSDK_LEIA_SDK_CORE_TYPES_HPP
