#pragma once

#include "Mat4f.h"

class Camera {
public:
    Camera() = default;
    Camera(Vec3f pos, float yaw_rad, float pitch_rad);
    ~Camera() = default;

    Mat4f get_view_mat();
    Vec3f get_look_dir() const;

private:
    Vec3f pos;
    Vec3f look_dir;

    float yaw_rad;
    float pitch_rad;
};
