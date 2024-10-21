#include "Vec4.h"

Vec4::Vec4(const Vec3& vec3, float w) : x(vec3.x), y(vec3.y), z(vec3.z), w(w) {
}

Vec4::Vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {
}

Vec4 Vec4::operator/(float scalar) const {
    const float scalar_inv = 1.0f / scalar;
    return Vec4(x * scalar_inv, y * scalar_inv, z * scalar_inv, w * scalar_inv);
}
