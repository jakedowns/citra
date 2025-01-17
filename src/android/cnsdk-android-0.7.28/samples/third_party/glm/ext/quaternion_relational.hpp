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

/// @ref ext_quaternion_relational
/// @file glm/ext/quaternion_relational.hpp
///
/// @defgroup ext_quaternion_relational GLM_EXT_quaternion_relational
/// @ingroup ext
///
/// Exposes comparison functions for quaternion types that take a user defined epsilon values.
///
/// Include <glm/ext/quaternion_relational.hpp> to use the features of this extension.
///
/// @see core_vector_relational
/// @see ext_vector_relational
/// @see ext_matrix_relational
/// @see ext_quaternion_float
/// @see ext_quaternion_double

#pragma once

// Dependency:
#include "../vector_relational.hpp"

#if GLM_MESSAGES == GLM_ENABLE && !defined(GLM_EXT_INCLUDED)
#	pragma message("GLM: GLM_EXT_quaternion_relational extension included")
#endif

namespace glm
{
	/// @addtogroup ext_quaternion_relational
	/// @{

	/// Returns the component-wise comparison of result x == y.
	///
	/// @tparam T Floating-point scalar types
	/// @tparam Q Value from qualifier enum
	template<typename T, qualifier Q>
	GLM_FUNC_DECL vec<4, bool, Q> equal(qua<T, Q> const& x, qua<T, Q> const& y);

	/// Returns the component-wise comparison of |x - y| < epsilon.
	///
	/// @tparam T Floating-point scalar types
	/// @tparam Q Value from qualifier enum
	template<typename T, qualifier Q>
	GLM_FUNC_DECL vec<4, bool, Q> equal(qua<T, Q> const& x, qua<T, Q> const& y, T epsilon);

	/// Returns the component-wise comparison of result x != y.
	///
	/// @tparam T Floating-point scalar types
	/// @tparam Q Value from qualifier enum
	template<typename T, qualifier Q>
	GLM_FUNC_DECL vec<4, bool, Q> notEqual(qua<T, Q> const& x, qua<T, Q> const& y);

	/// Returns the component-wise comparison of |x - y| >= epsilon.
	///
	/// @tparam T Floating-point scalar types
	/// @tparam Q Value from qualifier enum
	template<typename T, qualifier Q>
	GLM_FUNC_DECL vec<4, bool, Q> notEqual(qua<T, Q> const& x, qua<T, Q> const& y, T epsilon);

	/// @}
} //namespace glm

#include "quaternion_relational.inl"
