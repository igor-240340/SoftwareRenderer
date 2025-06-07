#include <iostream>
#include <ranges>
#include <numbers>
#include <format>
#include <random>
#include <algorithm>

#include <SFML/Graphics.hpp>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include "Vec3f.h"
#include "Vec4f.h"
#include "Mat4f.h"

#include "graphics.h"

constexpr int w = 800;
constexpr int h = 600;

void load_model(std::string model_path, std::vector<Polygon>& polygons);
void rasterize_polygons_wireframe(const std::vector<Polygon>& polygons, FrameBuffer& frame_buffer);
void rasterize_polygons_solid(const std::vector<Polygon>& polygons, FrameBuffer& frame_buffer, ZBuffer& z_buffer);

void debug_z_fighting(FrameBuffer& frame_buffer, ZBuffer& z_buffer);

sf::Color get_random_color() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    //static std::normal_distribution<float> dist(128.0f, 28.0f);
    static std::uniform_int_distribution dist(0, 255);

    /*sf::Uint8 r = static_cast<sf::Uint8>(std::clamp(static_cast<int>(std::round(dist(gen))), 0, 255));
    sf::Uint8 g = static_cast<sf::Uint8>(std::clamp(static_cast<int>(std::round(dist(gen))), 0, 255));
    sf::Uint8 b = static_cast<sf::Uint8>(std::clamp(static_cast<int>(std::round(dist(gen))), 0, 255));*/

    sf::Uint8 r = static_cast<sf::Uint8>(dist(gen));
    sf::Uint8 g = static_cast<sf::Uint8>(dist(gen));
    sf::Uint8 b = static_cast<sf::Uint8>(dist(gen));
    return sf::Color(r, g, b);
}

int main() {
    sf::RenderWindow window(sf::VideoMode(w, h), "Software Renderer");

    sf::Texture texture;
    if (!texture.create(w, h))
        std::cout << "sfml: texture.create() failed\n";
    sf::Sprite sprite(texture);

    FrameBuffer frame_buffer{ w, h, std::vector<sf::Uint8>(w * h * 4) };
    ZBuffer z_buffer{ w, h, std::vector<float>(w * h) };

    std::vector<Polygon> polygons;
    //const std::string model_path = "data/triangles/triangles.obj";
    //const std::string model_path = "data/triangles_clipping/triangles_clipping.obj";
    //const std::string model_path = "data/viking_room/viking_room.obj";
    //const std::string model_path = "data/blender_monkey/blender_monkey.obj";
    const std::string model_path = "data/box/box.obj";
    //const std::string model_path = "data/torus/torus.obj";
    //const std::string model_path = "data/uv_sphere/uv_sphere.obj";
    load_model(model_path, polygons);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        clear_frame_buffer(sf::Color::Black, frame_buffer);
        clear_z_buffer(1.0f, z_buffer);

        //rasterize_polygons_wireframe(polygons, frame_buffer);
        rasterize_polygons_solid(polygons, frame_buffer, z_buffer);
        //debug_z_fighting(frame_buffer, z_buffer);

        texture.update(frame_buffer.rgba_array.data());

        window.clear();
        window.draw(sprite);
        window.display();
    }

    return 0;
}

void load_model(std::string model_path, std::vector<Polygon>& polygons) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, model_path.c_str()))
        std::cout << "tinyobjloader: " << err << '\n';

    for (const auto& shape : shapes) {
        for (size_t i = 0; i <= shape.mesh.indices.size() - 3; i += 3) {
            const auto& index0 = shape.mesh.indices[i + 0];
            const auto& index1 = shape.mesh.indices[i + 1];
            const auto& index2 = shape.mesh.indices[i + 2];

            const Vertex v0{ Vec3f{
                attrib.vertices[3 * index0.vertex_index + 0],
                attrib.vertices[3 * index0.vertex_index + 1],
                attrib.vertices[3 * index0.vertex_index + 2]
            } };

            const Vertex v1{ Vec3f{
                attrib.vertices[3 * index1.vertex_index + 0],
                attrib.vertices[3 * index1.vertex_index + 1],
                attrib.vertices[3 * index1.vertex_index + 2]
            } };

            const Vertex v2{ Vec3f{
                attrib.vertices[3 * index2.vertex_index + 0],
                attrib.vertices[3 * index2.vertex_index + 1],
                attrib.vertices[3 * index2.vertex_index + 2]
            } };

            polygons.push_back(Polygon{ { v0, v1, v2 }, get_random_color() });
        }
    }
}

void rasterize_polygons_wireframe(const std::vector<Polygon>& polygons, FrameBuffer& frame_buffer) {
    const float fov_vert_rad = static_cast<float>(45.0 * (std::numbers::pi / 180.0));
    const float aspect_ratio = static_cast<float>(frame_buffer.w) / frame_buffer.h;
    const Mat4f proj = Mat4f::create_perspective(fov_vert_rad, aspect_ratio, 0.1f, 10.0f);
    const Mat4f viewport = Mat4f::create_viewport(frame_buffer.w, frame_buffer.h);

    for (const auto& polygon : polygons) {
        std::vector<Vertex> vertices_screen;
        for (const auto& vertex : polygon.vertices) {
            Vec4f pos{ vertex.pos };

            Vec4f pos_clip = proj * pos;
            Vec4f pos_ndc = pos_clip / pos_clip.w;
            Vec4f pos_screen = viewport * pos_ndc;

            vertices_screen.push_back(Vertex{ Vec3f{pos_screen} });
        }
        Polygon polygon_screen{ {vertices_screen[0], vertices_screen[1], vertices_screen[2]} };
        draw_polygon_wireframe(polygon_screen, frame_buffer);
    }
}

void rasterize_polygons_solid(const std::vector<Polygon>& polygons, FrameBuffer& frame_buffer, ZBuffer& z_buffer) {
    const float fov_vert_rad = static_cast<float>(45.0 * (std::numbers::pi / 180.0));
    const float aspect_ratio = static_cast<float>(frame_buffer.w) / frame_buffer.h;
    const Mat4f proj = Mat4f::create_perspective(fov_vert_rad, aspect_ratio, 0.1f, 10.0f);
    const Mat4f viewport = Mat4f::create_viewport(frame_buffer.w, frame_buffer.h);

    for (const auto& polygon : polygons) {
        std::vector<Vertex> vertices_screen;
        for (const auto& vertex : polygon.vertices) {
            Vec4f pos{ vertex.pos };

            Vec4f pos_clip = proj * pos;
            Vec4f pos_ndc = pos_clip / pos_clip.w;
            Vec4f pos_screen = viewport * pos_ndc;

            vertices_screen.push_back(Vertex{ Vec3f{pos_screen} });
        }
        Polygon polygon_screen{ {vertices_screen[0], vertices_screen[1], vertices_screen[2]}, polygon.color };
        draw_polygon_solid(polygon_screen, frame_buffer, z_buffer);
    }
}

// NOTE: Отладка z-файтинга.
// Артефакт возникает на стыке 7-го и 9-го полигонов во фрагменте с координатами (384, 488).
// Фрагмент растеризуется для обоих полигонов, но, видимо, из-за разной геометрии, интерполяция z-атрибута
// в пределах 9-го полигона (нижний) даёт значение ближе к камере, в результате чего несколько
// пикселов нижнего полигона "торчат" из-под вертикального полигона в месте стыка.
// 
// Возможные решения:
// - Depth Bias.
// - Back-face culling.
void debug_z_fighting(FrameBuffer& frame_buffer, ZBuffer& z_buffer) {
    std::vector<Polygon> polygons;
    const std::string model_path = "data/box/box.obj";
    load_model(model_path, polygons);

    const float fov_vert_rad = static_cast<float>(45.0 * (std::numbers::pi / 180.0));
    const float aspect_ratio = static_cast<float>(frame_buffer.w) / frame_buffer.h;
    const Mat4f proj = Mat4f::create_perspective(fov_vert_rad, aspect_ratio, 0.1f, 10.0f);
    const Mat4f viewport = Mat4f::create_viewport(frame_buffer.w, frame_buffer.h);

    std::vector<Polygon> polygons_screen;
    for (const auto& polygon : polygons) {
        std::vector<Vertex> vertices_screen;
        for (const auto& vertex : polygon.vertices) {
            Vec4f pos{ vertex.pos };

            Vec4f pos_clip = proj * pos;
            Vec4f pos_ndc = pos_clip / pos_clip.w;
            Vec4f pos_screen = viewport * pos_ndc;

            vertices_screen.push_back(Vertex{ Vec3f{pos_screen} });
        }
        Polygon polygon_screen{ {vertices_screen[0], vertices_screen[1], vertices_screen[2]}, polygon.color };
        polygons_screen.push_back(polygon_screen);
    }

    //draw_polygon_solid(polygons_screen[0], frame_buffer, z_buffer);
    //draw_polygon_solid(polygons_screen[1], frame_buffer, z_buffer);
    //draw_polygon_solid(polygons_screen[2], frame_buffer, z_buffer);
    //draw_polygon_solid(polygons_screen[3], frame_buffer, z_buffer);
    //draw_polygon_solid(polygons_screen[4], frame_buffer, z_buffer);
    //draw_polygon_solid(polygons_screen[5], frame_buffer, z_buffer);
    //draw_polygon_solid(polygons_screen[6], frame_buffer, z_buffer);
    //draw_polygon_solid(polygons_screen[7], frame_buffer, z_buffer);
    //draw_polygon_solid(polygons_screen[8], frame_buffer, z_buffer);
    //draw_polygon_solid(polygons_screen[9], frame_buffer, z_buffer);
    //draw_polygon_solid(polygons_screen[10], frame_buffer, z_buffer);
    //draw_polygon_solid(polygons_screen[11], frame_buffer, z_buffer);

    polygons_screen[7].color = sf::Color::Blue; // Стоит.
    polygons_screen[9].color = sf::Color::Red; // Лежит.

    // Z-Bias.
    polygons_screen[9].vertices[0].pos.z += 1e-6;
    polygons_screen[9].vertices[1].pos.z += 1e-6;
    polygons_screen[9].vertices[2].pos.z += 1e-6;

    draw_polygon_solid(polygons_screen[7], frame_buffer, z_buffer);
    draw_polygon_solid(polygons_screen[9], frame_buffer, z_buffer);

    const int x = 384;
    const int y = 488;
    const sf::Color color = read_frame_buffer(x, y, frame_buffer);
    const float depth = read_z_buffer(x, y, z_buffer);
}
