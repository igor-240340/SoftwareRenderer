#include <iostream>
#include <ranges>
#include <numbers>
#include <format>
#include <random>
#include <algorithm>
#include <chrono>
#include <bitset>

#include <SFML/Graphics.hpp>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include "Vec3f.h"
#include "Vec4f.h"
#include "Mat4f.h"

#include "graphics.h"

void load_model(std::string model_path, std::vector<Polygon>& polygons);

void test(const sf::Image& texture_image, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer);
void test_triangle_1(const sf::Image& texture_image, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer);
void test_triangle_2(const sf::Image& texture_image, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer);
void test_triangle_3(const sf::Image& texture_image, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer);

void rasterize_polygons_flat_shaded_textured_affine(const std::vector<Polygon>& polygons, const sf::Image& texture_image, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer, const Mat4f& transformations);

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

	sf::Texture polygon_texture;
	if (!polygon_texture.loadFromFile("data/check_texture.png")) {
		std::cout << "sfml: model_texture.loadFromFile() failed\n";
		return 1;
	}
	sf::Image polygon_texture_image = polygon_texture.copyToImage();

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

		test(polygon_texture_image, light, framebuffer, z_buffer);

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

// TODO: Когда появится камера, нужно будет сначала перевести геометрию в пространство камеры.
// То есть, сперва сделать умножение на MV.
void rasterize_polygons_flat_shaded_textured_affine(const std::vector<Polygon>& polygons, const sf::Image& texture_image, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer, const Mat4f& transformations) {
	float fov_vert_rad = static_cast<float>(45.0 * (std::numbers::pi / 180.0));
	float aspect_ratio = static_cast<float>(framebuffer.w) / framebuffer.h;
	Mat4f proj = Mat4f::create_perspective(fov_vert_rad, aspect_ratio, 0.1f, 10.0f);
	Mat4f viewport = Mat4f::create_viewport(framebuffer.w, framebuffer.h);

	for (Polygon polygon : polygons) {
		for (Vertex& vertex : polygon.vertices) {
			Vec4f pos{ vertex.pos };
			vertex.pos = transformations * pos;
		}

		// Frustum culling.
		enum FrustumPlaneBit {
			left,
			right,
			bottom,
			top,
			near,
			far
		};
		float ratio = static_cast<float>(framebuffer.w) / framebuffer.h;
		float fov_vert_half_rad = 45.0f / 2.0f * static_cast<float>(std::numbers::pi / 180.0);
		float d = 1.0f / std::tan(fov_vert_half_rad);
		float frustum_x_slope_left = ratio / d;
		float frustum_x_slope_right = -ratio / d;
		float frustum_y_slope_bottom = 1.0f / d;
		float frustum_y_slope_top = -1.0f / d;
		std::array<std::bitset<6>, 3> region_codes{};
		for (int i = 0; i < polygon.vertices.size(); ++i) {
			// Куллинг с левой плоскостью фрустума.
			float left_lim = polygon.vertices[i].pos.z * frustum_x_slope_left;
			region_codes[i].set(FrustumPlaneBit::left, polygon.vertices[i].pos.x <= left_lim);

			// Куллинг с правой плоскостью фрустума.
			float right_lim = polygon.vertices[i].pos.z * frustum_x_slope_right;
			region_codes[i].set(FrustumPlaneBit::right, polygon.vertices[i].pos.x >= right_lim);

			// Куллинг с нижней плоскостью фрустума.
			float bottom_lim = polygon.vertices[i].pos.z * frustum_y_slope_bottom;
			region_codes[i].set(FrustumPlaneBit::bottom, polygon.vertices[i].pos.y <= bottom_lim);

			// Куллинг с верхней плоскостью фрустума.
			float top_lim = polygon.vertices[i].pos.z * frustum_y_slope_top;
			region_codes[i].set(FrustumPlaneBit::top, polygon.vertices[i].pos.y >= top_lim);

			// Куллинг с дальней плоскосотью фрустума.
			float far_lim = -10.0f;
			region_codes[i].set(FrustumPlaneBit::far, polygon.vertices[i].pos.z <= far_lim);

			// Куллинг с ближней плоскосотью фрустума.
			float near_lim = -0.1f;
			region_codes[i].set(FrustumPlaneBit::near, polygon.vertices[i].pos.z >= near_lim);
		}
		// Все три вершины находятся за пределами общей плоскости - отбрасываем полигон.
		if ((region_codes[0] & region_codes[1] & region_codes[2]).any())
			continue;

		// Near plane clipping.
		int in = 0;
		int out = 0;
		for (Vertex& vertex : polygon.vertices) {
			if (vertex.pos.z > -0.1f)
				++out;
			else if (vertex.pos.z < -0.1f)
				++in;
		}
		Vertex v0 = polygon.vertices[0];
		Vertex v1 = polygon.vertices[1];
		Vertex v2 = polygon.vertices[2];
		if (in == 1 && out == 2) {
			if (v1.pos.z < -0.1f) {
				std::swap(v0, v1);
				std::swap(v1, v2);
			}
			else if (v2.pos.z < -0.1f) {
				std::swap(v0, v2);
				std::swap(v1, v2);
			}
		}
		float t1 = (-0.1f - v0.pos.z) / (v1.pos.z - v0.pos.z);
		float t2 = (-0.1f - v0.pos.z) / (v2.pos.z - v0.pos.z);
		/*
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
		*/
	}
}

void test(const sf::Image& texture_image, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer) {
	test_triangle_1(texture_image, light, framebuffer, z_buffer);
	//test_triangle_2(texture_image, light, framebuffer, z_buffer);
	//test_triangle_3(texture_image, light, framebuffer, z_buffer);
}

void test_triangle_1(const sf::Image& texture_image, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer) {
	float ratio = static_cast<float>(framebuffer.w) / framebuffer.h;
	float fov_vert_half_rad = 45.0f / 2.0f * static_cast<float>(std::numbers::pi / 180.0);
	float d = 1.0f / std::tan(fov_vert_half_rad);
	float frustum_x_slope_left = ratio / d;

	Polygon p{ {
		Vertex{ Vec3f{ 0.02f, 0.004646249246696f, -0.12f }, TexCoord{ 0.0f, 0.0f } },
		Vertex{ Vec3f{ 0.02f, 0.02f, -0.08f }, TexCoord{ 0.0f, 1.0f } },
		Vertex{ Vec3f{ 0.005597225788393f, 0.006742535066108f, -0.0720720701976f }, TexCoord{ 1.0f, 0.0f } }
	} };
	rasterize_polygons_flat_shaded_textured_affine(std::vector<Polygon>{p}, texture_image, light, framebuffer, z_buffer, Mat4f::create_identity());
}

void test_triangle_2(const sf::Image& texture_image, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer) {
	float ratio = static_cast<float>(framebuffer.w) / framebuffer.h;
	float fov_vert_half_rad = 45.0f / 2.0f * static_cast<float>(std::numbers::pi / 180.0);
	float d = 1.0f / std::tan(fov_vert_half_rad);
	float frustum_x_slope_left = ratio / d;

	Polygon p{ {
		Vertex{ Vec3f{ -0.01f, 0.006733721295353f, -0.12f }, TexCoord{ 0.0f, 0.0f } },
		Vertex{ Vec3f{ -0.03f, 0.0186003633741f, -0.08f }, TexCoord{ 0.0f, 1.0f } },
		Vertex{ Vec3f{ -0.03f, -0.008181730528715f, -0.12f }, TexCoord{ 1.0f, 0.0f } }
	} };
	rasterize_polygons_flat_shaded_textured_affine(std::vector<Polygon>{p}, texture_image, light, framebuffer, z_buffer, Mat4f::create_identity());
}

void test_triangle_3(const sf::Image& texture_image, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer) {
	float ratio = static_cast<float>(framebuffer.w) / framebuffer.h;
	float fov_vert_half_rad = 45.0f / 2.0f * static_cast<float>(std::numbers::pi / 180.0);
	float d = 1.0f / std::tan(fov_vert_half_rad);
	float frustum_x_slope_left = ratio / d;

	Polygon p{ {
		Vertex{ Vec3f{ 0.03271314484695f, 0.01291430289638f, -0.1124120569845f }, TexCoord{ 0.0f, 0.0f } },
		Vertex{ Vec3f{ 0.03150339640582f, -0.002456323436303f, -0.1f }, TexCoord{ 1.0f, 0.0f } },
		Vertex{ Vec3f{ 0.04386767665149f, 0.0f, -0.08308856946123f }, TexCoord{ 0.0f, 1.0f } }
	} };
	rasterize_polygons_flat_shaded_textured_affine(std::vector<Polygon>{p}, texture_image, light, framebuffer, z_buffer, Mat4f::create_identity());
}
