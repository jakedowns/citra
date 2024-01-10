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

#include <math.h>

namespace leia {
namespace renderer {
namespace math {
#pragma pack(push, 1)

struct vec2f {
    union {
        struct {
            float x; ///< X component
            float y;
        }; ///< Y component
        struct {
            float e[2];
        }; ///< Indexed components
    };

    // Default constructor
    vec2f() = default;

    // Constructors
    explicit vec2f(float xy) : x(xy), y(xy)
    {
    }
    explicit vec2f(float _x, float _y) : x(_x), y(_y)
    {
    }
};

struct vec3f {
    union {
        struct {
            float x; ///< X component
            float y; ///< Y component
            float z;
        }; ///< Z component
        struct {
            float e[3];
        }; ///< Indexed components
    };

    // Default constructor
    vec3f() = default;

    // Constructors
    explicit vec3f(float xyz) : x(xyz), y(xyz), z(xyz)
    {
    }
    explicit vec3f(float _x, float _y, float _z) : x(_x), y(_y), z(_z)
    {
    }
    explicit vec3f(const vec2f& xy, float _z) : x(xy.x), y(xy.y), z(_z)
    {
    }

    // Binary operators
    friend vec3f operator+(const vec3f& lhs, float rhs)
    {
        return vec3f(lhs.x + rhs, lhs.y + rhs, lhs.z + rhs);
    }
    friend vec3f operator-(const vec3f& lhs, float rhs)
    {
        return vec3f(lhs.x - rhs, lhs.y - rhs, lhs.z - rhs);
    }
    friend vec3f operator*(const vec3f& lhs, float rhs)
    {
        return vec3f(lhs.x * rhs, lhs.y * rhs, lhs.z * rhs);
    }
    friend vec3f operator/(const vec3f& lhs, float rhs)
    {
        const float InvRHS = 1.0f / rhs;
        return vec3f(lhs.x * InvRHS, lhs.y * InvRHS, lhs.z * InvRHS);
    }
    friend vec3f operator+(float lhs, const vec3f& rhs)
    {
        return vec3f(lhs + rhs.x, lhs + rhs.y, lhs + rhs.z);
    }
    friend vec3f operator-(float lhs, const vec3f& rhs)
    {
        return vec3f(lhs - rhs.x, lhs - rhs.y, lhs - rhs.z);
    }
    friend vec3f operator*(float lhs, const vec3f& rhs)
    {
        return vec3f(lhs * rhs.x, lhs * rhs.y, lhs * rhs.z);
    }
    friend vec3f operator/(float lhs, const vec3f& rhs)
    {
        return vec3f(lhs / rhs.x, lhs / rhs.y, lhs / rhs.z);
    }
    friend vec3f operator+(const vec3f& lhs, const vec3f& rhs)
    {
        return vec3f(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z);
    }
    friend vec3f operator-(const vec3f& lhs, const vec3f& rhs)
    {
        return vec3f(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z);
    }
    friend vec3f operator*(const vec3f& lhs, const vec3f& rhs)
    {
        return vec3f(lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z);
    }
    friend vec3f operator/(const vec3f& lhs, const vec3f& rhs)
    {
        return vec3f(lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z);
    }
    friend bool operator<(const vec3f& lhs, const vec3f& rhs)
    {
        return (lhs.x < rhs.x) && (lhs.y < rhs.y) && (lhs.z < rhs.z);
    }
    friend bool operator<(const vec3f& lhs, float rhs)
    {
        return (lhs.x < rhs) && (lhs.y < rhs) && (lhs.z < rhs);
    }
    friend bool operator<=(const vec3f& lhs, const vec3f& rhs)
    {
        return (lhs.x <= rhs.x) && (lhs.y <= rhs.y) && (lhs.z <= rhs.z);
    }
    friend bool operator<=(const vec3f& lhs, float rhs)
    {
        return (lhs.x <= rhs) && (lhs.y <= rhs) && (lhs.z <= rhs);
    }
    friend bool operator>(const vec3f& lhs, const vec3f& rhs)
    {
        return (lhs.x > rhs.x) && (lhs.y > rhs.y) && (lhs.z > rhs.z);
    }
    friend bool operator>(const vec3f& lhs, float rhs)
    {
        return (lhs.x > rhs) && (lhs.y > rhs) && (lhs.z > rhs);
    }
    friend bool operator>=(const vec3f& lhs, const vec3f& rhs)
    {
        return (lhs.x >= rhs.x) && (lhs.y >= rhs.y) && (lhs.z >= rhs.z);
    }
    friend bool operator>=(const vec3f& lhs, float rhs)
    {
        return (lhs.x >= rhs) && (lhs.y >= rhs) && (lhs.z >= rhs);
    }

    // Singular operators
    float operator[](int index) const
    {
        return e[index];
    }
    float& operator[](int index)
    {
        return e[index];
    }
    bool operator==(const vec3f& rhs) const
    {
        return (x == rhs.x) && (y == rhs.y) && (z == rhs.z);
    }
    bool operator!=(const vec3f& rhs) const
    {
        return (x != rhs.x) || (y != rhs.y) || (z != rhs.z);
    }
    vec3f& operator+=(const vec3f& rhs)
    {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        return *this;
    }
    vec3f& operator-=(const vec3f& rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        return *this;
    }
    vec3f& operator*=(const vec3f& rhs)
    {
        x *= rhs.x;
        y *= rhs.y;
        z *= rhs.z;
        return *this;
    }
    vec3f& operator/=(const vec3f& rhs)
    {
        x /= rhs.x;
        y /= rhs.y;
        z /= rhs.z;
        return *this;
    }
    vec3f& operator+=(float rhs)
    {
        x += rhs;
        y += rhs;
        z += rhs;
        return *this;
    }
    vec3f& operator-=(float rhs)
    {
        x -= rhs;
        y -= rhs;
        z -= rhs;
        return *this;
    }
    vec3f& operator*=(float rhs)
    {
        x *= rhs;
        y *= rhs;
        z *= rhs;
        return *this;
    }
    vec3f& operator/=(float rhs)
    {
        const float InvRHS = 1.0f / rhs;
        x *= InvRHS;
        y *= InvRHS;
        z *= InvRHS;
        return *this;
    }

    // Unary operators
    vec3f operator-() const
    {
        return vec3f(-x, -y, -z);
    };

    // Static methods
    static vec3f cross(const vec3f& lhs, const vec3f& rhs)
    {
        return vec3f(lhs.y * rhs.z - lhs.z * rhs.y, lhs.z * rhs.x - lhs.x * rhs.z, lhs.x * rhs.y - lhs.y * rhs.x);
    }
    static float dot(const vec3f& lhs, const vec3f& rhs)
    {
        return (lhs.x * rhs.x) + (lhs.y * rhs.y) + (lhs.z * rhs.z);
    }

    float getLengthSq() const
    {
        return (x * x) + (y * y) + (z * z);
    }

    float getLength() const
    {
        float l = 0.0f;

        const float lsq = getLengthSq();
        if (lsq >= 0.0f)
            l = sqrtf(lsq);

        return l;
    }

    float normalize(float epsilon = 0.0f)
    {
        const float length = getLength();

        if (length > epsilon)
        {
            const float invLength = 1.0f / length;
            x *= invLength;
            y *= invLength;
            z *= invLength;
        }

        return length;
    }

    vec3f getNormal(float epsilon = 0.0f) const
    {
        vec3f normal;

        const float length = getLength();

        if (length > epsilon)
        {
            const float invLength = 1.0f / length;
            normal.x              = x * invLength;
            normal.y              = y * invLength;
            normal.z              = z * invLength;
        }
        else
        {
            normal = vec3f(0.0f);
        }

        return normal;
    }

    bool isNormalized(float epsilon = 0.000001f) const
    {
        const float d = fabsf(1.0f - getLength());
        return d <= epsilon;
    }

    void fromEuler(float yaw, float pitch)
    {
        const float sy = sinf(yaw);
        const float cy = cosf(yaw);
        const float sp = sinf(pitch);
        const float cp = cosf(pitch);

        x = cy * cp;
        y = sy * cp;
        z = sp;
    }
};

struct vec4f {
    union {
        struct {
            float x; ///< X component
            float y; ///< Y component
            float z; ///< Z component
            float w;
        }; ///< W component
        struct {
            float e[4];
        }; ///< Indexed components
    };

    // Default constructor
    vec4f() = default;

    // Constructors
    explicit vec4f(float xyzw) : x(xyzw), y(xyzw), z(xyzw), w(xyzw)
    {
    }
    explicit vec4f(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w)
    {
    }
    explicit vec4f(const vec3f& xyz, float _w) : x(xyz.x), y(xyz.y), z(xyz.z), w(_w)
    {
    }

    // Binary operators
    friend vec4f operator+(const vec4f& lhs, float rhs)
    {
        return vec4f(lhs.x + rhs, lhs.y + rhs, lhs.z + rhs, lhs.w + rhs);
    };
    friend vec4f operator-(const vec4f& lhs, float rhs)
    {
        return vec4f(lhs.x - rhs, lhs.y - rhs, lhs.z - rhs, lhs.w - rhs);
    };
    friend vec4f operator*(const vec4f& lhs, float rhs)
    {
        return vec4f(lhs.x * rhs, lhs.y * rhs, lhs.z * rhs, lhs.w * rhs);
    };
    friend vec4f operator/(const vec4f& lhs, float rhs)
    {
        const float InvRHS = 1.0f / rhs;
        return vec4f(lhs.x * InvRHS, lhs.y * InvRHS, lhs.z * InvRHS, lhs.w * InvRHS);
    };
    friend vec4f operator+(float lhs, const vec4f& rhs)
    {
        return vec4f(lhs + rhs.x, lhs + rhs.y, lhs + rhs.z, lhs + rhs.w);
    };
    friend vec4f operator-(float lhs, const vec4f& rhs)
    {
        return vec4f(lhs - rhs.x, lhs - rhs.y, lhs - rhs.z, lhs - rhs.w);
    };
    friend vec4f operator*(float lhs, const vec4f& rhs)
    {
        return vec4f(lhs * rhs.x, lhs * rhs.y, lhs * rhs.z, lhs * rhs.w);
    };
    friend vec4f operator/(float lhs, const vec4f& rhs)
    {
        return vec4f(lhs / rhs.x, lhs / rhs.y, lhs / rhs.z, lhs / rhs.w);
    };
    friend vec4f operator+(const vec4f& lhs, const vec4f& rhs)
    {
        return vec4f(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z, lhs.w + rhs.w);
    };
    friend vec4f operator-(const vec4f& lhs, const vec4f& rhs)
    {
        return vec4f(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z, lhs.w - rhs.w);
    };
    friend vec4f operator*(const vec4f& lhs, const vec4f& rhs)
    {
        return vec4f(lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z, lhs.w * rhs.w);
    };
    friend vec4f operator/(const vec4f& lhs, const vec4f& rhs)
    {
        return vec4f(lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z, lhs.w / rhs.w);
    };

    // Singular operators
    float operator[](int index) const
    {
        return e[index];
    }
    float& operator[](int index)
    {
        return e[index];
    }
    bool operator==(const vec4f& rhs) const
    {
        return (x == rhs.x) && (y == rhs.y) && (z == rhs.z) && (w == rhs.w);
    }
    bool operator!=(const vec4f& rhs) const
    {
        return (x != rhs.x) || (y != rhs.y) || (z != rhs.z) || (w != rhs.w);
    }
    vec4f& operator+=(const vec4f& rhs)
    {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        w += rhs.w;
        return *this;
    }
    vec4f& operator-=(const vec4f& rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        w -= rhs.w;
        return *this;
    }
    vec4f& operator*=(const vec4f& rhs)
    {
        x *= rhs.x;
        y *= rhs.y;
        z *= rhs.z;
        w *= rhs.w;
        return *this;
    }
    vec4f& operator/=(const vec4f& rhs)
    {
        x /= rhs.x;
        y /= rhs.y;
        z /= rhs.z;
        w /= rhs.w;
        return *this;
    }
    vec4f& operator+=(float rhs)
    {
        x += rhs;
        y += rhs;
        z += rhs;
        w += rhs;
        return *this;
    }
    vec4f& operator-=(float rhs)
    {
        x -= rhs;
        y -= rhs;
        z -= rhs;
        w -= rhs;
        return *this;
    }
    vec4f& operator*=(float rhs)
    {
        x *= rhs;
        y *= rhs;
        z *= rhs;
        w *= rhs;
        return *this;
    }
    vec4f& operator/=(float rhs)
    {
        const float InvRHS = 1.0f / rhs;
        x *= InvRHS;
        y *= InvRHS;
        z *= InvRHS;
        w *= InvRHS;
        return *this;
    }

    // Unary operators
    vec4f operator-() const
    {
        return vec4f(-x, -y, -z, -w);
    };

    //
    float getLengthSq() const
    {
        return (x * x) + (y * y) + (z * z) + (w * w);
    }

    float getLength() const
    {
        float l = 0.0f;

        const float lsq = getLengthSq();
        if (lsq >= 0.0f)
            l = sqrtf(lsq);

        return l;
    }

    float normalize(float epsilon = 0.0f)
    {
        const float length = getLength();

        if (length > epsilon)
        {
            const float invLength = 1.0f / length;
            x *= invLength;
            y *= invLength;
            z *= invLength;
            w *= invLength;
        }

        return length;
    }

    vec4f getNormal(float epsilon = 0.0f) const
    {
        vec4f normal;

        const float length = getLength();

        if (length > epsilon)
        {
            const float invLength = 1.0f / length;
            normal.x              = x * invLength;
            normal.y              = y * invLength;
            normal.z              = z * invLength;
            normal.w              = w * invLength;
        }
        else
        {
            normal = vec4f(0.0f);
        }

        return normal;
    }

    bool isNormalized(float epsilon = 0.000001f) const
    {
        const float d = fabsf(1.0f - getLength());
        return d <= epsilon;
    }
};

struct mat3f {
    union {
        struct {
            vec3f x; ///< X-Axis
            vec3f y; ///< Y-Axis
            vec3f z;
        }; ///< Z-Axis
        struct {
            float Xx, Xy, Xz; ///< X-Axis scalars
            float Yx, Yy, Yz; ///< Y-Axis scalars
            float Zx, Zy, Zz;
        }; ///< Z-Axis scalars
        struct {
            float M[3][3];
        }; ///< All components in a float array
        struct {
            float m[9];
        }; ///< All components in an array
        struct {
            vec3f e[3];
        }; ///< Indexed axes
    };

    mat3f() = default;

    vec3f operator[](int index) const
    {
        return e[index];
    }
    vec3f& operator[](int index)
    {
        return e[index];
    }

    mat3f operator*(const mat3f& rhs) const
    {
        mat3f ret;
        ret.x = x * rhs.Xx + y * rhs.Xy + z * rhs.Xz;
        ret.y = x * rhs.Yx + y * rhs.Yy + z * rhs.Yz;
        ret.z = x * rhs.Zx + y * rhs.Zy + z * rhs.Zz;
        return ret;
    }

    void clear(float value = 0.0)
    {
        x = vec3f(value);
        y = vec3f(value);
        z = vec3f(value);
    }

    void setIdentity()
    {
        x = vec3f(1.0, 0.0, 0.0);
        y = vec3f(0.0, 1.0, 0.0);
        z = vec3f(0.0, 0.0, 1.0);
    }

    void setAxisAngleRotation(const vec3f& _Axis, float _Angle)
    {
        const float ca           = cosf(_Angle);
        const float sa           = sinf(_Angle);
        const float oneMinusCosA = 1.0f - ca;

        x.x = oneMinusCosA * _Axis.x * _Axis.x + ca;
        x.y = oneMinusCosA * _Axis.x * _Axis.y - sa * _Axis.z;
        x.z = oneMinusCosA * _Axis.x * _Axis.z + sa * _Axis.y;

        y.x = oneMinusCosA * _Axis.y * _Axis.x + sa * _Axis.z;
        y.y = oneMinusCosA * _Axis.y * _Axis.y + ca;
        y.z = oneMinusCosA * _Axis.y * _Axis.z - sa * _Axis.x;

        z.x = oneMinusCosA * _Axis.z * _Axis.x - sa * _Axis.y;
        z.y = oneMinusCosA * _Axis.z * _Axis.y + sa * _Axis.x;
        z.z = oneMinusCosA * _Axis.z * _Axis.z + ca;
    }

    void fromQuaternion(vec4f u)
    {
        // assert(u.isNormalized());

        const float x2  = u.x + u.x;
        const float y2  = u.y + u.y;
        const float z2  = u.z + u.z;
        const float xx2 = u.x * x2;
        const float xy2 = u.x * y2;
        const float xz2 = u.x * z2;
        const float yy2 = u.y * y2;
        const float yz2 = u.y * z2;
        const float zz2 = u.z * z2;
        const float sx2 = u.w * x2;
        const float sy2 = u.w * y2;
        const float sz2 = u.w * z2;

        Xx = 1.0f - (yy2 + zz2);
        Xy = xy2 + sz2;
        Xz = xz2 - sy2;

        Yx = xy2 - sz2;
        Yy = 1.0f - (xx2 + zz2);
        Yz = yz2 + sx2;

        Zx = xz2 + sy2;
        Zy = yz2 - sx2;
        Zz = 1.0f - (xx2 + yy2);
    }
};

struct mat4f {
    union {
        struct {
            vec4f x; ///< X-Axis
            vec4f y; ///< Y-Axis
            vec4f z; ///< Z-Axis
            vec4f w;
        }; ///< W-Axis
        struct {
            float Xx, Xy, Xz, Xw; ///< X-Axis scalars
            float Yx, Yy, Yz, Yw; ///< Y-Axis scalars
            float Zx, Zy, Zz, Zw; ///< Z-Axis scalars
            float Wx, Wy, Wz, Ww;
        }; ///< W-Axis scalars
        struct {
            float M[4][4];
        }; ///< All components in a float array
        struct {
            float m[16];
        }; ///< All components in an array
        struct {
            vec4f e[4];
        }; ///< Indexed axes
    };

    mat4f() = default;

    mat4f(float _Xx,
          float _Xy,
          float _Xz,
          float _Xw,
          float _Yx,
          float _Yy,
          float _Yz,
          float _Yw,
          float _Zx,
          float _Zy,
          float _Zz,
          float _Zw,
          float _Wx,
          float _Wy,
          float _Wz,
          float _Ww)
    {
        Xx = _Xx;
        Xy = _Xy;
        Xz = _Xz;
        Xw = _Xw;
        Yx = _Yx;
        Yy = _Yy;
        Yz = _Yz;
        Yw = _Yw;
        Zx = _Zx;
        Zy = _Zy;
        Zz = _Zz;
        Zw = _Zw;
        Wx = _Wx;
        Wy = _Wy;
        Wz = _Wz;
        Ww = _Ww;
    }

    /*explicit mat4f(const mat4& mat)
    {
        Xx = (float) mat.Xx; Xy = (float) mat.Xy; Xz = (float) mat.Xz; Xw = (float) mat.Xw;
        Yx = (float) mat.Yx; Yy = (float) mat.Yy; Yz = (float) mat.Yz; Yw = (float) mat.Yw;
        Zx = (float) mat.Zx; Zy = (float) mat.Zy; Zz = (float) mat.Zz; Zw = (float) mat.Zw;
        Wx = (float) mat.Wx; Wy = (float) mat.Wy; Wz = (float) mat.Wz; Ww = (float) mat.Ww;
    }*/

    vec4f operator[](int index) const
    {
        return e[index];
    }
    vec4f& operator[](int index)
    {
        return e[index];
    }

    void setIdentity()
    {
        x = vec4f(1.0f, 0.0f, 0.0f, 0.0f);
        y = vec4f(0.0f, 1.0f, 0.0f, 0.0f);
        z = vec4f(0.0f, 0.0f, 1.0f, 0.0f);
        w = vec4f(0.0f, 0.0f, 0.0f, 1.0f);
    }

    void create(const mat3f& orientation, const vec3f& position)
    {
        Xx = orientation.Xx;
        Xy = orientation.Xy;
        Xz = orientation.Xz;
        Xw = 0.0f;
        Yx = orientation.Yx;
        Yy = orientation.Yy;
        Yz = orientation.Yz;
        Yw = 0.0f;
        Zx = orientation.Zx;
        Zy = orientation.Zy;
        Zz = orientation.Zz;
        Zw = 0.0f;
        Wx = position.x;
        Wy = position.y;
        Wz = position.z;
        Ww = 1.0f;
    }

    mat4f operator*(const mat4f& rhs) const
    {
        mat4f ret;
        ret.x = x * rhs.Xx + y * rhs.Xy + z * rhs.Xz + w * rhs.Xw;
        ret.y = x * rhs.Yx + y * rhs.Yy + z * rhs.Yz + w * rhs.Yw;
        ret.z = x * rhs.Zx + y * rhs.Zy + z * rhs.Zz + w * rhs.Zw;
        ret.w = x * rhs.Wx + y * rhs.Wy + z * rhs.Wz + w * rhs.Ww;
        return ret;
    }

    void setPerspective(float fovy, float aspectRatio, float znear, float zfar)
    {
        const float tanHalfFovy = tanf(fovy / 2.0f);

        const float Xs = 1.0f / (aspectRatio * tanHalfFovy);
        const float Ys = 1.0f / (tanHalfFovy);
        const float Zs = -(zfar + znear) / (zfar - znear);
        const float Us = -(2.0f * zfar * znear) / (zfar - znear);

        x = vec4f(Xs, 0.0f, 0.0f, 0.0f);
        y = vec4f(0.0f, Ys, 0.0f, 0.0f);
        z = vec4f(0.0f, 0.0f, Zs, -1.0f);
        w = vec4f(0.0f, 0.0f, Us, 0.0f);
    }

    void lookAt(const vec3f& eye, const vec3f& center, const vec3f& up)
    {
        const vec3f f = (center - eye).getNormal();
        const vec3f s = vec3f::cross(f, up).getNormal();
        const vec3f u = vec3f::cross(s, f);

        const float tx = -vec3f::dot(s, eye);
        const float ty = -vec3f::dot(u, eye);
        const float tz = vec3f::dot(f, eye);

        x.x = s.x;
        x.y = u.x;
        x.z = -f.x;
        x.w = 0.0f;
        y.x = s.y;
        y.y = u.y;
        y.z = -f.y;
        y.w = 0.0f;
        z.x = s.z;
        z.y = u.z;
        z.z = -f.z;
        z.w = 0.0f;
        w.x = tx;
        w.y = ty;
        w.z = tz;
        w.w = 1.0f;
    }
};

#if 0
    struct vec2
    {
        union
        {
            struct { double x;         ///< X component
                     double y; };      ///< Y component
            struct { double e[2]; };   ///< Indexed components
        };

        // Default constructor
        vec2() = default;

        // Constructors
        explicit vec2(double xy) : x(xy), y(xy) {}
        explicit vec2(double _x, double _y) : x(_x), y(_y) {}

        // Binary operators
        friend vec2 operator+ (const vec2& lhs, double rhs)      { return vec2(lhs.x + rhs, lhs.y + rhs); }
        friend vec2 operator- (const vec2& lhs, double rhs)      { return vec2(lhs.x - rhs, lhs.y - rhs); }
        friend vec2 operator* (const vec2& lhs, double rhs)      { return vec2(lhs.x * rhs, lhs.y * rhs); }
        friend vec2 operator/ (const vec2& lhs, double rhs)      { const double InvRHS = 1.0 / rhs; return vec2(lhs.x * InvRHS, lhs.y * InvRHS); }
        friend vec2 operator+ (double lhs, const vec2& rhs)      { return vec2(lhs + rhs.x, lhs + rhs.y); }
        friend vec2 operator- (double lhs, const vec2& rhs)      { return vec2(lhs - rhs.x, lhs - rhs.y); }
        friend vec2 operator* (double lhs, const vec2& rhs)      { return vec2(lhs * rhs.x, lhs * rhs.y); }
        friend vec2 operator/ (double lhs, const vec2& rhs)      { return vec2(lhs / rhs.x, lhs / rhs.y); }
        friend vec2 operator+ (const vec2& lhs, const vec2& rhs) { return vec2(lhs.x + rhs.x, lhs.y + rhs.y); }
        friend vec2 operator- (const vec2& lhs, const vec2& rhs) { return vec2(lhs.x - rhs.x, lhs.y - rhs.y); }
        friend vec2 operator* (const vec2& lhs, const vec2& rhs) { return vec2(lhs.x * rhs.x, lhs.y * rhs.y); }
        friend vec2 operator/ (const vec2& lhs, const vec2& rhs) { return vec2(lhs.x / rhs.x, lhs.y / rhs.y); }
        friend bool operator< (const vec2& lhs, const vec2& rhs) { return (lhs.x <  rhs.x) && (lhs.y <  rhs.y); }
        friend bool operator< (const vec2& lhs, double rhs)      { return (lhs.x <  rhs)   && (lhs.y <  rhs); }
        friend bool operator<=(const vec2& lhs, const vec2& rhs) { return (lhs.x <= rhs.x) && (lhs.y <= rhs.y); }
        friend bool operator<=(const vec2& lhs, double rhs)      { return (lhs.x <= rhs)   && (lhs.y <= rhs); }
        friend bool operator> (const vec2& lhs, const vec2& rhs) { return (lhs.x >  rhs.x) && (lhs.y >  rhs.y); }
        friend bool operator> (const vec2& lhs, double rhs)      { return (lhs.x >  rhs)   && (lhs.y >  rhs); }
        friend bool operator>=(const vec2& lhs, const vec2& rhs) { return (lhs.x >= rhs.x) && (lhs.y >= rhs.y); }
        friend bool operator>=(const vec2& lhs, double rhs)      { return (lhs.x >= rhs)   && (lhs.y >= rhs); }

        // Singular operators
        double  operator[](int index) const       { return e[index]; }
        double& operator[](int index)             { return e[index]; }
        bool    operator==(const vec2& rhs) const { return (x == rhs.x) && (y == rhs.y); }
        bool    operator!=(const vec2& rhs) const { return (x != rhs.x) || (y != rhs.y); }
        vec2&   operator+=(const vec2& rhs)       { x += rhs.x; y += rhs.y; return *this; }
        vec2&   operator-=(const vec2& rhs)       { x -= rhs.x; y -= rhs.y; return *this; }
        vec2&   operator*=(const vec2& rhs)       { x *= rhs.x; y *= rhs.y; return *this; }
        vec2&   operator/=(const vec2& rhs)       { x /= rhs.x; y /= rhs.y; return *this; }
        vec2&   operator+=(double rhs)            { x += rhs; y += rhs; return *this; }
        vec2&   operator-=(double rhs)            { x -= rhs; y -= rhs; return *this; }
        vec2&   operator*=(double rhs)            { x *= rhs; y *= rhs; return *this; }
        vec2&   operator/=(double rhs)            { const double InvRHS = 1.0 / rhs;  x *= InvRHS; y *= InvRHS; return *this; }

        // Unary operators
        vec2 operator-() const { return vec2(-x, -y); };

        double getLengthSq() const
        {
            return (x * x) + (y * y);
        }

        double getLength() const
        {
            double l = 0.0;

            const double lsq = getLengthSq();
            if (lsq >= 0.0)
                l = sqrt(lsq);

            return l;
        }

        double normalize(double epsilon = 0.0)
        {
            const double length = getLength();

            if (length > epsilon)
            {
                const double invLength = 1.0 / length;
                x *= invLength;
                y *= invLength;
            }

            return length;
        }

        vec2 getNormal(double epsilon = 0.0) const
        {
            vec2 normal;

            const double length = getLength();

            if (length > epsilon)
            {
                const double invLength = 1.0 / length;
                normal.x = x * invLength;
                normal.y = y * invLength;
            }
            else
            {
                normal = vec2(0.0);
            }

            return normal;
        }

        bool isNormalized(double epsilon = 0.000001) const
        {
            const double d = fabs(1.0 - getLength());
            return d <= epsilon;
        }
    };

    struct vec3
    {
        union
        {
            struct {
                double x;         ///< X component
                double y;         ///< Y component
                double z;
            };      ///< Z component
            struct { double e[3]; };   ///< Indexed components
        };

        // Default constructor
        vec3() = default;

        // Constructors
        explicit vec3(double xyz) : x(xyz), y(xyz), z(xyz) {}
        explicit vec3(double _x, double _y, double _z) : x(_x), y(_y), z(_z) {}
        explicit vec3(const vec2& xy, double _z) : x(xy.x), y(xy.y), z(_z) {}

        // Binary operators
        friend vec3 operator+ (const vec3& lhs, double rhs) { return vec3(lhs.x + rhs, lhs.y + rhs, lhs.z + rhs); }
        friend vec3 operator- (const vec3& lhs, double rhs) { return vec3(lhs.x - rhs, lhs.y - rhs, lhs.z - rhs); }
        friend vec3 operator* (const vec3& lhs, double rhs) { return vec3(lhs.x * rhs, lhs.y * rhs, lhs.z * rhs); }
        friend vec3 operator/ (const vec3& lhs, double rhs) { const double InvRHS = 1.0 / rhs; return vec3(lhs.x * InvRHS, lhs.y * InvRHS, lhs.z * InvRHS); }
        friend vec3 operator+ (double lhs, const vec3& rhs) { return vec3(lhs + rhs.x, lhs + rhs.y, lhs + rhs.z); }
        friend vec3 operator- (double lhs, const vec3& rhs) { return vec3(lhs - rhs.x, lhs - rhs.y, lhs - rhs.z); }
        friend vec3 operator* (double lhs, const vec3& rhs) { return vec3(lhs * rhs.x, lhs * rhs.y, lhs * rhs.z); }
        friend vec3 operator/ (double lhs, const vec3& rhs) { return vec3(lhs / rhs.x, lhs / rhs.y, lhs / rhs.z); }
        friend vec3 operator+ (const vec3& lhs, const vec3& rhs) { return vec3(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z); }
        friend vec3 operator- (const vec3& lhs, const vec3& rhs) { return vec3(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z); }
        friend vec3 operator* (const vec3& lhs, const vec3& rhs) { return vec3(lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z); }
        friend vec3 operator/ (const vec3& lhs, const vec3& rhs) { return vec3(lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z); }
        friend bool operator< (const vec3& lhs, const vec3& rhs) { return (lhs.x < rhs.x) && (lhs.y < rhs.y) && (lhs.z < rhs.z); }
        friend bool operator< (const vec3& lhs, double rhs) { return (lhs.x < rhs) && (lhs.y < rhs) && (lhs.z < rhs); }
        friend bool operator<=(const vec3& lhs, const vec3& rhs) { return (lhs.x <= rhs.x) && (lhs.y <= rhs.y) && (lhs.z <= rhs.z); }
        friend bool operator<=(const vec3& lhs, double rhs) { return (lhs.x <= rhs) && (lhs.y <= rhs) && (lhs.z <= rhs); }
        friend bool operator> (const vec3& lhs, const vec3& rhs) { return (lhs.x > rhs.x) && (lhs.y > rhs.y) && (lhs.z > rhs.z); }
        friend bool operator> (const vec3& lhs, double rhs) { return (lhs.x > rhs) && (lhs.y > rhs) && (lhs.z > rhs); }
        friend bool operator>=(const vec3& lhs, const vec3& rhs) { return (lhs.x >= rhs.x) && (lhs.y >= rhs.y) && (lhs.z >= rhs.z); }
        friend bool operator>=(const vec3& lhs, double rhs) { return (lhs.x >= rhs) && (lhs.y >= rhs) && (lhs.z >= rhs); }

        // Singular operators
        double  operator[](int index) const { return e[index]; }
        double& operator[](int index) { return e[index]; }
        bool    operator==(const vec3& rhs) const { return (x == rhs.x) && (y == rhs.y) && (z == rhs.z); }
        bool    operator!=(const vec3& rhs) const { return (x != rhs.x) || (y != rhs.y) || (z != rhs.z); }
        vec3& operator+=(const vec3& rhs) { x += rhs.x; y += rhs.y; z += rhs.z; return *this; }
        vec3& operator-=(const vec3& rhs) { x -= rhs.x; y -= rhs.y; z -= rhs.z; return *this; }
        vec3& operator*=(const vec3& rhs) { x *= rhs.x; y *= rhs.y; z *= rhs.z; return *this; }
        vec3& operator/=(const vec3& rhs) { x /= rhs.x; y /= rhs.y; z /= rhs.z; return *this; }
        vec3& operator+=(double rhs) { x += rhs; y += rhs; z += rhs; return *this; }
        vec3& operator-=(double rhs) { x -= rhs; y -= rhs; z -= rhs; return *this; }
        vec3& operator*=(double rhs) { x *= rhs; y *= rhs; z *= rhs; return *this; }
        vec3& operator/=(double rhs) { const double InvRHS = 1.0 / rhs;  x *= InvRHS; y *= InvRHS; z *= InvRHS; return *this; }

        // Unary operators
        vec3 operator-() const { return vec3(-x, -y, -z); };

        // Static methods
        static vec3  cross(const vec3& lhs, const vec3& rhs) { return vec3(lhs.y * rhs.z - lhs.z * rhs.y, lhs.z * rhs.x - lhs.x * rhs.z, lhs.x * rhs.y - lhs.y * rhs.x); }
        static double dot(const vec3& lhs, const vec3& rhs) { return (lhs.x * rhs.x) + (lhs.y * rhs.y) + (lhs.z * rhs.z); }

        double getLengthSq() const
        {
            return (x * x) + (y * y) + (z * z);
        }

        double getLength() const
        {
            double l = 0.0;

            const double lsq = getLengthSq();
            if (lsq >= 0.0)
                l = sqrt(lsq);

            return l;
        }

        double normalize(double epsilon = 0.0)
        {
            const double length = getLength();

            if (length > epsilon)
            {
                const double invLength = 1.0 / length;
                x *= invLength;
                y *= invLength;
                z *= invLength;
            }

            return length;
        }

        vec3 getNormal(double epsilon = 0.0) const
        {
            vec3 normal;

            const double length = getLength();

            if (length > epsilon)
            {
                const double invLength = 1.0 / length;
                normal.x = x * invLength;
                normal.y = y * invLength;
                normal.z = z * invLength;
            }
            else
            {
                normal = vec3(0.0);
            }

            return normal;
        }

        bool isNormalized(double epsilon = 0.000001) const
        {
            const double d = fabs(1.0 - getLength());
            return d <= epsilon;
        }

        void fromEuler(double yaw, double pitch)
        {
            const double sy = sin(yaw);
            const double cy = cos(yaw);
            const double sp = sin(pitch);
            const double cp = cos(pitch);

            x = cy * cp;
            y = sy * cp;
            z = sp;
        }
    };

    struct vec4
    {
        union
        {
            struct {
                double x;         ///< X component
                double y;         ///< Y component
                double z;         ///< Z component
                double w;
            };      ///< W component
            struct { double e[4]; };   ///< Indexed components
        };

        // Default constructor
        vec4() = default;

        // Constructors
        explicit vec4(double xyzw) : x(xyzw), y(xyzw), z(xyzw), w(xyzw) {}
        explicit vec4(double _x, double _y, double _z, double _w) : x(_x), y(_y), z(_z), w(_w) {}
        explicit vec4(const vec3& xyz, double _w) : x(xyz.x), y(xyz.y), z(xyz.z), w(_w) {}

        // Binary operators
        friend vec4 operator+(const vec4& lhs, double rhs) { return vec4(lhs.x + rhs, lhs.y + rhs, lhs.z + rhs, lhs.w + rhs); };
        friend vec4 operator-(const vec4& lhs, double rhs) { return vec4(lhs.x - rhs, lhs.y - rhs, lhs.z - rhs, lhs.w - rhs); };
        friend vec4 operator*(const vec4& lhs, double rhs) { return vec4(lhs.x * rhs, lhs.y * rhs, lhs.z * rhs, lhs.w * rhs); };
        friend vec4 operator/(const vec4& lhs, double rhs) { const double InvRHS = 1.0 / rhs; return vec4(lhs.x * InvRHS, lhs.y * InvRHS, lhs.z * InvRHS, lhs.w * InvRHS); };
        friend vec4 operator+(double lhs, const vec4& rhs) { return vec4(lhs + rhs.x, lhs + rhs.y, lhs + rhs.z, lhs + rhs.w); };
        friend vec4 operator-(double lhs, const vec4& rhs) { return vec4(lhs - rhs.x, lhs - rhs.y, lhs - rhs.z, lhs - rhs.w); };
        friend vec4 operator*(double lhs, const vec4& rhs) { return vec4(lhs * rhs.x, lhs * rhs.y, lhs * rhs.z, lhs * rhs.w); };
        friend vec4 operator/(double lhs, const vec4& rhs) { return vec4(lhs / rhs.x, lhs / rhs.y, lhs / rhs.z, lhs / rhs.w); };
        friend vec4 operator+(const vec4& lhs, const vec4& rhs) { return vec4(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z, lhs.w + rhs.w); };
        friend vec4 operator-(const vec4& lhs, const vec4& rhs) { return vec4(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z, lhs.w - rhs.w); };
        friend vec4 operator*(const vec4& lhs, const vec4& rhs) { return vec4(lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z, lhs.w * rhs.w); };
        friend vec4 operator/(const vec4& lhs, const vec4& rhs) { return vec4(lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z, lhs.w / rhs.w); };

        // Singular operators
        double  operator[](int index) const { return e[index]; }
        double& operator[](int index) { return e[index]; }
        bool    operator==(const vec4& rhs) const { return (x == rhs.x) && (y == rhs.y) && (z == rhs.z) && (w == rhs.w); }
        bool    operator!=(const vec4& rhs) const { return (x != rhs.x) || (y != rhs.y) || (z != rhs.z) || (w != rhs.w); }
        vec4& operator+=(const vec4& rhs) { x += rhs.x; y += rhs.y; z += rhs.z; w += rhs.w; return *this; }
        vec4& operator-=(const vec4& rhs) { x -= rhs.x; y -= rhs.y; z -= rhs.z; w -= rhs.w; return *this; }
        vec4& operator*=(const vec4& rhs) { x *= rhs.x; y *= rhs.y; z *= rhs.z; w *= rhs.w; return *this; }
        vec4& operator/=(const vec4& rhs) { x /= rhs.x; y /= rhs.y; z /= rhs.z; w /= rhs.w; return *this; }
        vec4& operator+=(double rhs) { x += rhs; y += rhs; z += rhs; w += rhs; return *this; }
        vec4& operator-=(double rhs) { x -= rhs; y -= rhs; z -= rhs; w -= rhs; return *this; }
        vec4& operator*=(double rhs) { x *= rhs; y *= rhs; z *= rhs; w *= rhs; return *this; }
        vec4& operator/=(double rhs) { const double InvRHS = 1.0 / rhs; x *= InvRHS; y *= InvRHS; z *= InvRHS; w *= InvRHS; return *this; }

        // Unary operators
        vec4 operator-() const { return vec4(-x, -y, -z, -w); };

        //
        double getLengthSq() const
        {
            return (x * x) + (y * y) + (z * z) + (w * w);
        }

        double getLength() const
        {
            double l = 0.0;

            const double lsq = getLengthSq();
            if (lsq >= 0.0)
                l = sqrt(lsq);

            return l;
        }

        double normalize(double epsilon = 0.0)
        {
            const double length = getLength();

            if (length > epsilon)
            {
                const double invLength = 1.0 / length;
                x *= invLength;
                y *= invLength;
                z *= invLength;
                w *= invLength;
            }

            return length;
        }

        vec4 getNormal(double epsilon = 0.0) const
        {
            vec4 normal;

            const double length = getLength();

            if (length > epsilon)
            {
                const double invLength = 1.0 / length;
                normal.x = x * invLength;
                normal.y = y * invLength;
                normal.z = z * invLength;
                normal.w = w * invLength;
            }
            else
            {
                normal = vec4(0.0);
            }

            return normal;
        }

        bool isNormalized(double epsilon = 0.000001) const
        {
            const double d = fabs(1.0 - getLength());
            return d <= epsilon;
        }
    };

    struct mat3
    {
        union
        {
            struct {
                vec3   x;                ///< X-Axis
                vec3   y;                ///< Y-Axis
                vec3   z;
            };             ///< Z-Axis
            struct {
                double Xx, Xy, Xz;       ///< X-Axis scalars
                double Yx, Yy, Yz;       ///< Y-Axis scalars
                double Zx, Zy, Zz;
            };    ///< Z-Axis scalars
            struct { double M[3][3]; };       ///< All components in a double array
            struct { double m[9]; };          ///< All components in an array
            struct { vec3   e[3]; };          ///< Indexed axes
        };

        mat3() = default;

        vec3  operator[](int index) const { return e[index]; }
        vec3& operator[](int index) { return e[index]; }

        mat3 operator*(const mat3& rhs) const
        {
            mat3 ret;
            ret.x = x * rhs.Xx + y * rhs.Xy + z * rhs.Xz;
            ret.y = x * rhs.Yx + y * rhs.Yy + z * rhs.Yz;
            ret.z = x * rhs.Zx + y * rhs.Zy + z * rhs.Zz;
            return ret;
        }

        void clear(double value = 0.0)
        {
            x = vec3(value);
            y = vec3(value);
            z = vec3(value);
        }

        void setIdentity()
        {
            x = vec3(1.0, 0.0, 0.0);
            y = vec3(0.0, 1.0, 0.0);
            z = vec3(0.0, 0.0, 1.0);
        }

        void setAxisAngleRotation(const vec3& _Axis, double _Angle)
        {
            const double ca = cos(_Angle);
            const double sa = sin(_Angle);
            const double oneMinusCosA = 1.0 - ca;

            x.x = oneMinusCosA * _Axis.x * _Axis.x + ca;
            x.y = oneMinusCosA * _Axis.x * _Axis.y - sa * _Axis.z;
            x.z = oneMinusCosA * _Axis.x * _Axis.z + sa * _Axis.y;

            y.x = oneMinusCosA * _Axis.y * _Axis.x + sa * _Axis.z;
            y.y = oneMinusCosA * _Axis.y * _Axis.y + ca;
            y.z = oneMinusCosA * _Axis.y * _Axis.z - sa * _Axis.x;

            z.x = oneMinusCosA * _Axis.z * _Axis.x - sa * _Axis.y;
            z.y = oneMinusCosA * _Axis.z * _Axis.y + sa * _Axis.x;
            z.z = oneMinusCosA * _Axis.z * _Axis.z + ca;
        }

        void fromQuaternion(vec4 u)
        {
            //assert(u.isNormalized());

            const double x2 = u.x + u.x;
            const double y2 = u.y + u.y;
            const double z2 = u.z + u.z;
            const double xx2 = u.x * x2;
            const double xy2 = u.x * y2;
            const double xz2 = u.x * z2;
            const double yy2 = u.y * y2;
            const double yz2 = u.y * z2;
            const double zz2 = u.z * z2;
            const double sx2 = u.w * x2;
            const double sy2 = u.w * y2;
            const double sz2 = u.w * z2;

            Xx = 1.0 - (yy2 + zz2);
            Xy = xy2 + sz2;
            Xz = xz2 - sy2;

            Yx = xy2 - sz2;
            Yy = 1.0 - (xx2 + zz2);
            Yz = yz2 + sx2;

            Zx = xz2 + sy2;
            Zy = yz2 - sx2;
            Zz = 1.0 - (xx2 + yy2);
        }
    };

    struct mat4
    {
        union
        {
            struct {
                vec4   x;                     ///< X-Axis
                vec4   y;                     ///< Y-Axis
                vec4   z;                     ///< Z-Axis
                vec4   w;
            };                  ///< W-Axis
            struct {
                double Xx, Xy, Xz, Xw;        ///< X-Axis scalars
                double Yx, Yy, Yz, Yw;        ///< Y-Axis scalars
                double Zx, Zy, Zz, Zw;        ///< Z-Axis scalars
                double Wx, Wy, Wz, Ww;
            };     ///< W-Axis scalars
            struct { double M[4][4]; };            ///< All components in a double array
            struct { double m[16]; };              ///< All components in an array
            struct { vec4   e[4]; };               ///< Indexed axes
        };

        mat4() = default;

        mat4
        (
            double _Xx, double _Xy, double _Xz, double _Xw,
            double _Yx, double _Yy, double _Yz, double _Yw,
            double _Zx, double _Zy, double _Zz, double _Zw,
            double _Wx, double _Wy, double _Wz, double _Ww)
        {
            Xx = _Xx; Xy = _Xy; Xz = _Xz; Xw = _Xw;
            Yx = _Yx; Yy = _Yy; Yz = _Yz; Yw = _Yw;
            Zx = _Zx; Zy = _Zy; Zz = _Zz; Zw = _Zw;
            Wx = _Wx; Wy = _Wy; Wz = _Wz; Ww = _Ww;
        }

        vec4  operator[](int index) const { return e[index]; }
        vec4& operator[](int index) { return e[index]; }

        void clear(double value = 0.0)
        {
            x = vec4(value);
            y = vec4(value);
            z = vec4(value);
            w = vec4(value);
        }

        void setIdentity()
        {
            x = vec4(1.0, 0.0, 0.0, 0.0);
            y = vec4(0.0, 1.0, 0.0, 0.0);
            z = vec4(0.0, 0.0, 1.0, 0.0);
            w = vec4(0.0, 0.0, 0.0, 1.0);
        }

        vec3 getPosition() const
        {
            return vec3(w.x, w.y, w.z) / w.w;
        }

        void create(const mat3& orientation, const vec3& position)
        {
            Xx = orientation.Xx; Xy = orientation.Xy; Xz = orientation.Xz; Xw = 0.0;
            Yx = orientation.Yx; Yy = orientation.Yy; Yz = orientation.Yz; Yw = 0.0;
            Zx = orientation.Zx; Zy = orientation.Zy; Zz = orientation.Zz; Zw = 0.0;
            Wx = position.x;     Wy = position.y;     Wz = position.z;     Ww = 1.0;
        }

        mat4 operator*(const mat4& rhs) const
        {
            mat4 ret;
            ret.x = x * rhs.Xx + y * rhs.Xy + z * rhs.Xz + w * rhs.Xw;
            ret.y = x * rhs.Yx + y * rhs.Yy + z * rhs.Yz + w * rhs.Yw;
            ret.z = x * rhs.Zx + y * rhs.Zy + z * rhs.Zz + w * rhs.Zw;
            ret.w = x * rhs.Wx + y * rhs.Wy + z * rhs.Wz + w * rhs.Ww;
            return ret;
        }

        void setPerspective(double fovy, double aspectRatio, double znear, double zfar)
        {
            const double tanHalfFovy = tan(fovy / 2.0);

            const double Xs = 1.0 / (aspectRatio * tanHalfFovy);
            const double Ys = 1.0 / (tanHalfFovy);
            const double Zs = -(zfar + znear) / (zfar - znear);
            const double Us = -(2.0 * zfar * znear) / (zfar - znear);

            x = vec4(Xs, 0.0, 0.0, 0.0);
            y = vec4(0.0, Ys, 0.0, 0.0);
            z = vec4(0.0, 0.0, Zs, -1.0);
            w = vec4(0.0, 0.0, Us, 0.0);
        }

        void setOrthographic(double left, double right, double bottom, double top, double znear, double zfar)
        {
            const double Xs = 2.0 / (right - left);
            const double Ys = 2.0 / (top - bottom);
            const double Zs = -2.0 / (zfar - znear);

            const double Ux = -(right + left) / (right - left);
            const double Uy = -(top + bottom) / (top - bottom);
            const double Uz = -(zfar + znear) / (zfar - znear);

            x = vec4(Xs, 0.0, 0.0, 0.0);
            y = vec4(0.0, Ys, 0.0, 0.0);
            z = vec4(0.0, 0.0, Zs, 0.0);
            w = vec4(Ux, Uy, Uz, 1.0);
        }

        void setAxisAngleRotation(const vec3& _Axis, double _Angle)
        {
            // Compute 3d rotation matrix
            mat3 Rm;
            Rm.setAxisAngleRotation(_Axis, _Angle);

            // Set 4d matrix
            x.x = Rm.x.x; x.y = Rm.x.y; x.z = Rm.x.z; x.w = 0.0;
            y.x = Rm.y.x; y.y = Rm.y.y; y.z = Rm.y.z; y.w = 0.0;
            z.x = Rm.z.x; z.y = Rm.z.y; z.z = Rm.z.z; z.w = 0.0;
            w.x = 0.0;   w.y = 0.0;   w.z = 0.0;   w.w = 1.0;
        }

        void lookAt(const vec3& eye, const vec3& center, const vec3& up)
        {
            const vec3 f = (center - eye).getNormal();
            const vec3 s = vec3::cross(f, up).getNormal();
            const vec3 u = vec3::cross(s, f);

            const double tx = -vec3::dot(s, eye);
            const double ty = -vec3::dot(u, eye);
            const double tz = vec3::dot(f, eye);

            x.x = s.x; x.y = u.x; x.z = -f.x; x.w = 0.0;
            y.x = s.y; y.y = u.y; y.z = -f.y; y.w = 0.0;
            z.x = s.z; z.y = u.z; z.z = -f.z; z.w = 0.0;
            w.x = tx;  w.y = ty;  w.z = tz;   w.w = 1.0;
        }

        mat4 getInverse() const
        {
            // Compute the determinant
            const double a0 = m[0] * m[5] - m[1] * m[4];
            const double a1 = m[0] * m[6] - m[2] * m[4];
            const double a2 = m[0] * m[7] - m[3] * m[4];
            const double a3 = m[1] * m[6] - m[2] * m[5];
            const double a4 = m[1] * m[7] - m[3] * m[5];
            const double a5 = m[2] * m[7] - m[3] * m[6];
            const double b0 = m[8] * m[13] - m[9] * m[12];
            const double b1 = m[8] * m[14] - m[10] * m[12];
            const double b2 = m[8] * m[15] - m[11] * m[12];
            const double b3 = m[9] * m[14] - m[10] * m[13];
            const double b4 = m[9] * m[15] - m[11] * m[13];
            const double b5 = m[10] * m[15] - m[11] * m[14];

            const double det = a0 * b5 - a1 * b4 + a2 * b3 + a3 * b2 - a4 * b1 + a5 * b0;

            if (det != 0.0)
            {
                const double invDet = 1.0 / det;

                return mat4
                (
                    (m[5] * b5 - m[6] * b4 + m[7] * b3) * invDet,
                    (-m[1] * b5 + m[2] * b4 - m[3] * b3) * invDet,
                    (m[13] * a5 - m[14] * a4 + m[15] * a3) * invDet,
                    (-m[9] * a5 + m[10] * a4 - m[11] * a3) * invDet,
                    (-m[4] * b5 + m[6] * b2 - m[7] * b1) * invDet,
                    (m[0] * b5 - m[2] * b2 + m[3] * b1) * invDet,
                    (-m[12] * a5 + m[14] * a2 - m[15] * a1) * invDet,
                    (m[8] * a5 - m[10] * a2 + m[11] * a1) * invDet,
                    (m[4] * b4 - m[5] * b2 + m[7] * b0) * invDet,
                    (-m[0] * b4 + m[1] * b2 - m[3] * b0) * invDet,
                    (m[12] * a4 - m[13] * a2 + m[15] * a0) * invDet,
                    (-m[8] * a4 + m[9] * a2 - m[11] * a0) * invDet,
                    (-m[4] * b3 + m[5] * b1 - m[6] * b0) * invDet,
                    (m[0] * b3 - m[1] * b1 + m[2] * b0) * invDet,
                    (-m[12] * a3 + m[13] * a1 - m[14] * a0) * invDet,
                    (m[8] * a3 - m[9] * a1 + m[10] * a0) * invDet
                );
            }
            else
            {
                // Return zero-matrix if there's no determinant
                return mat4(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
            }
        }

        mat4 getTranspose() const
        {
            mat4 transpose;
            transpose.Xx = Xx; transpose.Yx = Xy; transpose.Zx = Xz; transpose.Wx = Xw;
            transpose.Xy = Yx; transpose.Yy = Yy; transpose.Zy = Yz; transpose.Wy = Yw;
            transpose.Xz = Zx; transpose.Yz = Zy; transpose.Zz = Zz; transpose.Wz = Zw;
            transpose.Xw = Wx; transpose.Yw = Wy; transpose.Zw = Wz; transpose.Ww = Ww;
            return transpose;
        }
    };
#endif

#pragma pack(pop)

inline float DegreesToRadians(float degrees)
{
    return degrees * 0.01745329251994329576923690768489f;
}

} // namespace math
} // namespace renderer
} // namespace leia
