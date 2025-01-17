/*
 * Copyright (c) 2023 Leia Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/// @ref gtc_matrix_transform
/// @file glm/gtc/matrix_transform.hpp
///
/// @see core (dependence)
/// @see gtx_transform
/// @see gtx_transform2
///
/// @defgroup gtc_matrix_transform GLM_GTC_matrix_transform
/// @ingroup gtc
///
/// Include <glm/gtc/matrix_transform.hpp> to use the features of this extension.
///
/// Defines functions that generate common transformation matrices.
///
/// The matrices generated by this extension use standard OpenGL fixed-function
/// conventions. For example, the lookAt function generates a transform from world
/// space into the specific eye space that the projective matrix functions
/// (perspective, ortho, etc) are designed to expect. The OpenGL compatibility
/// specifications defines the particular layout of this eye space.

#pragma once

// Dependencies
#include "../mat4x4.hpp"
#include "../vec2.hpp"
#include "../vec3.hpp"
#include "../vec4.hpp"
#include "../ext/matrix_projection.hpp"
#include "../ext/matrix_clip_space.hpp"
#include "../ext/matrix_transform.hpp"

#if GLM_MESSAGES == GLM_ENABLE && !defined(GLM_EXT_INCLUDED)
#	pragma message("GLM: GLM_GTC_matrix_transform extension included")
#endif

#include "matrix_transform.inl"
