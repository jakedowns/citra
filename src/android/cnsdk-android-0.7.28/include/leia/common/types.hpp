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
#include "leia/common/types.h"

namespace leia {

using Timestamp             = leia_timestamp;
using TimestampSpace        = leia_timestamp_space;
using ImageDesc             = leia_image_desc;
using CameraIntrinsics      = leia_camera_intrinsics;
using FaceDetectorBackend   = leia_face_detector_backend;
using FaceDetectorInputType = leia_face_detector_input_type;
using FaceDetectorConfig    = leia_face_detector_config;
using SourceLocation        = leia_source_location;

LEIA_NODISCARD
LEIA_FORCE_INLINE const char* ToStr(FaceDetectorBackend backend)
{
    return leia_face_detector_backend_to_str(backend);
}
LEIA_NODISCARD
LEIA_FORCE_INLINE const char* ToUiStr(FaceDetectorBackend backend)
{
    return leia_face_detector_backend_to_ui_str(backend);
}
LEIA_NODISCARD
LEIA_FORCE_INLINE const char* ToStr(FaceDetectorInputType inputType)
{
    return leia_face_detector_input_type_to_str(inputType);
}
LEIA_NODISCARD
LEIA_FORCE_INLINE const char* ToUiStr(FaceDetectorInputType inputType)
{
    return leia_face_detector_input_type_to_ui_str(inputType);
}

} // namespace leia
