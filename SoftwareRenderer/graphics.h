#pragma once

#include <array>

#include <SFML/Graphics.hpp>

#include "Vec3f.h"

struct Vertex_new {
    Vec3f pos;
};

struct Polygon {
    std::array<Vertex_new, 3> vertices;
};

struct FrameBuffer {
    int w;
    int h;
    std::vector<sf::Uint8> rgba_array;
};

void draw_polygon_wireframe(Polygon polygon_screen, FrameBuffer& frame_buffer);
void draw_line_dda(int x0, int y0, int x1, int y1, sf::Color color, FrameBuffer& frame_buffer);
void set_pixel_color(int x, int y, sf::Color color, FrameBuffer& frame_buffer);
void clear_frame_buffer(sf::Color color, FrameBuffer& frame_buffer);
bool clip_line_coh_suth(float& x0, float& y0, float& x1, float& y1, const FrameBuffer& frame_buffer);
