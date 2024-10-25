#pragma once

#include "Vec4f.h"

// Column-major order.
class Mat4f {
public:
    Mat4f() = default;
    ~Mat4f() = default;
    Mat4f(float value);

    Vec4f operator*(const Vec4f& vec) const;

    static Mat4f create_perspective(float fov_vert_rad, float aspect_ratio, float near, float far);
    static Mat4f create_viewport(int w, int h);

private:
    float data[16];
};
