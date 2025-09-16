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

constexpr float deg_to_rad = static_cast<float>(std::numbers::pi / 180.0);
constexpr float rad_to_deg = 1.0f / deg_to_rad;

struct Segment {
	Vec3f a;
	Vec3f b;
};

void load_model(std::string model_path, std::vector<Polygon>& polygons);
void rasterize_polygons_flat_shaded(const std::vector<Polygon>& polygons, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer);
void draw_segment_z(const Segment& segment, Framebuffer& framebuffer, ZBuffer& z_buffer);

// Каждая функция рисует соответствующий сегмент из файла test_segments.ggb.
void draw_test_segments(Framebuffer& framebuffer, ZBuffer& z_buffer);
void draw_segment_1(Framebuffer& framebuffer, ZBuffer& z_buffer);
void draw_segment_2(Framebuffer& framebuffer, ZBuffer& z_buffer);
void draw_segment_3(Framebuffer& framebuffer, ZBuffer& z_buffer);
void draw_segment_4(Framebuffer& framebuffer, ZBuffer& z_buffer);

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

	sf::RenderWindow window(sf::VideoMode(w, h), "Draw a segment with z-buffer");
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
	const std::string model_path = "data/quad/quad.obj";
	load_model(model_path, polygons);

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

		clear_framebuffer(sf::Color::Blue, framebuffer);
		clear_z_buffer(1.0f, z_buffer);

		rasterize_polygons_flat_shaded(polygons, light, framebuffer, z_buffer);
		draw_test_segments(framebuffer, z_buffer);

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

			//polygons.push_back(Polygon{ { v0, v1, v2 }, get_random_color() });
			polygons.push_back(Polygon{ { v0, v1, v2 }, sf::Color::White });
		}
	}
}

void rasterize_polygons_flat_shaded(const std::vector<Polygon>& polygons, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer) {
	const Mat4f translation = Mat4f::create_translation(Vec3f{ 0.0f, 0.0f, -4.0f });
	const Mat4f rotation_x = Mat4f::create_rotation_x(23.0f * deg_to_rad);
	const Mat4f rotation_y = Mat4f::create_rotation_y(23.0f * deg_to_rad);

	const float fov_vert_rad = 45.0f * deg_to_rad;
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

void draw_segment_z(const Segment& segment, Framebuffer& framebuffer, ZBuffer& z_buffer) {
	float fov_vert_rad = 45.0f * deg_to_rad;
	float aspect_ratio = static_cast<float>(framebuffer.w) / framebuffer.h;
	Mat4f proj = Mat4f::create_perspective(fov_vert_rad, aspect_ratio, 0.1f, 10.0f);
	Mat4f viewport = Mat4f::create_viewport(framebuffer.w, framebuffer.h);

	Vec4f a{ segment.a };
	Vec4f b{ segment.b };

	Vec4f a_clip = proj * a;
	Vec4f a_ndc = a_clip / a_clip.w;
	Vec4f a_screen = viewport * a_ndc;

	Vec4f b_clip = proj * b;
	Vec4f b_ndc = b_clip / b_clip.w;
	Vec4f b_screen = viewport * b_ndc;

	int x0 = static_cast<int>(std::round(a_screen.x));
	int y0 = static_cast<int>(std::round(a_screen.y));
	float z0 = a_screen.z;
	int x1 = static_cast<int>(std::round(b_screen.x));
	int y1 = static_cast<int>(std::round(b_screen.y));
	float z1 = b_screen.z;
	draw_line_dda_z(x0, y0, z0, x1, y1, z1, sf::Color::Magenta, framebuffer, z_buffer);
}

void draw_test_segments(Framebuffer& framebuffer, ZBuffer& z_buffer) {
	draw_segment_1(framebuffer, z_buffer);
	draw_segment_2(framebuffer, z_buffer);
	draw_segment_3(framebuffer, z_buffer);
	draw_segment_4(framebuffer, z_buffer);
}

void draw_segment_1(Framebuffer& framebuffer, ZBuffer& z_buffer) {
	Segment s{
		Vec3f{0.327694684f, -0.544639051f, -3.22799969f},
		Vec3f{-1.63847339f, 2.72319531f, -7.86000156f}
	};

	draw_segment_z(s, framebuffer, z_buffer);
}

void draw_segment_2(Framebuffer& framebuffer, ZBuffer& z_buffer) {
	Segment s{
		Vec3f{2.487350944059f, 1.847279348198f, -5.903277503310f},
		Vec3f{-1.298756193452f, 0.2646881313974f, -2.993716320522f}
	};

	draw_segment_z(s, framebuffer, z_buffer);
}

void draw_segment_3(Framebuffer& framebuffer, ZBuffer& z_buffer) {
	Segment s{
		Vec3f{-0.5f, -1.5f, -4.0f},
		Vec3f{-0.5f, 1.3f, -4.0f}
	};

	draw_segment_z(s, framebuffer, z_buffer);
}

void draw_segment_4(Framebuffer& framebuffer, ZBuffer& z_buffer) {
	Segment s{
		Vec3f{1.805626523726f, 0.5f, -4.0f},
		Vec3f{-1.8f, 0.5f, -4.0f}
	};

	draw_segment_z(s, framebuffer, z_buffer);
}
