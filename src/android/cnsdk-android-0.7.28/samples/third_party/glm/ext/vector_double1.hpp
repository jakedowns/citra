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

/// @ref ext_vector_double1
/// @file glm/ext/vector_double1.hpp
///
/// @defgroup ext_vector_double1 GLM_EXT_vector_double1
/// @ingroup ext
///
/// Exposes double-precision floating point vector type with one component.
///
/// Include <glm/ext/vector_double1.hpp> to use the features of this extension.
///
/// @see ext_vector_double1_precision extension.
/// @see ext_vector_float1 extension.

#pragma once

#include "../detail/type_vec1.hpp"

#if GLM_MESSAGES == GLM_ENABLE && !defined(GLM_EXT_INCLUDED)
#	pragma message("GLM: GLM_EXT_vector_double1 extension included")
#endif

namespace glm
{
	/// @addtogroup ext_vector_double1
	/// @{

	/// 1 components vector of double-precision floating-point numbers.
	typedef vec<1, double, defaultp>		dvec1;

	/// @}
}//namespace glm
