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
#pragma once

#include <glm/glm.hpp>

namespace leia {
namespace lwe {

/// <summary>
/// Camera using a right-handed coordinate system and Z-Up axis.
/// </summary>
class LWE_Camera {
public:
    glm::vec3 pos;
    glm::vec3 dir;

    LWE_Camera()
    {
        pos = glm::vec3(0.0f, 0.0f, 0.0f);
        dir = glm::vec3(0.0f, 1.0f, 0.0f);
    }

    glm::mat3x3 getBasis() const
    {
        glm::vec3 fwd   = dir;
        glm::vec3 right = glm::cross(fwd, glm::vec3(0, 0, 1)); // note: z-up. also deal with failure when camera pointed straight up.
        glm::vec3 up    = glm::cross(dir, right);

        glm::mat3x3 basis;
        basis[0] = right;
        basis[1] = up;
        basis[2] = fwd;

        return basis;
    }

    glm::vec3 getForward() const
    {
        return dir;
    }

    glm::vec3 getRight() const
    {
        glm::vec3 right = glm::cross(dir, glm::vec3(0, 0, 1)); // note: z-up also deal with failure when camera pointed straight up.
        return right;
    }

    glm::vec3 getUp() const
    {
        glm::vec3 up = glm::cross(getRight(), dir);
        return up;
    }
};

} // namespace lwe
} // namespace leia
