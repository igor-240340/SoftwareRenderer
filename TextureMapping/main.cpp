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

void rasterize_polygons_flat_shaded_textured_affine(const std::vector<Polygon>& polygons, const sf::Image& texture_image, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer);

void test(const std::vector<Polygon>& polygons, const sf::Image& texture_image, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer);
void test_simple_case(const std::vector<Polygon>& polygons, const sf::Image& texture_image, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer);
void test_clipping_cases(const std::vector<Polygon>& polygons, const sf::Image& texture_image, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer);
void test_clipping_case_left(const std::vector<Polygon>& polygons, const sf::Image& texture_image, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer);
void test_clipping_case_right(const std::vector<Polygon>& polygons, const sf::Image& texture_image, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer);
void test_clipping_case_bottom(const std::vector<Polygon>& polygons, const sf::Image& texture_image, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer);
void test_clipping_case_top(const std::vector<Polygon>& polygons, const sf::Image& texture_image, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer);
void test_clipping_case_left_bottom(const std::vector<Polygon>& polygons, const sf::Image& texture_image, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer);
void test_clipping_case_left_top(const std::vector<Polygon>& polygons, const sf::Image& texture_image, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer);
void test_clipping_case_right_bottom(const std::vector<Polygon>& polygons, const sf::Image& texture_image, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer);
void test_clipping_case_right_top(const std::vector<Polygon>& polygons, const sf::Image& texture_image, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer);

// Треугольник образован двумя простыми треугольниками.
void test_general_case(const std::vector<Polygon>& polygons, const sf::Image& texture_image, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer);

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
	Framebuffer framebuffer{ w, h, std::vector<sf::Uint8>(w * h * 4) };
	ZBuffer z_buffer{ w, h, std::vector<float>(w * h) };

	std::vector<Polygon> polygons;
	std::string model_path = "data/plane_textured/plane_textured.obj";
	load_model(model_path, polygons);

	// Читаем текстуру модели.
	sf::Texture model_texture;
	if (!model_texture.loadFromFile("data/plane_textured/check_texture.png")) {
		std::cout << "sfml: model_texture.loadFromFile() failed\n";
		return 1;
	}
	sf::Image model_texture_image = model_texture.copyToImage();

	auto measure_start = high_res_clock::now();
	int frame_count = 0;

	float angle_rad_step = static_cast<float>(std::numbers::pi / 180.0);
	float two_pi = static_cast<float>(std::numbers::pi * 2.0);
	float angle_rad_accum = 0.0f;
	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed)
				window.close();
		}

		clear_framebuffer(sf::Color::Blue, framebuffer);
		clear_z_buffer(1.0f, z_buffer);

		test(polygons, model_texture_image, light, framebuffer, z_buffer);

		texture.update(framebuffer.rgba_array.data());

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

void rasterize_polygons_flat_shaded_textured_affine(const std::vector<Polygon>& polygons, const sf::Image& texture_image, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer) {
	Mat4f translation = Mat4f::create_translation(Vec3f{ 0.0f, 0.0f, -5.0f });
	Mat4f rotation_x = Mat4f::create_rotation_x(0.0f * static_cast<float>(std::numbers::pi / 180.0));
	Mat4f rotation_y = Mat4f::create_rotation_y(0.0f * static_cast<float>(std::numbers::pi / 180.0));
	Mat4f scale_xy = Mat4f::create_scale_x(0.2f) * Mat4f::create_scale_y(0.2f);

	float fov_vert_rad = static_cast<float>(45.0 * (std::numbers::pi / 180.0));
	float aspect_ratio = static_cast<float>(framebuffer.w) / framebuffer.h;
	Mat4f proj = Mat4f::create_perspective(fov_vert_rad, aspect_ratio, 0.1f, 10.0f);
	Mat4f viewport = Mat4f::create_viewport(framebuffer.w, framebuffer.h);

	for (Polygon polygon : polygons) {
		for (Vertex& vertex : polygon.vertices) {
			Vec4f pos{ vertex.pos };
			vertex.pos = translation * rotation_y * rotation_x * scale_xy * pos;
		}

		std::vector<Vertex> vertices_screen;
		for (const auto& vertex : polygon.vertices) {
			Vec4f pos{ vertex.pos };

			Vec4f pos_clip = proj * pos;
			Vec4f pos_ndc = pos_clip / pos_clip.w;
			Vec4f pos_screen = viewport * pos_ndc;

			vertices_screen.push_back(Vertex{ Vec3f{pos_screen}, vertex.tex_coord });
		}
		Polygon polygon_screen{ {vertices_screen[0], vertices_screen[1], vertices_screen[2]}, polygon.albedo_color };
		draw_polygon_flat_shaded_textured_affine(polygon_screen, texture_image, light, framebuffer, z_buffer);
	}
}

void test(const std::vector<Polygon>& polygons, const sf::Image& texture_image, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer) {
	test_simple_case(polygons, texture_image, light, framebuffer, z_buffer);
	test_clipping_cases(polygons, texture_image, light, framebuffer, z_buffer);
	test_general_case(polygons, texture_image, light, framebuffer, z_buffer);
}

void test_simple_case(const std::vector<Polygon>& polygons, const sf::Image& texture_image, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer) {
	Mat4f translation = Mat4f::create_translation(Vec3f{ -1.0f, 0.0f, -5.0f });
	Mat4f rotation_x = Mat4f::create_rotation_x(0.0f * static_cast<float>(std::numbers::pi / 180.0));
	Mat4f rotation_y = Mat4f::create_rotation_y(0.0f * static_cast<float>(std::numbers::pi / 180.0));
	Mat4f scale_xy = Mat4f::create_scale_x(0.5f) * Mat4f::create_scale_y(0.5f);

	float fov_vert_rad = static_cast<float>(45.0 * (std::numbers::pi / 180.0));
	float aspect_ratio = static_cast<float>(framebuffer.w) / framebuffer.h;
	Mat4f proj = Mat4f::create_perspective(fov_vert_rad, aspect_ratio, 0.1f, 10.0f);
	Mat4f viewport = Mat4f::create_viewport(framebuffer.w, framebuffer.h);

	for (Polygon polygon : polygons) {
		for (Vertex& vertex : polygon.vertices) {
			Vec4f pos{ vertex.pos };
			vertex.pos = translation * rotation_y * rotation_x * scale_xy * pos;
		}

		std::vector<Vertex> vertices_screen;
		for (const auto& vertex : polygon.vertices) {
			Vec4f pos{ vertex.pos };

			Vec4f pos_clip = proj * pos;
			Vec4f pos_ndc = pos_clip / pos_clip.w;
			Vec4f pos_screen = viewport * pos_ndc;

			vertices_screen.push_back(Vertex{ Vec3f{pos_screen}, vertex.tex_coord });
		}
		Polygon polygon_screen{ {vertices_screen[0], vertices_screen[1], vertices_screen[2]}, polygon.albedo_color };
		draw_polygon_flat_shaded_textured_affine(polygon_screen, texture_image, light, framebuffer, z_buffer);
	}
}

void test_clipping_cases(const std::vector<Polygon>& polygons, const sf::Image& texture_image, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer) {
	test_clipping_case_left(polygons, texture_image, light, framebuffer, z_buffer);
	test_clipping_case_right(polygons, texture_image, light, framebuffer, z_buffer);

	test_clipping_case_bottom(polygons, texture_image, light, framebuffer, z_buffer);
	test_clipping_case_top(polygons, texture_image, light, framebuffer, z_buffer);

	test_clipping_case_left_bottom(polygons, texture_image, light, framebuffer, z_buffer);
	test_clipping_case_left_top(polygons, texture_image, light, framebuffer, z_buffer);

	test_clipping_case_right_bottom(polygons, texture_image, light, framebuffer, z_buffer);
	test_clipping_case_right_top(polygons, texture_image, light, framebuffer, z_buffer);
}

void test_clipping_case_left(const std::vector<Polygon>& polygons, const sf::Image& texture_image, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer) {
	Mat4f translation = Mat4f::create_translation(Vec3f{ -2.75f, 0.0f, -5.0f });
	Mat4f rotation_x = Mat4f::create_rotation_x(0.0f * static_cast<float>(std::numbers::pi / 180.0));
	Mat4f rotation_y = Mat4f::create_rotation_y(0.0f * static_cast<float>(std::numbers::pi / 180.0));
	Mat4f scale_xy = Mat4f::create_scale_x(0.2f) * Mat4f::create_scale_y(0.2f);

	float fov_vert_rad = static_cast<float>(45.0 * (std::numbers::pi / 180.0));
	float aspect_ratio = static_cast<float>(framebuffer.w) / framebuffer.h;
	Mat4f proj = Mat4f::create_perspective(fov_vert_rad, aspect_ratio, 0.1f, 10.0f);
	Mat4f viewport = Mat4f::create_viewport(framebuffer.w, framebuffer.h);

	for (Polygon polygon : polygons) {
		for (Vertex& vertex : polygon.vertices) {
			Vec4f pos{ vertex.pos };
			vertex.pos = translation * rotation_y * rotation_x * scale_xy * pos;
		}

		std::vector<Vertex> vertices_screen;
		for (const auto& vertex : polygon.vertices) {
			Vec4f pos{ vertex.pos };

			Vec4f pos_clip = proj * pos;
			Vec4f pos_ndc = pos_clip / pos_clip.w;
			Vec4f pos_screen = viewport * pos_ndc;

			vertices_screen.push_back(Vertex{ Vec3f{pos_screen}, vertex.tex_coord });
		}
		Polygon polygon_screen{ {vertices_screen[0], vertices_screen[1], vertices_screen[2]}, polygon.albedo_color };
		draw_polygon_flat_shaded_textured_affine(polygon_screen, texture_image, light, framebuffer, z_buffer);
	}
}

void test_clipping_case_right(const std::vector<Polygon>& polygons, const sf::Image& texture_image, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer) {
	Mat4f translation = Mat4f::create_translation(Vec3f{ 2.75f, 0.0f, -5.0f });
	Mat4f rotation_x = Mat4f::create_rotation_x(0.0f * static_cast<float>(std::numbers::pi / 180.0));
	Mat4f rotation_y = Mat4f::create_rotation_y(0.0f * static_cast<float>(std::numbers::pi / 180.0));
	Mat4f scale_xy = Mat4f::create_scale_x(0.2f) * Mat4f::create_scale_y(0.2f);

	float fov_vert_rad = static_cast<float>(45.0 * (std::numbers::pi / 180.0));
	float aspect_ratio = static_cast<float>(framebuffer.w) / framebuffer.h;
	Mat4f proj = Mat4f::create_perspective(fov_vert_rad, aspect_ratio, 0.1f, 10.0f);
	Mat4f viewport = Mat4f::create_viewport(framebuffer.w, framebuffer.h);

	for (Polygon polygon : polygons) {
		for (Vertex& vertex : polygon.vertices) {
			Vec4f pos{ vertex.pos };
			vertex.pos = translation * rotation_y * rotation_x * scale_xy * pos;
		}

		std::vector<Vertex> vertices_screen;
		for (const auto& vertex : polygon.vertices) {
			Vec4f pos{ vertex.pos };

			Vec4f pos_clip = proj * pos;
			Vec4f pos_ndc = pos_clip / pos_clip.w;
			Vec4f pos_screen = viewport * pos_ndc;

			vertices_screen.push_back(Vertex{ Vec3f{pos_screen}, vertex.tex_coord });
		}
		Polygon polygon_screen{ {vertices_screen[0], vertices_screen[1], vertices_screen[2]}, polygon.albedo_color };
		draw_polygon_flat_shaded_textured_affine(polygon_screen, texture_image, light, framebuffer, z_buffer);
	}
}

void test_clipping_case_bottom(const std::vector<Polygon>& polygons, const sf::Image& texture_image, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer) {
	Mat4f translation = Mat4f::create_translation(Vec3f{ 0.0f, -2.1f, -5.0f });
	Mat4f rotation_x = Mat4f::create_rotation_x(0.0f * static_cast<float>(std::numbers::pi / 180.0));
	Mat4f rotation_y = Mat4f::create_rotation_y(0.0f * static_cast<float>(std::numbers::pi / 180.0));
	Mat4f scale_xy = Mat4f::create_scale_x(0.2f) * Mat4f::create_scale_y(0.2f);

	float fov_vert_rad = static_cast<float>(45.0 * (std::numbers::pi / 180.0));
	float aspect_ratio = static_cast<float>(framebuffer.w) / framebuffer.h;
	Mat4f proj = Mat4f::create_perspective(fov_vert_rad, aspect_ratio, 0.1f, 10.0f);
	Mat4f viewport = Mat4f::create_viewport(framebuffer.w, framebuffer.h);

	for (Polygon polygon : polygons) {
		for (Vertex& vertex : polygon.vertices) {
			Vec4f pos{ vertex.pos };
			vertex.pos = translation * rotation_y * rotation_x * scale_xy * pos;
		}

		std::vector<Vertex> vertices_screen;
		for (const auto& vertex : polygon.vertices) {
			Vec4f pos{ vertex.pos };

			Vec4f pos_clip = proj * pos;
			Vec4f pos_ndc = pos_clip / pos_clip.w;
			Vec4f pos_screen = viewport * pos_ndc;

			vertices_screen.push_back(Vertex{ Vec3f{pos_screen}, vertex.tex_coord });
		}
		Polygon polygon_screen{ {vertices_screen[0], vertices_screen[1], vertices_screen[2]}, polygon.albedo_color };
		draw_polygon_flat_shaded_textured_affine(polygon_screen, texture_image, light, framebuffer, z_buffer);
	}
}

void test_clipping_case_top(const std::vector<Polygon>& polygons, const sf::Image& texture_image, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer) {
	Mat4f translation = Mat4f::create_translation(Vec3f{ 0.0f, 2.1f, -5.0f });
	Mat4f rotation_x = Mat4f::create_rotation_x(0.0f * static_cast<float>(std::numbers::pi / 180.0));
	Mat4f rotation_y = Mat4f::create_rotation_y(0.0f * static_cast<float>(std::numbers::pi / 180.0));
	Mat4f scale_xy = Mat4f::create_scale_x(0.2f) * Mat4f::create_scale_y(0.2f);

	float fov_vert_rad = static_cast<float>(45.0 * (std::numbers::pi / 180.0));
	float aspect_ratio = static_cast<float>(framebuffer.w) / framebuffer.h;
	Mat4f proj = Mat4f::create_perspective(fov_vert_rad, aspect_ratio, 0.1f, 10.0f);
	Mat4f viewport = Mat4f::create_viewport(framebuffer.w, framebuffer.h);

	for (Polygon polygon : polygons) {
		for (Vertex& vertex : polygon.vertices) {
			Vec4f pos{ vertex.pos };
			vertex.pos = translation * rotation_y * rotation_x * scale_xy * pos;
		}

		std::vector<Vertex> vertices_screen;
		for (const auto& vertex : polygon.vertices) {
			Vec4f pos{ vertex.pos };

			Vec4f pos_clip = proj * pos;
			Vec4f pos_ndc = pos_clip / pos_clip.w;
			Vec4f pos_screen = viewport * pos_ndc;

			vertices_screen.push_back(Vertex{ Vec3f{pos_screen}, vertex.tex_coord });
		}
		Polygon polygon_screen{ {vertices_screen[0], vertices_screen[1], vertices_screen[2]}, polygon.albedo_color };
		draw_polygon_flat_shaded_textured_affine(polygon_screen, texture_image, light, framebuffer, z_buffer);
	}
}

void test_clipping_case_left_bottom(const std::vector<Polygon>& polygons, const sf::Image& texture_image, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer) {
	Mat4f translation = Mat4f::create_translation(Vec3f{ -2.75f, -2.1f, -5.0f });
	Mat4f rotation_x = Mat4f::create_rotation_x(0.0f * static_cast<float>(std::numbers::pi / 180.0));
	Mat4f rotation_y = Mat4f::create_rotation_y(0.0f * static_cast<float>(std::numbers::pi / 180.0));
	Mat4f scale_xy = Mat4f::create_scale_x(0.2f) * Mat4f::create_scale_y(0.2f);

	float fov_vert_rad = static_cast<float>(45.0 * (std::numbers::pi / 180.0));
	float aspect_ratio = static_cast<float>(framebuffer.w) / framebuffer.h;
	Mat4f proj = Mat4f::create_perspective(fov_vert_rad, aspect_ratio, 0.1f, 10.0f);
	Mat4f viewport = Mat4f::create_viewport(framebuffer.w, framebuffer.h);

	for (Polygon polygon : polygons) {
		for (Vertex& vertex : polygon.vertices) {
			Vec4f pos{ vertex.pos };
			vertex.pos = translation * rotation_y * rotation_x * scale_xy * pos;
		}

		std::vector<Vertex> vertices_screen;
		for (const auto& vertex : polygon.vertices) {
			Vec4f pos{ vertex.pos };

			Vec4f pos_clip = proj * pos;
			Vec4f pos_ndc = pos_clip / pos_clip.w;
			Vec4f pos_screen = viewport * pos_ndc;

			vertices_screen.push_back(Vertex{ Vec3f{pos_screen}, vertex.tex_coord });
		}
		Polygon polygon_screen{ {vertices_screen[0], vertices_screen[1], vertices_screen[2]}, polygon.albedo_color };
		draw_polygon_flat_shaded_textured_affine(polygon_screen, texture_image, light, framebuffer, z_buffer);
	}
}

void test_clipping_case_left_top(const std::vector<Polygon>& polygons, const sf::Image& texture_image, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer) {
	Mat4f translation = Mat4f::create_translation(Vec3f{ -2.75f, 2.1f, -5.0f });
	Mat4f rotation_x = Mat4f::create_rotation_x(0.0f * static_cast<float>(std::numbers::pi / 180.0));
	Mat4f rotation_y = Mat4f::create_rotation_y(0.0f * static_cast<float>(std::numbers::pi / 180.0));
	Mat4f scale_xy = Mat4f::create_scale_x(0.2f) * Mat4f::create_scale_y(0.2f);

	float fov_vert_rad = static_cast<float>(45.0 * (std::numbers::pi / 180.0));
	float aspect_ratio = static_cast<float>(framebuffer.w) / framebuffer.h;
	Mat4f proj = Mat4f::create_perspective(fov_vert_rad, aspect_ratio, 0.1f, 10.0f);
	Mat4f viewport = Mat4f::create_viewport(framebuffer.w, framebuffer.h);

	for (Polygon polygon : polygons) {
		for (Vertex& vertex : polygon.vertices) {
			Vec4f pos{ vertex.pos };
			vertex.pos = translation * rotation_y * rotation_x * scale_xy * pos;
		}

		std::vector<Vertex> vertices_screen;
		for (const auto& vertex : polygon.vertices) {
			Vec4f pos{ vertex.pos };

			Vec4f pos_clip = proj * pos;
			Vec4f pos_ndc = pos_clip / pos_clip.w;
			Vec4f pos_screen = viewport * pos_ndc;

			vertices_screen.push_back(Vertex{ Vec3f{pos_screen}, vertex.tex_coord });
		}
		Polygon polygon_screen{ {vertices_screen[0], vertices_screen[1], vertices_screen[2]}, polygon.albedo_color };
		draw_polygon_flat_shaded_textured_affine(polygon_screen, texture_image, light, framebuffer, z_buffer);
	}
}

void test_clipping_case_right_bottom(const std::vector<Polygon>& polygons, const sf::Image& texture_image, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer) {
	Mat4f translation = Mat4f::create_translation(Vec3f{ 2.75f, -2.1f, -5.0f });
	Mat4f rotation_x = Mat4f::create_rotation_x(0.0f * static_cast<float>(std::numbers::pi / 180.0));
	Mat4f rotation_y = Mat4f::create_rotation_y(0.0f * static_cast<float>(std::numbers::pi / 180.0));
	Mat4f scale_xy = Mat4f::create_scale_x(0.2f) * Mat4f::create_scale_y(0.2f);

	float fov_vert_rad = static_cast<float>(45.0 * (std::numbers::pi / 180.0));
	float aspect_ratio = static_cast<float>(framebuffer.w) / framebuffer.h;
	Mat4f proj = Mat4f::create_perspective(fov_vert_rad, aspect_ratio, 0.1f, 10.0f);
	Mat4f viewport = Mat4f::create_viewport(framebuffer.w, framebuffer.h);

	for (Polygon polygon : polygons) {
		for (Vertex& vertex : polygon.vertices) {
			Vec4f pos{ vertex.pos };
			vertex.pos = translation * rotation_y * rotation_x * scale_xy * pos;
		}

		std::vector<Vertex> vertices_screen;
		for (const auto& vertex : polygon.vertices) {
			Vec4f pos{ vertex.pos };

			Vec4f pos_clip = proj * pos;
			Vec4f pos_ndc = pos_clip / pos_clip.w;
			Vec4f pos_screen = viewport * pos_ndc;

			vertices_screen.push_back(Vertex{ Vec3f{pos_screen}, vertex.tex_coord });
		}
		Polygon polygon_screen{ {vertices_screen[0], vertices_screen[1], vertices_screen[2]}, polygon.albedo_color };
		draw_polygon_flat_shaded_textured_affine(polygon_screen, texture_image, light, framebuffer, z_buffer);
	}
}

void test_clipping_case_right_top(const std::vector<Polygon>& polygons, const sf::Image& texture_image, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer) {
	Mat4f translation = Mat4f::create_translation(Vec3f{ 2.75f, 2.1f, -5.0f });
	Mat4f rotation_x = Mat4f::create_rotation_x(0.0f * static_cast<float>(std::numbers::pi / 180.0));
	Mat4f rotation_y = Mat4f::create_rotation_y(0.0f * static_cast<float>(std::numbers::pi / 180.0));
	Mat4f scale_xy = Mat4f::create_scale_x(0.2f) * Mat4f::create_scale_y(0.2f);

	float fov_vert_rad = static_cast<float>(45.0 * (std::numbers::pi / 180.0));
	float aspect_ratio = static_cast<float>(framebuffer.w) / framebuffer.h;
	Mat4f proj = Mat4f::create_perspective(fov_vert_rad, aspect_ratio, 0.1f, 10.0f);
	Mat4f viewport = Mat4f::create_viewport(framebuffer.w, framebuffer.h);

	for (Polygon polygon : polygons) {
		for (Vertex& vertex : polygon.vertices) {
			Vec4f pos{ vertex.pos };
			vertex.pos = translation * rotation_y * rotation_x * scale_xy * pos;
		}

		std::vector<Vertex> vertices_screen;
		for (const auto& vertex : polygon.vertices) {
			Vec4f pos{ vertex.pos };

			Vec4f pos_clip = proj * pos;
			Vec4f pos_ndc = pos_clip / pos_clip.w;
			Vec4f pos_screen = viewport * pos_ndc;

			vertices_screen.push_back(Vertex{ Vec3f{pos_screen}, vertex.tex_coord });
		}
		Polygon polygon_screen{ {vertices_screen[0], vertices_screen[1], vertices_screen[2]}, polygon.albedo_color };
		draw_polygon_flat_shaded_textured_affine(polygon_screen, texture_image, light, framebuffer, z_buffer);
	}
}

void test_general_case(const std::vector<Polygon>& polygons, const sf::Image& texture_image, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer) {
	Mat4f translation = Mat4f::create_translation(Vec3f{ 1.0f, 0.0f, -5.0f });
	Mat4f rotation_x = Mat4f::create_rotation_x(0.0f * static_cast<float>(std::numbers::pi / 180.0));
	Mat4f rotation_y = Mat4f::create_rotation_y(55.0f * static_cast<float>(std::numbers::pi / 180.0));
	Mat4f scale_xy = Mat4f::create_scale_x(0.5f) * Mat4f::create_scale_y(0.5f);

	float fov_vert_rad = static_cast<float>(45.0 * (std::numbers::pi / 180.0));
	float aspect_ratio = static_cast<float>(framebuffer.w) / framebuffer.h;
	Mat4f proj = Mat4f::create_perspective(fov_vert_rad, aspect_ratio, 0.1f, 10.0f);
	Mat4f viewport = Mat4f::create_viewport(framebuffer.w, framebuffer.h);

	for (Polygon polygon : polygons) {
		for (Vertex& vertex : polygon.vertices) {
			Vec4f pos{ vertex.pos };
			vertex.pos = translation * rotation_y * rotation_x * scale_xy * pos;
		}

		std::vector<Vertex> vertices_screen;
		for (const auto& vertex : polygon.vertices) {
			Vec4f pos{ vertex.pos };

			Vec4f pos_clip = proj * pos;
			Vec4f pos_ndc = pos_clip / pos_clip.w;
			Vec4f pos_screen = viewport * pos_ndc;

			vertices_screen.push_back(Vertex{ Vec3f{pos_screen}, vertex.tex_coord });
		}
		Polygon polygon_screen{ {vertices_screen[0], vertices_screen[1], vertices_screen[2]}, polygon.albedo_color };
		draw_polygon_flat_shaded_textured_affine(polygon_screen, texture_image, light, framebuffer, z_buffer);
	}
}
