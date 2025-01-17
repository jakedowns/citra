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

/// @ref gtx_matrix_query
/// @file glm/gtx/matrix_query.hpp
///
/// @see core (dependence)
/// @see gtx_vector_query (dependence)
///
/// @defgroup gtx_matrix_query GLM_GTX_matrix_query
/// @ingroup gtx
///
/// Include <glm/gtx/matrix_query.hpp> to use the features of this extension.
///
/// Query to evaluate matrix properties

#pragma once

// Dependency:
#include "../glm.hpp"
#include "../gtx/vector_query.hpp"
#include <limits>

#if GLM_MESSAGES == GLM_ENABLE && !defined(GLM_EXT_INCLUDED)
#	ifndef GLM_ENABLE_EXPERIMENTAL
#		pragma message("GLM: GLM_GTX_matrix_query is an experimental extension and may change in the future. Use #define GLM_ENABLE_EXPERIMENTAL before including it, if you really want to use it.")
#	else
#		pragma message("GLM: GLM_GTX_matrix_query extension included")
#	endif
#endif

namespace glm
{
	/// @addtogroup gtx_matrix_query
	/// @{

	/// Return whether a matrix a null matrix.
	/// From GLM_GTX_matrix_query extension.
	template<typename T, qualifier Q>
	GLM_FUNC_DECL bool isNull(mat<2, 2, T, Q> const& m, T const& epsilon);

	/// Return whether a matrix a null matrix.
	/// From GLM_GTX_matrix_query extension.
	template<typename T, qualifier Q>
	GLM_FUNC_DECL bool isNull(mat<3, 3, T, Q> const& m, T const& epsilon);

	/// Return whether a matrix is a null matrix.
	/// From GLM_GTX_matrix_query extension.
	template<typename T, qualifier Q>
	GLM_FUNC_DECL bool isNull(mat<4, 4, T, Q> const& m, T const& epsilon);

	/// Return whether a matrix is an identity matrix.
	/// From GLM_GTX_matrix_query extension.
	template<length_t C, length_t R, typename T, qualifier Q, template<length_t, length_t, typename, qualifier> class matType>
	GLM_FUNC_DECL bool isIdentity(matType<C, R, T, Q> const& m, T const& epsilon);

	/// Return whether a matrix is a normalized matrix.
	/// From GLM_GTX_matrix_query extension.
	template<typename T, qualifier Q>
	GLM_FUNC_DECL bool isNormalized(mat<2, 2, T, Q> const& m, T const& epsilon);

	/// Return whether a matrix is a normalized matrix.
	/// From GLM_GTX_matrix_query extension.
	template<typename T, qualifier Q>
	GLM_FUNC_DECL bool isNormalized(mat<3, 3, T, Q> const& m, T const& epsilon);

	/// Return whether a matrix is a normalized matrix.
	/// From GLM_GTX_matrix_query extension.
	template<typename T, qualifier Q>
	GLM_FUNC_DECL bool isNormalized(mat<4, 4, T, Q> const& m, T const& epsilon);

	/// Return whether a matrix is an orthonormalized matrix.
	/// From GLM_GTX_matrix_query extension.
	template<length_t C, length_t R, typename T, qualifier Q, template<length_t, length_t, typename, qualifier> class matType>
	GLM_FUNC_DECL bool isOrthogonal(matType<C, R, T, Q> const& m, T const& epsilon);

	/// @}
}//namespace glm

#include "matrix_query.inl"
