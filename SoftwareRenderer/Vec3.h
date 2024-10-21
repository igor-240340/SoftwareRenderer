#pragma once

class Vec3 {
public:
    float x;
    float y;
    float z;

public:
    static const Vec3 zero;

public:
    Vec3() = default;
    ~Vec3() = default;
    Vec3(float x, float y, float z);

    float length() const;
    float length_squared() const;
    Vec3 normalized() const;

    Vec3 operator/(float scalar) const;
    Vec3 operator+(const Vec3& b) const;
    Vec3 operator-(const Vec3& b) const;
    Vec3 operator*(float scalar) const;
    Vec3 operator-() const;

    static float dot(const Vec3& a, const Vec3& b);
    static Vec3 cross(const Vec3& a, const Vec3& b);
};
