#pragma once

#include <cmath>

class Vec3f
{
public:
    float x;
    float y;
    float z;

    Vec3f operator-(const Vec3f& b)
    {
        return Vec3f(x - b.x, y - b.y, z - b.z);
    }

    static Vec3f cross(const Vec3f& a, const Vec3f& b)
    {
        return Vec3f(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
    }

    static Vec3f normalize(const Vec3f& a)
    {
        float len = Vec3f::length(a);
        return Vec3f(a.x / len, a.y / len, a.z / len);
    }

    static float length(const Vec3f& a)
    {
        return std::sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
    }

    static float dot(const Vec3f& a, const Vec3f& b)
    {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }
};
