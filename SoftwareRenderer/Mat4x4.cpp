#include <cmath>

#include "Mat4x4.h"

Mat4x4::Mat4x4(float value) {
    for (int i = 0; i < 16; i++) {
        data[i] = value;
    }
}

Vec4 Mat4x4::operator*(const Vec4& vec) const {
    return Vec4(
        data[0] * vec.x + data[4] * vec.y + data[8] * vec.z + data[12] * vec.w,
        data[1] * vec.x + data[5] * vec.y + data[9] * vec.z + data[13] * vec.w,
        data[2] * vec.x + data[6] * vec.y + data[10] * vec.z + data[14] * vec.w,
        data[3] * vec.x + data[7] * vec.y + data[11] * vec.z + data[15] * vec.w
    );
}

Mat4x4 Mat4x4::create_perspective(float fov_vert_rad, float aspect_ratio, float near, float far) {
    Mat4x4 mat_proj{ 0.0f };

    mat_proj.data[0] = 1.0f / (aspect_ratio * std::tanf(fov_vert_rad / 2.0f));
    mat_proj.data[5] = 1.0f / std::tanf(fov_vert_rad / 2.0f);
    mat_proj.data[10] = far / (far - near);
    mat_proj.data[11] = -1.0f;
    mat_proj.data[14] = -(far * near) / (far - near);

    return mat_proj;
}
