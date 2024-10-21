#include <cmath>

#include "Vec3.h"

const Vec3 Vec3::zero{ 0.0f, 0.0f, 0.0f };

Vec3::Vec3(float x, float y, float z) : x(x), y(y), z(z) {
}

float Vec3::length() const {
    return std::sqrt(x * x + y * y + z * z);
}

float Vec3::length_squared() const {
    return x * x + y * y + z * z;
}

Vec3 Vec3::normalized() const {
    // Исключаем появление nan в компонентах вектора.
    if (x * x + y * y + z * z > 0)
        return *this / length();
    else
        return Vec3::zero;
}

Vec3 Vec3::operator/(float scalar) const {
    const float scalar_inv = 1.0f / scalar;
    return Vec3(x * scalar_inv, y * scalar_inv, z * scalar_inv);
}

Vec3 Vec3::operator+(const Vec3& b) const {
    return Vec3(x + b.x, y + b.y, z + b.z);
}

Vec3 Vec3::operator-(const Vec3& b) const {
    return Vec3(x - b.x, y - b.y, z - b.z);
}

Vec3 Vec3::operator*(float scalar) const {
    return Vec3(x * scalar, y * scalar, z * scalar);
}

Vec3 Vec3::operator-() const {
    return Vec3(-x, -y, -z);
}

float Vec3::dot(const Vec3& a, const Vec3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vec3 Vec3::cross(const Vec3& a, const Vec3& b) {
    return Vec3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}
