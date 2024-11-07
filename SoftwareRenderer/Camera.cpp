#include <cmath>

#include "Camera.h"

Camera::Camera(Vec3f pos, float yaw_rad, float pitch_rad) {
    this->pos = pos;
    this->yaw_rad = yaw_rad;
    this->pitch_rad = pitch_rad;
}

Mat4f Camera::get_view_mat() {
    Vec3f forward{
        std::cos(yaw_rad) * std::sin(pitch_rad),
        std::cos(pitch_rad),
        -std::sin(yaw_rad) * std::sin(pitch_rad)
    };
    forward = -forward; // Right-handed.
    look_dir = forward;
    Vec3f up = (Vec3f::up - (Vec3f::dot(Vec3f::up, forward) * forward)).get_normalized();

    return Mat4f::create_look_at(pos, forward, up);
}

Vec3f Camera::get_look_dir() const {
    return look_dir;
}
