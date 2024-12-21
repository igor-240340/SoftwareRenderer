#include <cmath>
#include <algorithm>

#include "Mat4f.h"

// Identity.
Mat4f::Mat4f() {
    std::fill(data.begin(), data.end(), 0.0f);

    data.at(0) = 1.0f;
    data.at(5) = 1.0f;
    data.at(10) = 1.0f;
    data.at(15) = 1.0f;
}

Vec4f Mat4f::operator*(const Vec4f& vec) const {
    return Vec4f(
        data.at(0) * vec.x + data.at(4) * vec.y + data.at(8) * vec.z + data.at(12) * vec.w,
        data.at(1) * vec.x + data.at(5) * vec.y + data.at(9) * vec.z + data.at(13) * vec.w,
        data.at(2) * vec.x + data.at(6) * vec.y + data.at(10) * vec.z + data.at(14) * vec.w,
        data.at(3) * vec.x + data.at(7) * vec.y + data.at(11) * vec.z + data.at(15) * vec.w
    );
}

Mat4f Mat4f::create_identity() {
    return Mat4f{};
}

Mat4f Mat4f::create_perspective(float fov_vert_rad, float aspect_ratio, float near, float far) {
    Mat4f mat_proj{};

    mat_proj.data.at(0) = 1.0f / (aspect_ratio * std::tanf(fov_vert_rad / 2.0f));
    mat_proj.data.at(5) = 1.0f / std::tanf(fov_vert_rad / 2.0f);
    mat_proj.data.at(10) = far / (far - near);
    mat_proj.data.at(11) = -1.0f;
    mat_proj.data.at(14) = -(far * near) / (far - near);
    mat_proj.data.at(15) = 0.0f;

    return mat_proj;
}

Mat4f Mat4f::create_viewport(int w, int h) {
    Mat4f mat_view{};

    mat_view.data.at(0) = w / 2;
    mat_view.data.at(5) = h / 2;
    mat_view.data.at(11) = -1.0f;
    mat_view.data.at(12) = w / 2;
    mat_view.data.at(13) = h / 2;

    return mat_view;
}

Mat4f Mat4f::create_look_at(const Vec3f& pos, const Vec3f& look_dir, const Vec3f& up) {
    Vec3f right = Vec3f::cross(up, look_dir);

    Mat4f view_mat{};
    view_mat.data.at(0) = right.x;
    view_mat.data.at(4) = right.y;
    view_mat.data.at(8) = right.z;
    view_mat.data.at(12) = -Vec3f::dot(right, pos);

    view_mat.data.at(1) = up.x;
    view_mat.data.at(5) = up.y;
    view_mat.data.at(9) = up.z;
    view_mat.data.at(13) = -Vec3f::dot(up, pos);

    view_mat.data.at(2) = look_dir.x;
    view_mat.data.at(6) = look_dir.y;
    view_mat.data.at(10) = look_dir.z;
    view_mat.data.at(14) = -Vec3f::dot(look_dir, pos);

    return view_mat;
}

Mat4f Mat4f::create_translation(const Vec3f& v) {
    Mat4f mat_trans{};

    mat_trans.data.at(12) = v.x;
    mat_trans.data.at(13) = v.y;
    mat_trans.data.at(14) = v.z;

    return mat_trans;
}

Mat4f Mat4f::create_rotation_x(const float angle_rad) {
    Mat4f mat_rot{};

    mat_rot.data.at(5) = std::cosf(angle_rad);
    mat_rot.data.at(6) = std::sinf(angle_rad);
    mat_rot.data.at(9) = -std::sinf(angle_rad);
    mat_rot.data.at(10) = std::cosf(angle_rad);

    return mat_rot;
}

Mat4f Mat4f::create_rotation_y(const float angle_rad) {
    Mat4f mat_rot{};

    mat_rot.data.at(0) = std::cosf(angle_rad);
    mat_rot.data.at(2) = -std::sinf(angle_rad);
    mat_rot.data.at(8) = std::sinf(angle_rad);
    mat_rot.data.at(10) = std::cosf(angle_rad);

    return mat_rot;
}

Mat4f Mat4f::create_rotation_z(const float angle_rad) {
    Mat4f mat_rot{};

    mat_rot.data.at(0) = std::cosf(angle_rad);
    mat_rot.data.at(1) = std::sinf(angle_rad);
    mat_rot.data.at(4) = -std::sinf(angle_rad);
    mat_rot.data.at(5) = std::cosf(angle_rad);

    return mat_rot;
}
