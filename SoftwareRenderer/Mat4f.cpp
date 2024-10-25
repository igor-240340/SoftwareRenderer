#include <cmath>

#include "Mat4f.h"

Mat4f::Mat4f(float value) {
    for (int i = 0; i < 16; i++) {
        data[i] = value;
    }
}

Vec4f Mat4f::operator*(const Vec4f& vec) const {
    return Vec4f(
        data[0] * vec.x + data[4] * vec.y + data[8] * vec.z + data[12] * vec.w,
        data[1] * vec.x + data[5] * vec.y + data[9] * vec.z + data[13] * vec.w,
        data[2] * vec.x + data[6] * vec.y + data[10] * vec.z + data[14] * vec.w,
        data[3] * vec.x + data[7] * vec.y + data[11] * vec.z + data[15] * vec.w
    );
}

Mat4f Mat4f::create_perspective(float fov_vert_rad, float aspect_ratio, float near, float far) {
    Mat4f mat_proj{ 0.0f };

    mat_proj.data[0] = 1.0f / (aspect_ratio * std::tanf(fov_vert_rad / 2.0f));
    mat_proj.data[5] = 1.0f / std::tanf(fov_vert_rad / 2.0f);
    mat_proj.data[10] = far / (far - near);
    mat_proj.data[11] = -1.0f;
    mat_proj.data[14] = -(far * near) / (far - near);

    return mat_proj;
}

Mat4f Mat4f::create_viewport(int w, int h)
{
    Mat4f mat_view{ 0.0f };

    mat_view.data[0] = w / 2;
    mat_view.data[5] = h / 2;
    mat_view.data[10] = 1.0f;
    mat_view.data[11] = -1.0f;
    mat_view.data[12] = w / 2;
    mat_view.data[13] = h / 2;
    mat_view.data[15] = 1.0f;

    return mat_view;
}
