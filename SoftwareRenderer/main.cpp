#include <iostream>
#include <ranges>
#include <numbers>
#include <format>
#include <random>
#include <algorithm>
#include <chrono>

#include <SFML/Graphics.hpp>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include "Vec3f.h"
#include "Vec4f.h"
#include "Mat4f.h"

#include "graphics.h"

void load_model(std::string model_path, std::vector<Polygon>& polygons);

void rasterize_polygons_wireframe(const std::vector<Polygon>& polygons, Framebuffer& framebuffer);
void rasterize_polygons_solid(const std::vector<Polygon>& polygons, Framebuffer& framebuffer, ZBuffer& z_buffer);
void rasterize_polygons_flat_shaded(const std::vector<Polygon>& polygons, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer);

void rasterize_polygons_flat_shaded_and_rotate(const std::vector<Polygon>& polygons, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer, float angle_rad);
void rasterize_polygons_flat_shaded_textured_affine(const std::vector<Polygon>& polygons, const sf::Image& texture_image, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer, const Mat4f& transformations);

void rasterize_polygons_wireframe_ortho(const std::vector<Polygon>& polygons, Framebuffer& framebuffer);
void rasterize_polygons_solid_ortho(const std::vector<Polygon>& polygons, Framebuffer& framebuffer, ZBuffer& z_buffer);
void rasterize_polygons_flat_shaded_ortho(const std::vector<Polygon>& polygons, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer);

void debug_z_fighting(Framebuffer& frame_buffer, ZBuffer& z_buffer);

void demonstrate_gimbal_lock(const std::vector<Polygon>& polygons, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer);

sf::Color get_random_color();

using high_res_clock = std::chrono::high_resolution_clock;

int main() {
    constexpr int w = 800;
    constexpr int h = 600;

    sf::Font font;
    if (!font.loadFromFile("data/fira_mono.ttf")) {
        std::cout << "sfml: font.loadFromFile() failed\n";
        return 1;
    }

    sf::Text fps_text;
    fps_text.setString("fps: 0");
    fps_text.setFont(font);
    fps_text.setCharacterSize(16);
    fps_text.setFillColor(sf::Color::White);
    fps_text.setPosition(5.0f, 5.0f);

    sf::RenderWindow window(sf::VideoMode(w, h), "Software Renderer");
    window.setFramerateLimit(0);
    window.setVerticalSyncEnabled(true);

    sf::Texture texture;
    if (!texture.create(w, h)) {
        std::cout << "sfml: texture.create() failed\n";
        return 1;
    }
    sf::Sprite sprite(texture);

    Light light{ Vec3f{0.0f, 0.0f, -1.0f} };
    Framebuffer frame_buffer{ w, h, std::vector<sf::Uint8>(w * h * 4) };
    ZBuffer z_buffer{ w, h, std::vector<float>(w * h) };

    std::vector<Polygon> polygons;
    // NOTE: Отладочные треугольники рисовать без трансформаций.
    //const std::string model_path = "data/triangles/triangles.obj";
    //const std::string model_path = "data/triangles_clipping/triangles_clipping.obj";

    //const std::string model_path = "data/box/box.obj";
    //const std::string model_path = "data/torus/torus.obj";
    //const std::string model_path = "data/uv_sphere/uv_sphere.obj";
    //const std::string model_path = "data/skull/skull.obj";
    //const std::string model_path = "data/blender_monkey/blender_monkey.obj";
    //const std::string model_path = "data/pig/pig.obj";
    //const std::string model_path = "data/plane/plane.obj";
    const std::string model_path = "data/new_balance/nb574.obj";
    load_model(model_path, polygons);

    // Читаем текстуру модели.
    sf::Texture model_texture;
    if (!model_texture.loadFromFile("data/new_balance/nb574.jpeg")) {
        std::cout << "sfml: model_texture.loadFromFile() failed\n";
        return 1;
    }
    sf::Image model_texture_image = model_texture.copyToImage();

    auto measure_start = high_res_clock::now();
    int frame_count = 0;

    const float angle_rad_step = static_cast<float>(std::numbers::pi / 180.0);
    const float two_pi = static_cast<float>(std::numbers::pi * 2.0);
    float angle_rad_accum = 0.0f;
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        clear_framebuffer(sf::Color::White, frame_buffer);
        clear_z_buffer(1.0f, z_buffer);

        //debug_z_fighting(frame_buffer, z_buffer);

        //rasterize_polygons_wireframe(polygons, frame_buffer);
        //rasterize_polygons_solid(polygons, frame_buffer, z_buffer);
        //rasterize_polygons_flat_shaded(polygons, light, frame_buffer, z_buffer);

        //rasterize_polygons_flat_shaded_and_rotate(polygons, light, frame_buffer, z_buffer, angle_rad_accum);

        Mat4f translation = Mat4f::create_translation(Vec3f{ 0.0f, 0.0f, -5.0f });
        Mat4f rotation_x = Mat4f::create_rotation_x(-90.0f * static_cast<float>(std::numbers::pi / 180.0));
        Mat4f rotation_y = Mat4f::create_rotation_y(angle_rad_accum);
		Mat4f scale_xy = Mat4f::create_scale_x(0.4f) * Mat4f::create_scale_y(0.4f) * Mat4f::create_scale_z(0.4f);
        Mat4f transformations = translation * rotation_y * rotation_x * scale_xy;
        rasterize_polygons_flat_shaded_textured_affine(polygons, model_texture_image, light, frame_buffer, z_buffer, transformations);

        //rasterize_polygons_wireframe_ortho(polygons, frame_buffer);
        //rasterize_polygons_solid_ortho(polygons, frame_buffer, z_buffer);
        //rasterize_polygons_flat_shaded_ortho(polygons, light, frame_buffer, z_buffer);

        //demonstrate_gimbal_lock(polygons, light, frame_buffer, z_buffer);

        texture.update(frame_buffer.rgba_array.data());

        window.clear();
        window.draw(sprite);
        window.draw(fps_text);
        window.display();

        std::chrono::duration<float> elapsed_seconds = high_res_clock::now() - measure_start;
        ++frame_count;
        if (elapsed_seconds.count() >= 1.0f) {
            fps_text.setString(std::format("fps: {}", static_cast<int>(frame_count / elapsed_seconds.count())));
            frame_count = 0;
            measure_start = high_res_clock::now();
        }

        // TODO: Отвязать от фреймрейта.
        angle_rad_accum += angle_rad_step;
        if (angle_rad_accum > two_pi)
            angle_rad_accum = std::fmod(angle_rad_accum, two_pi);
    }

    return 0;
}

void load_model(std::string model_path, std::vector<Polygon>& polygons) {
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, model_path.c_str()))
		std::cout << "tinyobjloader: " << err << '\n';

	for (const auto& shape : shapes) {
		for (size_t i = 0; i <= shape.mesh.indices.size() - 3; i += 3) {
			auto& index0 = shape.mesh.indices[i + 0];
			auto& index1 = shape.mesh.indices[i + 1];
			auto& index2 = shape.mesh.indices[i + 2];

			Vertex v0{ Vec3f{
				attrib.vertices[3 * index0.vertex_index + 0],
				attrib.vertices[3 * index0.vertex_index + 1],
				attrib.vertices[3 * index0.vertex_index + 2]
			}, TexCoord{
				attrib.texcoords[2 * index0.texcoord_index + 0],
				attrib.texcoords[2 * index0.texcoord_index + 1]
			} };

			Vertex v1{ Vec3f{
				attrib.vertices[3 * index1.vertex_index + 0],
				attrib.vertices[3 * index1.vertex_index + 1],
				attrib.vertices[3 * index1.vertex_index + 2]
			}, TexCoord{
				attrib.texcoords[2 * index1.texcoord_index + 0],
				attrib.texcoords[2 * index1.texcoord_index + 1]
			} };

			Vertex v2{ Vec3f{
				attrib.vertices[3 * index2.vertex_index + 0],
				attrib.vertices[3 * index2.vertex_index + 1],
				attrib.vertices[3 * index2.vertex_index + 2]
			}, TexCoord{
				attrib.texcoords[2 * index2.texcoord_index + 0],
				attrib.texcoords[2 * index2.texcoord_index + 1]
			} };

			polygons.push_back(Polygon{ { v0, v1, v2 }, sf::Color::White });
		}
	}
}

void rasterize_polygons_wireframe(const std::vector<Polygon>& polygons, Framebuffer& framebuffer) {
    const Mat4f translation = Mat4f::create_translation(Vec3f{ 0.0f, 0.0f, -4.0f });
    const Mat4f rotation_x = Mat4f::create_rotation_x(23.0f * static_cast<float>(std::numbers::pi / 180.0));
    const Mat4f rotation_y = Mat4f::create_rotation_y(23.0f * static_cast<float>(std::numbers::pi / 180.0));

    const float fov_vert_rad = static_cast<float>(45.0 * (std::numbers::pi / 180.0));
    const float aspect_ratio = static_cast<float>(framebuffer.w) / framebuffer.h;
    const Mat4f proj = Mat4f::create_perspective(fov_vert_rad, aspect_ratio, 0.1f, 10.0f);
    const Mat4f viewport = Mat4f::create_viewport(framebuffer.w, framebuffer.h);

    for (Polygon polygon : polygons) {
        for (Vertex& vertex : polygon.vertices) {
            const Vec4f pos{ vertex.pos };
            vertex.pos = translation * rotation_y * rotation_x * pos;
        }

        std::vector<Vertex> vertices_screen;
        for (const auto& vertex : polygon.vertices) {
            Vec4f pos{ vertex.pos };

            Vec4f pos_clip = proj * pos;
            Vec4f pos_ndc = pos_clip / pos_clip.w;
            Vec4f pos_screen = viewport * pos_ndc;

            vertices_screen.push_back(Vertex{ Vec3f{pos_screen} });
        }
        Polygon polygon_screen{ {vertices_screen[0], vertices_screen[1], vertices_screen[2]} };
        draw_polygon_wireframe(polygon_screen, framebuffer);
    }
}

void rasterize_polygons_solid(const std::vector<Polygon>& polygons, Framebuffer& framebuffer, ZBuffer& z_buffer) {
    const Mat4f translation = Mat4f::create_translation(Vec3f{ 0.0f, 0.0f, -4.0f });
    const Mat4f rotation_x = Mat4f::create_rotation_x(23.0f * static_cast<float>(std::numbers::pi / 180.0));
    const Mat4f rotation_y = Mat4f::create_rotation_y(23.0f * static_cast<float>(std::numbers::pi / 180.0));

    const float fov_vert_rad = static_cast<float>(45.0 * (std::numbers::pi / 180.0));
    const float aspect_ratio = static_cast<float>(framebuffer.w) / framebuffer.h;
    const Mat4f proj = Mat4f::create_perspective(fov_vert_rad, aspect_ratio, 0.1f, 10.0f);
    const Mat4f viewport = Mat4f::create_viewport(framebuffer.w, framebuffer.h);

    for (Polygon polygon : polygons) {
        for (Vertex& vertex : polygon.vertices) {
            const Vec4f pos{ vertex.pos };
            vertex.pos = translation * rotation_y * rotation_x * pos;
        }

        std::vector<Vertex> vertices_screen;
        for (const auto& vertex : polygon.vertices) {
            Vec4f pos{ vertex.pos };

            Vec4f pos_clip = proj * pos;
            Vec4f pos_ndc = pos_clip / pos_clip.w;
            Vec4f pos_screen = viewport * pos_ndc;

            vertices_screen.push_back(Vertex{ Vec3f{pos_screen} });
        }
        Polygon polygon_screen{ {vertices_screen[0], vertices_screen[1], vertices_screen[2]}, polygon.albedo_color };
        draw_polygon_solid(polygon_screen, framebuffer, z_buffer);
    }
}

void rasterize_polygons_flat_shaded(const std::vector<Polygon>& polygons, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer) {
    const Mat4f translation = Mat4f::create_translation(Vec3f{ 0.0f, 0.0f, -4.0f });
    const Mat4f rotation_x = Mat4f::create_rotation_x(23.0f * static_cast<float>(std::numbers::pi / 180.0));
    const Mat4f rotation_y = Mat4f::create_rotation_y(23.0f * static_cast<float>(std::numbers::pi / 180.0));

    const float fov_vert_rad = static_cast<float>(45.0 * (std::numbers::pi / 180.0));
    const float aspect_ratio = static_cast<float>(framebuffer.w) / framebuffer.h;
    const Mat4f proj = Mat4f::create_perspective(fov_vert_rad, aspect_ratio, 0.1f, 10.0f);
    const Mat4f viewport = Mat4f::create_viewport(framebuffer.w, framebuffer.h);

    for (Polygon polygon : polygons) {
        for (Vertex& vertex : polygon.vertices) {
            const Vec4f pos{ vertex.pos };
            vertex.pos = translation * rotation_y * rotation_x * pos;
        }

        // Вычисляем нормаль полигона.
        const Vertex& v0 = polygon.vertices[0];
        const Vertex& v1 = polygon.vertices[1];
        const Vertex& v2 = polygon.vertices[2];

        const Vec3f edge1 = v1.pos - v0.pos;
        const Vec3f edge2 = v2.pos - v0.pos;
        const Vec3f polygon_normal = Vec3f::cross(edge1, edge2).get_normalized();

        std::vector<Vertex> vertices_screen;
        for (const Vertex& vertex : polygon.vertices) {
            const Vec4f pos{ vertex.pos };

            Vec4f pos_clip = proj * pos;
            Vec4f pos_ndc = pos_clip / pos_clip.w;
            Vec4f pos_screen = viewport * pos_ndc;

            vertices_screen.push_back(Vertex{ Vec3f{pos_screen} });
        }

        Polygon polygon_screen{
            {vertices_screen[0], vertices_screen[1], vertices_screen[2]},
            polygon.albedo_color,
            polygon_normal
        };
        draw_polygon_flat_shaded(polygon_screen, light, framebuffer, z_buffer);
    }
}

void rasterize_polygons_flat_shaded_and_rotate(const std::vector<Polygon>& polygons, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer, float angle_rad) {
    const Mat4f translation = Mat4f::create_translation(Vec3f{ 0.0f, 0.0f, -4.0f });
    //const Mat4f rotation_x = Mat4f::create_rotation_x(angle_rad);
    const Mat4f rotation_y = Mat4f::create_rotation_y(angle_rad);

    const float fov_vert_rad = static_cast<float>(45.0 * (std::numbers::pi / 180.0));
    const float aspect_ratio = static_cast<float>(framebuffer.w) / framebuffer.h;
    const Mat4f proj = Mat4f::create_perspective(fov_vert_rad, aspect_ratio, 0.1f, 10.0f);
    const Mat4f viewport = Mat4f::create_viewport(framebuffer.w, framebuffer.h);

    for (Polygon polygon : polygons) {
        for (Vertex& vertex : polygon.vertices) {
            const Vec4f pos{ vertex.pos };
            vertex.pos = translation * rotation_y * /*rotation_x **/ pos;
        }

        // Вычисляем нормаль полигона.
        const Vertex& v0 = polygon.vertices[0];
        const Vertex& v1 = polygon.vertices[1];
        const Vertex& v2 = polygon.vertices[2];

        const Vec3f edge1 = v1.pos - v0.pos;
        const Vec3f edge2 = v2.pos - v0.pos;
        const Vec3f polygon_normal = Vec3f::cross(edge1, edge2).get_normalized();

        std::vector<Vertex> vertices_screen;
        for (const Vertex& vertex : polygon.vertices) {
            const Vec4f pos{ vertex.pos };

            Vec4f pos_clip = proj * pos;
            Vec4f pos_ndc = pos_clip / pos_clip.w;
            Vec4f pos_screen = viewport * pos_ndc;

            vertices_screen.push_back(Vertex{ Vec3f{pos_screen} });
        }

        Polygon polygon_screen{
            {vertices_screen[0], vertices_screen[1], vertices_screen[2]},
            polygon.albedo_color,
            polygon_normal
        };
        draw_polygon_flat_shaded(polygon_screen, light, framebuffer, z_buffer);
    }
}

void rasterize_polygons_flat_shaded_textured_affine(const std::vector<Polygon>& polygons, const sf::Image& texture_image, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer, const Mat4f& transformations) {
    float fov_vert_rad = static_cast<float>(45.0 * (std::numbers::pi / 180.0));
    float aspect_ratio = static_cast<float>(framebuffer.w) / framebuffer.h;
    Mat4f proj = Mat4f::create_perspective(fov_vert_rad, aspect_ratio, 0.1f, 10.0f);
    Mat4f viewport = Mat4f::create_viewport(framebuffer.w, framebuffer.h);

    for (Polygon polygon : polygons) {
        for (Vertex& vertex : polygon.vertices) {
            Vec4f pos{ vertex.pos };
            //vertex.pos = translation * rotation_y * rotation_x * scale_xy * pos;
            vertex.pos = transformations * pos;
        }

        // Вычисляем нормаль полигона.
        const Vertex& v0 = polygon.vertices[0];
        const Vertex& v1 = polygon.vertices[1];
        const Vertex& v2 = polygon.vertices[2];

        const Vec3f edge1 = v1.pos - v0.pos;
        const Vec3f edge2 = v2.pos - v0.pos;
        const Vec3f polygon_normal = Vec3f::cross(edge1, edge2).get_normalized();

        std::vector<Vertex> vertices_screen;
        for (const auto& vertex : polygon.vertices) {
            Vec4f pos{ vertex.pos };

            Vec4f pos_clip = proj * pos;
            Vec4f pos_ndc = pos_clip / pos_clip.w;
            Vec4f pos_screen = viewport * pos_ndc;

            vertices_screen.push_back(Vertex{ Vec3f{pos_screen}, vertex.tex_coord });
        }
        Polygon polygon_screen{ {vertices_screen[0], vertices_screen[1], vertices_screen[2]}, polygon.albedo_color, polygon_normal };
        draw_polygon_flat_shaded_textured_affine(polygon_screen, texture_image, light, framebuffer, z_buffer);
    }
}

void rasterize_polygons_wireframe_ortho(const std::vector<Polygon>& polygons, Framebuffer& framebuffer) {
    const Mat4f translation = Mat4f::create_translation(Vec3f{ 0.0f, 0.0f, -4.0f });
    const Mat4f rotation_x = Mat4f::create_rotation_x(23.0f * static_cast<float>(std::numbers::pi / 180.0));
    const Mat4f rotation_y = Mat4f::create_rotation_y(23.0f * static_cast<float>(std::numbers::pi / 180.0));

    const float ratio = static_cast<float>(framebuffer.w) / framebuffer.h;
    const float proj_plane_w = 4.0f; // [-2, 2].
    const float proj_plane_h = proj_plane_w / ratio; // Требуем, чтобы proj_plane_w / proj_plane_h = w / h.
    const Mat4f proj = Mat4f::create_ortho(-proj_plane_w / 2, proj_plane_w / 2, -proj_plane_h / 2, proj_plane_h / 2, 0.1f, 10.0f);
    const Mat4f viewport = Mat4f::create_viewport(framebuffer.w, framebuffer.h);

    for (Polygon polygon : polygons) {
        for (Vertex& vertex : polygon.vertices) {
            const Vec4f pos{ vertex.pos };
            vertex.pos = translation * rotation_y * rotation_x * pos;
        }

        std::vector<Vertex> vertices_screen;
        for (const auto& vertex : polygon.vertices) {
            Vec4f pos{ vertex.pos };

            Vec4f pos_clip = proj * pos;
            Vec4f pos_ndc = pos_clip / pos_clip.w; // NOTE: В случае ортографической проекции нет необходимости в перспективном делении.
            Vec4f pos_screen = viewport * pos_ndc;

            vertices_screen.push_back(Vertex{ Vec3f{pos_screen} });
        }
        Polygon polygon_screen{ {vertices_screen[0], vertices_screen[1], vertices_screen[2]} };
        draw_polygon_wireframe(polygon_screen, framebuffer);
    }
}

void rasterize_polygons_solid_ortho(const std::vector<Polygon>& polygons, Framebuffer& framebuffer, ZBuffer& z_buffer) {
    const Mat4f translation = Mat4f::create_translation(Vec3f{ 0.0f, 0.0f, -4.0f });
    const Mat4f rotation_x = Mat4f::create_rotation_x(23.0f * static_cast<float>(std::numbers::pi / 180.0));
    const Mat4f rotation_y = Mat4f::create_rotation_y(23.0f * static_cast<float>(std::numbers::pi / 180.0));

    const float ratio = static_cast<float>(framebuffer.w) / framebuffer.h;
    const float proj_plane_w = 4.0f;
    const float proj_plane_h = proj_plane_w / ratio;
    const Mat4f proj = Mat4f::create_ortho(-proj_plane_w / 2, proj_plane_w / 2, -proj_plane_h / 2, proj_plane_h / 2, 0.1f, 10.0f);
    const Mat4f viewport = Mat4f::create_viewport(framebuffer.w, framebuffer.h);

    for (Polygon polygon : polygons) {
        for (Vertex& vertex : polygon.vertices) {
            const Vec4f pos{ vertex.pos };
            vertex.pos = translation * rotation_y * rotation_x * pos;
        }

        std::vector<Vertex> vertices_screen;
        for (const auto& vertex : polygon.vertices) {
            Vec4f pos{ vertex.pos };

            Vec4f pos_clip = proj * pos;
            Vec4f pos_ndc = pos_clip / pos_clip.w;
            Vec4f pos_screen = viewport * pos_ndc;

            vertices_screen.push_back(Vertex{ Vec3f{pos_screen} });
        }
        Polygon polygon_screen{ {vertices_screen[0], vertices_screen[1], vertices_screen[2]}, polygon.albedo_color };
        draw_polygon_solid(polygon_screen, framebuffer, z_buffer);
    }
}

void rasterize_polygons_flat_shaded_ortho(const std::vector<Polygon>& polygons, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer) {
    const Mat4f translation = Mat4f::create_translation(Vec3f{ 0.0f, 0.0f, -4.0f });
    const Mat4f rotation_x = Mat4f::create_rotation_x(23.0f * static_cast<float>(std::numbers::pi / 180.0));
    const Mat4f rotation_y = Mat4f::create_rotation_y(23.0f * static_cast<float>(std::numbers::pi / 180.0));

    const float ratio = static_cast<float>(framebuffer.w) / framebuffer.h;
    const float proj_plane_w = 4.0f;
    const float proj_plane_h = proj_plane_w / ratio;
    const Mat4f proj = Mat4f::create_ortho(-proj_plane_w / 2, proj_plane_w / 2, -proj_plane_h / 2, proj_plane_h / 2, 0.1f, 10.0f);
    const Mat4f viewport = Mat4f::create_viewport(framebuffer.w, framebuffer.h);

    for (Polygon polygon : polygons) {
        for (Vertex& vertex : polygon.vertices) {
            const Vec4f pos{ vertex.pos };
            vertex.pos = translation * rotation_y * rotation_x * pos;
        }

        // Вычисляем нормаль полигона.
        const Vertex& v0 = polygon.vertices[0];
        const Vertex& v1 = polygon.vertices[1];
        const Vertex& v2 = polygon.vertices[2];

        const Vec3f edge1 = v1.pos - v0.pos;
        const Vec3f edge2 = v2.pos - v0.pos;
        const Vec3f polygon_normal = Vec3f::cross(edge1, edge2).get_normalized();

        std::vector<Vertex> vertices_screen;
        for (const Vertex& vertex : polygon.vertices) {
            Vec4f pos{ vertex.pos };

            Vec4f pos_clip = proj * pos;
            Vec4f pos_ndc = pos_clip / pos_clip.w;
            Vec4f pos_screen = viewport * pos_ndc;

            vertices_screen.push_back(Vertex{ Vec3f{pos_screen} });
        }

        Polygon polygon_screen{
            {vertices_screen[0], vertices_screen[1], vertices_screen[2]},
            polygon.albedo_color,
            polygon_normal
        };
        draw_polygon_flat_shaded(polygon_screen, light, framebuffer, z_buffer);
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
void debug_z_fighting(Framebuffer& framebuffer, ZBuffer& z_buffer) {
    std::vector<Polygon> polygons;
    const std::string model_path = "data/box/box.obj";
    load_model(model_path, polygons);

    const float fov_vert_rad = static_cast<float>(45.0 * (std::numbers::pi / 180.0));
    const float aspect_ratio = static_cast<float>(framebuffer.w) / framebuffer.h;
    const Mat4f proj = Mat4f::create_perspective(fov_vert_rad, aspect_ratio, 0.1f, 10.0f);
    const Mat4f viewport = Mat4f::create_viewport(framebuffer.w, framebuffer.h);

    std::vector<Polygon> polygons_screen;
    for (const Polygon& polygon : polygons) {
        std::vector<Vertex> vertices_screen;
        for (const Vertex& vertex : polygon.vertices) {
            Vec4f pos{ vertex.pos };

            Vec4f pos_clip = proj * pos;
            Vec4f pos_ndc = pos_clip / pos_clip.w;
            Vec4f pos_screen = viewport * pos_ndc;

            vertices_screen.push_back(Vertex{ Vec3f{pos_screen} });
        }
        Polygon polygon_screen{ {vertices_screen[0], vertices_screen[1], vertices_screen[2]}, polygon.albedo_color };
        polygons_screen.push_back(polygon_screen);
    }

    /*draw_polygon_solid(polygons_screen[0], framebuffer, z_buffer);
    draw_polygon_solid(polygons_screen[1], framebuffer, z_buffer);
    draw_polygon_solid(polygons_screen[2], framebuffer, z_buffer);
    draw_polygon_solid(polygons_screen[3], framebuffer, z_buffer);
    draw_polygon_solid(polygons_screen[4], framebuffer, z_buffer);
    draw_polygon_solid(polygons_screen[5], framebuffer, z_buffer);
    draw_polygon_solid(polygons_screen[6], framebuffer, z_buffer);
    draw_polygon_solid(polygons_screen[7], framebuffer, z_buffer);
    draw_polygon_solid(polygons_screen[8], framebuffer, z_buffer);
    draw_polygon_solid(polygons_screen[9], framebuffer, z_buffer);
    draw_polygon_solid(polygons_screen[10], framebuffer, z_buffer);
    draw_polygon_solid(polygons_screen[11], framebuffer, z_buffer);*/

    polygons_screen[7].albedo_color = sf::Color::Blue; // Стоит.
    polygons_screen[9].albedo_color = sf::Color::Red; // Лежит.

    // Z-Bias.
    polygons_screen[9].vertices[0].pos.z += 1e-6f;
    polygons_screen[9].vertices[1].pos.z += 1e-6f;
    polygons_screen[9].vertices[2].pos.z += 1e-6f;

    draw_polygon_solid(polygons_screen[7], framebuffer, z_buffer);
    draw_polygon_solid(polygons_screen[9], framebuffer, z_buffer);

    const int x = 384;
    const int y = 488;
    const sf::Color color = read_framebuffer(x, y, framebuffer);
    const float depth = read_z_buffer(x, y, z_buffer);
}

void demonstrate_gimbal_lock(const std::vector<Polygon>& polygons, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer) {
    const Mat4f translation = Mat4f::create_translation(Vec3f{ 0.0f, 0.0f, -4.0f });

    const Mat4f rotation_x = Mat4f::create_rotation_x(90.0f * static_cast<float>(std::numbers::pi / 180.0));
    const Mat4f rotation_y = Mat4f::create_rotation_y(45.0f * static_cast<float>(std::numbers::pi / 180.0));
    const Mat4f rotation_z = Mat4f::create_rotation_z(45.0f * static_cast<float>(std::numbers::pi / 180.0));

    const float fov_vert_rad = static_cast<float>(45.0 * (std::numbers::pi / 180.0));
    const float aspect_ratio = static_cast<float>(framebuffer.w) / framebuffer.h;
    const Mat4f proj = Mat4f::create_perspective(fov_vert_rad, aspect_ratio, 0.1f, 10.0f);
    const Mat4f viewport = Mat4f::create_viewport(framebuffer.w, framebuffer.h);

    for (Polygon polygon : polygons) {
        for (Vertex& vertex : polygon.vertices) {
            const Vec4f pos{ vertex.pos };
            // Поворот по Y "превратился" в поворот по Z - потеряли степень свободы.
            vertex.pos = translation * rotation_z * rotation_x * rotation_y * pos;
            //vertex.pos = translation * rotation_z * rotation_y * rotation_x * pos;
        }

        // Вычисляем нормаль полигона.
        const Vertex& v0 = polygon.vertices[0];
        const Vertex& v1 = polygon.vertices[1];
        const Vertex& v2 = polygon.vertices[2];

        const Vec3f edge1 = v1.pos - v0.pos;
        const Vec3f edge2 = v2.pos - v0.pos;
        const Vec3f polygon_normal = Vec3f::cross(edge1, edge2).get_normalized();

        std::vector<Vertex> vertices_screen;
        for (const Vertex& vertex : polygon.vertices) {
            const Vec4f pos{ vertex.pos };

            Vec4f pos_clip = proj * pos;
            Vec4f pos_ndc = pos_clip / pos_clip.w;
            Vec4f pos_screen = viewport * pos_ndc;

            vertices_screen.push_back(Vertex{ Vec3f{pos_screen} });
        }

        Polygon polygon_screen{
            {vertices_screen[0], vertices_screen[1], vertices_screen[2]},
            polygon.albedo_color,
            polygon_normal
        };
        draw_polygon_flat_shaded(polygon_screen, light, framebuffer, z_buffer);
    }
}

sf::Color get_random_color() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    //static std::normal_distribution<float> dist(128.0f, 48.0f);
    static std::uniform_int_distribution dist(0, 255);

    /*sf::Uint8 r = static_cast<sf::Uint8>(std::clamp(static_cast<int>(std::round(dist(gen))), 0, 255));
    sf::Uint8 g = static_cast<sf::Uint8>(std::clamp(static_cast<int>(std::round(dist(gen))), 0, 255));
    sf::Uint8 b = static_cast<sf::Uint8>(std::clamp(static_cast<int>(std::round(dist(gen))), 0, 255));*/

    sf::Uint8 r = static_cast<sf::Uint8>(dist(gen));
    sf::Uint8 g = static_cast<sf::Uint8>(dist(gen));
    sf::Uint8 b = static_cast<sf::Uint8>(dist(gen));
    return sf::Color(r, g, b);
}
