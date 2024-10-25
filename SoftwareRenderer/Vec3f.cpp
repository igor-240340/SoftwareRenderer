#include <cmath>

#include "Vec3f.h"

const Vec3f Vec3f::zero{ 0.0f, 0.0f, 0.0f };

Vec3f::Vec3f(float x, float y, float z) : x(x), y(y), z(z) {
}

Vec3f::Vec3f(Vec4f vec4) : x(vec4.x), y(vec4.y), z(vec4.z) {
}

float Vec3f::length() const {
    return std::sqrt(x * x + y * y + z * z);
}

float Vec3f::length_squared() const {
    return x * x + y * y + z * z;
}

Vec3f Vec3f::normalized() const {
    // Исключаем появление nan в компонентах вектора.
    if (x * x + y * y + z * z > 0)
        return *this / length();
    else
        return Vec3f::zero;
}

Vec3f Vec3f::operator/(float scalar) const {
    const float scalar_inv = 1.0f / scalar;
    return Vec3f(x * scalar_inv, y * scalar_inv, z * scalar_inv);
}

Vec3f Vec3f::operator+(const Vec3f& b) const {
    return Vec3f(x + b.x, y + b.y, z + b.z);
}

Vec3f Vec3f::operator-(const Vec3f& b) const {
    return Vec3f(x - b.x, y - b.y, z - b.z);
}

Vec3f Vec3f::operator*(float scalar) const {
    return Vec3f(x * scalar, y * scalar, z * scalar);
}

Vec3f Vec3f::operator-() const {
    return Vec3f(-x, -y, -z);
}

float Vec3f::dot(const Vec3f& a, const Vec3f& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vec3f Vec3f::cross(const Vec3f& a, const Vec3f& b) {
    return Vec3f(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}
