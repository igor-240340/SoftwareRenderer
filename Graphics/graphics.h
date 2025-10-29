#pragma once

#include <array>

#include <SFML/Graphics.hpp>

#include "Vec3f.h"

struct TexCoord {
	float u;
	float v;
};

struct Vertex {
	Vec3f pos;
	TexCoord tex_coord;
};

struct Polygon {
	std::array<Vertex, 3> vertices;
	sf::Color albedo_color;
	Vec3f normal;
};

struct Light {
	Vec3f dir;
};

struct Framebuffer {
	int w;
	int h;
	std::vector<sf::Uint8> rgba_array;
};

struct ZBuffer {
	int w;
	int h;
	std::vector<float> depth_array;
};

void set_pixel_color(int x, int y, sf::Color color, Framebuffer& framebuffer);

void draw_line_dda(int x0, int y0, int x1, int y1, sf::Color color, Framebuffer& framebuffer);
void draw_line_dda_z(float x0, float y0, float z0, float x1, float y1, float z1, sf::Color color, Framebuffer& framebuffer, ZBuffer& z_buffer);

bool clip_line_coh_suth(float& x0, float& y0, float& x1, float& y1, const Framebuffer& framebuffer);
bool clip_line_coh_suth_z(float& x0, float& y0, float& z0, float& x1, float& y1, float& z1, const Framebuffer& framebuffer);

void draw_polygon_wireframe(Polygon polygon_screen, Framebuffer& framebuffer);

void draw_polygon_solid(Polygon polygon_screen, Framebuffer& framebuffer, ZBuffer& z_buffer);
void draw_flat_bottom_polygon_solid(Polygon polygon_screen, Framebuffer& framebuffer, ZBuffer& z_buffer);
void draw_flat_top_polygon_solid(Polygon polygon_screen, Framebuffer& framebuffer, ZBuffer& z_buffer);

void draw_polygon_flat_shaded(Polygon polygon_screen, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer);
void draw_flat_bottom_polygon_flat_shaded(Polygon polygon_screen, Framebuffer& framebuffer, ZBuffer& z_buffer);
void draw_flat_top_polygon_flat_shaded(Polygon polygon_screen, Framebuffer& framebuffer, ZBuffer& z_buffer);

void draw_polygon_flat_shaded_textured_affine(Polygon polygon_screen, const sf::Image& texture_image, const Light& light, Framebuffer& framebuffer, ZBuffer& z_buffer);
void draw_flat_bottom_polygon_flat_shaded_textured_affine(Polygon polygon_screen, const sf::Image& texture_image, float light_intensity, Framebuffer& framebuffer, ZBuffer& z_buffer);
void draw_flat_top_polygon_flat_shaded_textured_affine(Polygon polygon_screen, const sf::Image& texture_image, float light_intensity, Framebuffer& framebuffer, ZBuffer& z_buffer);

bool perform_depth_test(int frag_x, int frag_y, float frag_z, ZBuffer& z_buffer);

void clear_framebuffer(sf::Color color, Framebuffer& framebuffer);
void clear_z_buffer(float depth_value, ZBuffer& z_buffer);

sf::Color read_framebuffer(int x, int y, const Framebuffer& framebuffer);
float read_z_buffer(int x, int y, const ZBuffer& z_buffer);