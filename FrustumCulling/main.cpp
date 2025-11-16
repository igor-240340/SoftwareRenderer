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

void rasterize_polygons_flat_shaded_textured_affine(const std::vector<Polygon>& polygons, const sf::Image& texture_image, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer, const Mat4f& transformations);

void test(const sf::Image& texture_image, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer);
void test_left_plane(const sf::Image& texture_image, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer);
void test_right_plane(const sf::Image& texture_image, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer);

/*
void test_bottom_plane(const sf::Image& texture_image, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer);
void test_triangle_7(const sf::Image& texture_image, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer);
void test_triangle_8(const sf::Image& texture_image, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer);
void test_triangle_9(const sf::Image& texture_image, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer);

void test_far_plane(const sf::Image& texture_image, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer);
void test_triangle_10(const sf::Image& texture_image, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer);
void test_triangle_11(const sf::Image& texture_image, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer);
void test_triangle_12(const sf::Image& texture_image, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer);

void test_top_plane(const sf::Image& texture_image, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer);
void test_triangle_13(const sf::Image& texture_image, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer);
void test_triangle_14(const sf::Image& texture_image, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer);
void test_triangle_15(const sf::Image& texture_image, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer);

void test_near_plane(const sf::Image& texture_image, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer);
void test_triangle_16(const sf::Image& texture_image, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer);
void test_triangle_17(const sf::Image& texture_image, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer);
void test_triangle_18(const sf::Image& texture_image, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer);
*/

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

void test(const sf::Image& texture_image, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer) {
	test_left_plane(texture_image, light, framebuffer, z_buffer);
	test_right_plane(texture_image, light, framebuffer, z_buffer);
	//test_bottom_plane(texture_image, light, framebuffer, z_buffer);
	//test_far_plane(texture_image, light, framebuffer, z_buffer);
	//test_top_plane(texture_image, light, framebuffer, z_buffer);
	//test_near_plane(texture_image, light, framebuffer, z_buffer);
}

void test_left_plane(const sf::Image& texture_image, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer) {
	float ratio = static_cast<float>(framebuffer.w) / framebuffer.h;
	float fov_vert_half_rad = 45.0f / 2.0f * static_cast<float>(std::numbers::pi / 180.0);
	float d = 1.0f / std::tan(fov_vert_half_rad);
	float frustum_x_slope_left = ratio / d;

	std::vector<Polygon> polygons{
		Polygon{ {
			Vertex{ Vec3f{ -2.0f, 1.0f, -7.0f}, TexCoord{1.0f, 0.0f} },
			Vertex{ Vec3f{ (-8.39680419252f * frustum_x_slope_left), 2.663429555932f, -8.39680419252f}, TexCoord{0.0f, 1.0f} },
			Vertex{ Vec3f{ (-7.750896177711f * frustum_x_slope_left), 0.3975463449545f, -7.750896177711f}, TexCoord{0.0f, 0.0f} }
		} },
		Polygon{ {
			Vertex{ Vec3f{ (-5.484101989121f * frustum_x_slope_left), -1.581996699731f, -5.484101989121f}, TexCoord{1.0f, 0.0f} },
			Vertex{ Vec3f{ (-5.719705745321f * frustum_x_slope_left), -0.5f, -5.719705745321f}, TexCoord{0.0f, 1.0f} },
			Vertex{ Vec3f{ -5.243424042825f, 2.0f, -5.012349197755f}, TexCoord{0.0f, 0.0f} }
		} },
		Polygon{ {
			Vertex{ Vec3f{ -2.0f, 1.0f, -7.0f}, TexCoord{1.0f, 0.0f} },
			Vertex{ Vec3f{ -4.637426902844f, 2.663429555932f, -8.39680419252f}, TexCoord{0.0f, 1.0f} },
			Vertex{ Vec3f{ -4.280701756471f, 0.3975463449545f, -7.750896177711f}, TexCoord{0.0f, 0.0f} }
		} }
	};
	rasterize_polygons_flat_shaded_textured_affine(polygons, texture_image, light, framebuffer, z_buffer, Mat4f::create_identity());
}

void test_right_plane(const sf::Image& texture_image, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer) {
	float ratio = static_cast<float>(framebuffer.w) / framebuffer.h;
	float fov_vert_half_rad = 45.0f / 2.0f * static_cast<float>(std::numbers::pi / 180.0);
	float d = 1.0f / std::tan(fov_vert_half_rad);
	float frustum_x_slope_right = -ratio / d;

	// NOTE: Поскольку при растеризации применяется правило top-left,
	// вершины, лежащие в точности на правой плоскости фрустума, спроецируются на правый крайний пиксел экрана равный (w - 1)
	// и результирующий пиксел для растеризации окажется равен (w - 1) - 1.
	// Поэтому мы увидим зазор в один пиксел между треугольником и правой границей экрана.
	// То же самое будет с вершинами, лежащими на нижней плоскости фрустума.
	std::vector<Polygon> polygons{
		Polygon{ {
			Vertex{ Vec3f{ (-8.146862577963f * frustum_x_slope_right), -1.409099401413f, -8.146862577963f}, TexCoord{1.0f, 0.0f} },
			Vertex{ Vec3f{ (-7.848171181114f * frustum_x_slope_right), 0.4851195548111f, -7.848171181114f}, TexCoord{0.0f, 1.0f} },
			Vertex{ Vec3f{ 2.528168363525f, 0.0f, -7.060050183665f}, TexCoord{0.0f, 0.0f} }
		} },
		Polygon{{
			Vertex{ Vec3f{ 4.0f, 0.5643227641134f, -4.0f}, TexCoord{1.0f, 0.0f} },
			Vertex{ Vec3f{ (-6.203766351018f * frustum_x_slope_right), -0.463252873246f, -6.203766351018f}, TexCoord{0.0f, 1.0f} },
			Vertex{ Vec3f{ (-5.300545534366f * frustum_x_slope_right), -1.0f, -5.300545534366f}, TexCoord{0.0f, 0.0f} }
		} },
		Polygon{ {
			Vertex{ Vec3f{ 4.0f, -0.6218971824187f, -5.0f}, TexCoord{1.0f, 0.0f} },
			Vertex{ Vec3f{ 5.0f, 1.0f, -6.0f}, TexCoord{0.0f, 1.0f} },
			Vertex{ Vec3f{ 5.0f, 0.0f, -7.0f}, TexCoord{0.0f, 0.0f} }
		} }
	};
	rasterize_polygons_flat_shaded_textured_affine(polygons, texture_image, light, framebuffer, z_buffer, Mat4f::create_identity());
}
