#pragma once

#include "Vec4f.h"

class Vec3f {
public:
    float x;
    float y;
    float z;

public:
    static const Vec3f zero;

public:
    Vec3f() = default;
    ~Vec3f() = default;
    Vec3f(float x, float y, float z);
    Vec3f(Vec4f vec4);

    float length() const;
    float length_squared() const;
    Vec3f normalized() const;

    Vec3f operator/(float scalar) const;
    Vec3f operator+(const Vec3f& b) const;
    Vec3f operator-(const Vec3f& b) const;
    Vec3f operator*(float scalar) const;
    Vec3f operator-() const;

    static float dot(const Vec3f& a, const Vec3f& b);
    static Vec3f cross(const Vec3f& a, const Vec3f& b);
};
