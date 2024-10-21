#pragma once

#include "Vec3.h"

class Vec4 {
public:
    float x;
    float y;
    float z;
    float w;

public:
    Vec4() = default;
    ~Vec4() = default;
    Vec4(const Vec3& vec3, float w = 1.0f);
    Vec4(float x, float y, float z, float w = 1.0f);

    Vec4 operator/(float scalar) const;
};
