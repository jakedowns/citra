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

#include "leia/common/assert.h"

#include <vector>
#include <string>
#include <map>

#if !defined(__ANDROID__)
#    include <basetsd.h>
#endif

#include <glm/glm.hpp>

// Definitions
#ifndef VALIDATE
#    define VALIDATE(x, msg)                                           \
        if (!(x))                                                      \
        {                                                              \
            MessageBoxA(NULL, (msg), "LeiaSDK", MB_ICONERROR | MB_OK); \
            exit(-1);                                                  \
        }
#endif

#ifndef OVR_DEBUG_LOG
#    define OVR_DEBUG_LOG(x)
#endif

namespace leia {
namespace lwe {

struct Material {
    std::string type;
    int         r;
    int         g;
    int         b;

    Material() : type(), r(-1), g(-1), b(-1)
    {
    }
};

struct UV {
    float u = 0.f;
    float v = 0.f;
};

struct Vert {
    float x;
    float y;
    float z;

    float r;
    float g;
    float b;

    float u = 0.f;
    float v = 0.f;

    Vert(float x = 0.0f, float y = 0.0f, float z = 0.0f, float textureId = 0.0f, float u = 0.0f, float v = 0.0f)
    {
        this->x = x;
        this->y = y;
        this->z = z;

        this->r = textureId;
        this->g = 0.0f;
        this->b = 0.0f;

        this->u = u;
        this->v = v;
    }
};

typedef std::vector<Vert>         VertArray;
typedef std::vector<unsigned int> IndexArray;

struct Face {
    Material  Mat;
    VertArray Verts;
};

typedef std::vector<Face>  FaceArray;
typedef std::map<int, int> FaceTextureMap;

struct BinModel {
    std::string    filePath;
    FaceArray      faces;
    FaceTextureMap textureMap;
    unsigned int   vbo;
    unsigned int   ibo;
    unsigned int   vao;
    unsigned int   indices_count;
};

typedef std::map<int, int> TextureMap;

#if defined(__ANDROID__)
typedef signed char    INT8, *PINT8;
typedef signed short   INT16, *PINT16;
typedef signed int     INT32, *PINT32;
typedef unsigned char  UINT8, *PUINT8;
typedef unsigned short UINT16, *PUINT16;
typedef unsigned int   UINT32, *PUINT32;
#endif

struct PIXCOL {
    UINT8 red;
    UINT8 green;
    UINT8 blue;
    UINT8 flags;
};

struct GameObject {
    GameObject();
    INT32     id;
    glm::mat4 mat;
};

typedef float FLOAT32;
struct PIXBITMAP {
    UINT16  mode;
    INT32   bitmap;
    INT32   texture;
    INT16   srcx, srcy;
    INT16   srcdx, srcdy;
    INT16   dstx, dsty;
    FLOAT32 dstdx, dstdy;
    FLOAT32 dstoffx, dstoffy;
    PIXCOL  maskcolor;
    UINT16  transparency;
    PIXCOL  startcolor;
    PIXCOL  endcolor;
    UINT8   paletteNumber;
    UINT32  update;
};

VertArray make_face(int v0, int v1, int v2, int v3, float r, float g, float b, VertArray const& allVerts, bool reverseWinding = false);

} // namespace lwe
} // namespace leia
