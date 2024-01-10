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
#include "LWE_Types.h"

#include <glm/ext/matrix_transform.hpp>

namespace leia {
namespace lwe {

VertArray make_face(int v0, int v1, int v2, int v3, float r, float g, float b, VertArray const& allVerts, bool reverseWinding)
{
    VertArray newFace;
    newFace.resize(4);

    newFace[0]   = allVerts[reverseWinding ? v3 : v0];
    newFace[0].r = r;
    newFace[0].g = g;
    newFace[0].b = b;
    newFace[0].u = 0.0f;
    newFace[0].v = 1.0f;

    newFace[1]   = allVerts[reverseWinding ? v2 : v1];
    newFace[1].r = r;
    newFace[1].g = g;
    newFace[1].b = b;
    newFace[1].u = 0.0f;
    newFace[1].v = 0.0f;

    newFace[2]   = allVerts[reverseWinding ? v1 : v2];
    newFace[2].r = r;
    newFace[2].g = g;
    newFace[2].b = b;
    newFace[2].u = 1.0f;
    newFace[2].v = 0.0f;

    newFace[3]   = allVerts[reverseWinding ? v0 : v3];
    newFace[3].r = r;
    newFace[3].g = g;
    newFace[3].b = b;
    newFace[3].u = 1.0f;
    newFace[3].v = 1.0f;

    return newFace;
}

GameObject::GameObject()
{
    mat = glm::identity<glm::mat4>();
}

} // namespace lwe
} // namespace leia
