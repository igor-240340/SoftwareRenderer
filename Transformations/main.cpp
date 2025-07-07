#include <iostream>
#include <numbers>
#include <format>

#include "Mat4f.h"
#include "Vec4f.h"

void test_rotate_then_translate();
void test_translate_then_rotate();

float to_radians(float angle_deg);

int main() {
    test_rotate_then_translate();
    test_translate_then_rotate();

    return 0;
}

// Референсный тестовый пример для сравнения расчётов см. в /docs/transformations/translation/translation.mcdx.
void test_rotate_then_translate() {
    const Vec4f vertex_before{ -2.431883860752f, 1.756684629583f, 0.0f, 1.0f };

    const float beta = to_radians(69.64592296022f);
    const float gamma = to_radians(46.92909951096f);
    const float delta = to_radians(76.48913183105f);
    const Mat4f rot_x = Mat4f::create_rotation_x(gamma);
    const Mat4f rot_y = Mat4f::create_rotation_y(delta);
    const Mat4f rot_z = Mat4f::create_rotation_z(beta);
    const Mat4f translation = Mat4f::create_translation(Vec3f{ 2.2401739117848f, 2.807020326003f, -0.477337272644f });

    const Mat4f transofrmations = translation * rot_y * rot_x * rot_z;
    const Vec3f vertex_after = transofrmations * vertex_before;

    std::cout << std::format("vertex_after: [{}, {}, {}]\n", vertex_after.x, vertex_after.y, vertex_after.z);
}

void test_translate_then_rotate() {
    const Vec4f vertex_before{ -2.431883860752f, 1.756684629583f, 0.0f, 1.0f };

    const float beta = to_radians(69.64592296022f);
    const Mat4f rot_z = Mat4f::create_rotation_z(beta);
    const Mat4f translation = Mat4f::create_translation(Vec3f{ 2.2401739117848f, 2.807020326003f, -0.477337272644f });

    const Mat4f transofrmations = rot_z * translation;
    const Vec3f vertex_after = transofrmations * vertex_before;

    std::cout << std::format("vertex_after: [{}, {}, {}]\n", vertex_after.x, vertex_after.y, vertex_after.z);
}

float to_radians(float angle_deg) {
    return angle_deg * static_cast<float>(std::numbers::pi / 180.0);
}
