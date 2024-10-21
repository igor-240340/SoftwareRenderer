#pragma once

#include "Vec4.h"

// Column-major order.
class Mat4x4 {
public:
    Mat4x4() = default;
    ~Mat4x4() = default;
    Mat4x4(float value);

    Vec4 operator*(const Vec4& vec) const;

    static Mat4x4 create_perspective(float fov_vert_rad, float aspect_ratio, float near, float far);

private:
    float data[16];
};
