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

/// @ref ext_matrix_uint2x2_sized
/// @file glm/ext/matrix_uint2x2_sized.hpp
///
/// @see core (dependence)
///
/// @defgroup ext_matrix_uint2x2_sized GLM_EXT_matrix_uint2x2_sized
/// @ingroup ext
///
/// Include <glm/ext/matrix_uint2x2_sized.hpp> to use the features of this extension.
///
/// Defines a number of matrices with integer types.

#pragma once

// Dependency:
#include "../mat2x2.hpp"
#include "../ext/scalar_uint_sized.hpp"

#if GLM_MESSAGES == GLM_ENABLE && !defined(GLM_EXT_INCLUDED)
#	pragma message("GLM: GLM_EXT_matrix_uint2x2_sized extension included")
#endif

namespace glm
{
	/// @addtogroup ext_matrix_uint2x2_sized
	/// @{

	/// 8 bit unsigned integer 2x2 matrix.
	///
	/// @see ext_matrix_uint2x2_sized
	typedef mat<2, 2, uint8, defaultp>				u8mat2x2;

	/// 16 bit unsigned integer 2x2 matrix.
	///
	/// @see ext_matrix_uint2x2_sized
	typedef mat<2, 2, uint16, defaultp>				u16mat2x2;

	/// 32 bit unsigned integer 2x2 matrix.
	///
	/// @see ext_matrix_uint2x2_sized
	typedef mat<2, 2, uint32, defaultp>				u32mat2x2;

	/// 64 bit unsigned integer 2x2 matrix.
	///
	/// @see ext_matrix_uint2x2_sized
	typedef mat<2, 2, uint64, defaultp>				u64mat2x2;


	/// 8 bit unsigned integer 2x2 matrix.
	///
	/// @see ext_matrix_uint2x2_sized
	typedef mat<2, 2, uint8, defaultp>				u8mat2;

	/// 16 bit unsigned integer 2x2 matrix.
	///
	/// @see ext_matrix_uint2x2_sized
	typedef mat<2, 2, uint16, defaultp>				u16mat2;

	/// 32 bit unsigned integer 2x2 matrix.
	///
	/// @see ext_matrix_uint2x2_sized
	typedef mat<2, 2, uint32, defaultp>				u32mat2;

	/// 64 bit unsigned integer 2x2 matrix.
	///
	/// @see ext_matrix_uint2x2_sized
	typedef mat<2, 2, uint64, defaultp>				u64mat2;

	/// @}
}//namespace glm
