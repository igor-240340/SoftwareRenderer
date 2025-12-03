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

void test(const sf::Image& texture_image, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer) {
	test_triangle_1(texture_image, light, framebuffer, z_buffer);
	test_triangle_2(texture_image, light, framebuffer, z_buffer);
	test_triangle_3(texture_image, light, framebuffer, z_buffer);
}

void test_triangle_1(const sf::Image& texture_image, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer) {
	float fov_vert_rad = static_cast<float>(45.0 * (std::numbers::pi / 180.0));
	float aspect_ratio = static_cast<float>(framebuffer.w) / framebuffer.h;
	MVP mvp{
		Mat4f::create_identity(),
		Mat4f::create_identity(),
		Mat4f::create_perspective(fov_vert_rad, aspect_ratio, 0.1f, 10.0f),
		-0.1f,
		-10.0f,
		fov_vert_rad / 2.0f
	};

	Polygon p{ {
		Vertex{ Vec3f{ 0.02f, 0.004646249246696f, -0.12f }, TexCoord{ 0.0f, 0.0f } },
		Vertex{ Vec3f{ 0.02f, 0.02f, -0.08f }, TexCoord{ 0.0f, 1.0f } },
		Vertex{ Vec3f{ 0.005597225788393f, 0.006742535066108f, -0.0720720701976f }, TexCoord{ 1.0f, 0.0f } }
	} };
	rasterize_polygons_flat_shaded_textured_affine(std::vector<Polygon>{p}, texture_image, light, framebuffer, z_buffer, mvp);
}

void test_triangle_2(const sf::Image& texture_image, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer) {
	float fov_vert_rad = static_cast<float>(45.0 * (std::numbers::pi / 180.0));
	float aspect_ratio = static_cast<float>(framebuffer.w) / framebuffer.h;
	MVP mvp{
		Mat4f::create_identity(),
		Mat4f::create_identity(),
		Mat4f::create_perspective(fov_vert_rad, aspect_ratio, 0.1f, 10.0f),
		-0.1f,
		-10.0f,
		fov_vert_rad / 2.0f
	};

	Polygon p{ {
		Vertex{ Vec3f{ -0.01f, 0.006733721295353f, -0.12f }, TexCoord{ 0.0f, 0.0f } },
		Vertex{ Vec3f{ -0.03f, 0.0186003633741f, -0.08f }, TexCoord{ 0.0f, 1.0f } },
		Vertex{ Vec3f{ -0.03f, -0.008181730528715f, -0.12f }, TexCoord{ 1.0f, 0.0f } }
	} };
	rasterize_polygons_flat_shaded_textured_affine(std::vector<Polygon>{p}, texture_image, light, framebuffer, z_buffer, mvp);
}

void test_triangle_3(const sf::Image& texture_image, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer) {
	float fov_vert_rad = static_cast<float>(45.0 * (std::numbers::pi / 180.0));
	float aspect_ratio = static_cast<float>(framebuffer.w) / framebuffer.h;
	MVP mvp{
		Mat4f::create_identity(),
		Mat4f::create_identity(),
		Mat4f::create_perspective(fov_vert_rad, aspect_ratio, 0.1f, 10.0f),
		-0.1f,
		-10.0f,
		fov_vert_rad / 2.0f
	};

	Polygon p{ {
		Vertex{ Vec3f{ 0.03271314484695f, 0.01291430289638f, -0.1124120569845f }, TexCoord{ 0.0f, 1.0f } },
		Vertex{ Vec3f{ 0.03150339640582f, -0.002456323436303f, -0.1f }, TexCoord{ 0.0f, 0.0f } },
		Vertex{ Vec3f{ 0.04386767665149f, 0.0f, -0.08308856946123f }, TexCoord{ 1.0f, 0.0f } }
	} };
	rasterize_polygons_flat_shaded_textured_affine(std::vector<Polygon>{p}, texture_image, light, framebuffer, z_buffer, mvp);
}
