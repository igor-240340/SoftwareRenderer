#include <bitset>

#include "graphics.h"

void draw_polygon_wireframe(Polygon polygon_screen, FrameBuffer& frame_buffer) {
    const Vec3f& v0_pos_orig = polygon_screen.vertices[0].pos;
    const Vec3f& v1_pos_orig = polygon_screen.vertices[1].pos;
    const Vec3f& v2_pos_orig = polygon_screen.vertices[2].pos;

    Vec3f v0_pos_copy = v0_pos_orig;
    Vec3f v1_pos_copy = v1_pos_orig;
    Vec3f v2_pos_copy = v2_pos_orig;

    if (clip_line_coh_suth(v0_pos_copy.x, v0_pos_copy.y, v1_pos_copy.x, v1_pos_copy.y, frame_buffer)) {
        const int x0 = static_cast<int>(std::round(v0_pos_copy.x));
        const int y0 = static_cast<int>(std::round(v0_pos_copy.y));
        const int x1 = static_cast<int>(std::round(v1_pos_copy.x));
        const int y1 = static_cast<int>(std::round(v1_pos_copy.y));
        draw_line_dda(x0, y0, x1, y1, sf::Color::White, frame_buffer);
    }

    // Восстанавливаем после клиппинга.
    v0_pos_copy = v0_pos_orig;
    if (clip_line_coh_suth(v0_pos_copy.x, v0_pos_copy.y, v2_pos_copy.x, v2_pos_copy.y, frame_buffer)) {
        const int x0 = static_cast<int>(std::round(v0_pos_copy.x));
        const int y0 = static_cast<int>(std::round(v0_pos_copy.y));
        const int x1 = static_cast<int>(std::round(v2_pos_copy.x));
        const int y1 = static_cast<int>(std::round(v2_pos_copy.y));
        draw_line_dda(x0, y0, x1, y1, sf::Color::White, frame_buffer);
    }

    // Восстанавливаем после клиппинга.
    v1_pos_copy = v1_pos_orig;
    v2_pos_copy = v2_pos_orig;
    if (clip_line_coh_suth(v1_pos_copy.x, v1_pos_copy.y, v2_pos_copy.x, v2_pos_copy.y, frame_buffer)) {
        const int x0 = static_cast<int>(std::round(v1_pos_copy.x));
        const int y0 = static_cast<int>(std::round(v1_pos_copy.y));
        const int x1 = static_cast<int>(std::round(v2_pos_copy.x));
        const int y1 = static_cast<int>(std::round(v2_pos_copy.y));
        draw_line_dda(x0, y0, x1, y1, sf::Color::White, frame_buffer);
    }
}

void draw_line_dda(int x0, int y0, int x1, int y1, sf::Color color, FrameBuffer& frame_buffer) {
    // Vertical.
    if (x0 == x1) {
        // Make ascending.
        if (y0 > y1)
            std::swap(y0, y1);

        for (int y = y0; y <= y1; y++)
            set_pixel_color(x0, y, color, frame_buffer);
    }
    // Horizontal.
    else if (y0 == y1) {
        // Make ascending.
        if (x0 > x1)
            std::swap(x0, x1);

        for (int x = x0; x <= x1; x++)
            set_pixel_color(x, y0, color, frame_buffer);
    }

    int dy = y1 - y0;
    int dx = x1 - x0;

    // Non-steep.
    if (std::abs(dy) <= std::abs(dx)) {
        // Make ascending.
        if (x0 > x1) {
            std::swap(x0, x1);
            std::swap(y0, y1);
            dx = -dx;
            dy = -dy;
        }

        const float slope = static_cast<float>(dy) / dx;

        float y_accum = static_cast<float>(y0);
        for (int x = x0; x <= x1; x++) {
            int y = static_cast<int>(std::round(y_accum));
            set_pixel_color(x, y, color, frame_buffer);
            y_accum += slope;
        }
    }
    // Steep.
    else {
        // Make ascending.
        if (y0 > y1) {
            std::swap(y0, y1);
            std::swap(x0, x1);
            dy = -dy;
            dx = -dx;
        }

        const float inv_slope = static_cast<float>(dx) / dy;

        float x_accum = static_cast<float>(x0);
        for (int y = y0; y <= y1; y++) {
            int x = static_cast<int>(std::round(x_accum));
            set_pixel_color(x, y, color, frame_buffer);
            x_accum += inv_slope;
        }
    }
}

void set_pixel_color(int x, int y, sf::Color color, FrameBuffer& frame_buffer) {
    const int index = (y * frame_buffer.w + x) * 4;

    frame_buffer.rgba_array[index] = color.r;
    frame_buffer.rgba_array[index + 1] = color.g;
    frame_buffer.rgba_array[index + 2] = color.b;
    frame_buffer.rgba_array[index + 3] = color.a;
}

void clear_frame_buffer(sf::Color color, FrameBuffer& frame_buffer) {
    for (size_t i = 0; i < frame_buffer.rgba_array.size(); i += 4) {
        frame_buffer.rgba_array[i] = color.r;
        frame_buffer.rgba_array[i + 1] = color.g;
        frame_buffer.rgba_array[i + 2] = color.b;
        frame_buffer.rgba_array[i + 3] = color.a;
    }
}

bool clip_line_coh_suth(float& x0, float& y0, float& x1, float& y1, const FrameBuffer& frame_buffer) {
    enum EdgeBit {
        left = 3,
        right = 2,
        top = 1,
        bottom = 0
    };

    struct Point {
        float x;
        float y;
        std::bitset<4> region_code;
    };

    Point p0{ x0, y0 };
    p0.region_code.set(EdgeBit::left, p0.x < 0);
    p0.region_code.set(EdgeBit::right, p0.x > frame_buffer.w - 1);
    p0.region_code.set(EdgeBit::top, p0.y < 0);
    p0.region_code.set(EdgeBit::bottom, p0.y > frame_buffer.h - 1);

    Point p1{ x1, y1 };
    p1.region_code.set(EdgeBit::left, p1.x < 0);
    p1.region_code.set(EdgeBit::right, p1.x > frame_buffer.w - 1);
    p1.region_code.set(EdgeBit::top, p1.y < 0);
    p1.region_code.set(EdgeBit::bottom, p1.y > frame_buffer.h - 1);

    bool line_inside = (p0.region_code | p1.region_code).none();
    bool line_outside = (p0.region_code & p1.region_code).any();
    while (!line_inside && !line_outside) {
        // Make sure the first point is the one that is outside.
        if (p0.region_code.none())
            std::swap(p0, p1);

        // Find the first edge outside of which the point is.
        EdgeBit first_edge{};
        for (int i = 3; i >= 0; --i) {
            if (p0.region_code[i]) {
                first_edge = static_cast<EdgeBit>(i);
                break;
            }
        }

        if (first_edge == EdgeBit::left || first_edge == EdgeBit::right) {
            float edge_x = first_edge == EdgeBit::left ? 0 : (frame_buffer.w - 1);
            float slope = (p1.y - p0.y) / (p1.x - p0.x);

            float x_excess = edge_x - p0.x;
            p0.x = edge_x;
            p0.y += x_excess * slope;
        }
        else {
            float edge_y = first_edge == EdgeBit::top ? 0 : (frame_buffer.h - 1);
            float inv_slope = (p1.x - p0.x) / (p1.y - p0.y);

            float y_excess = edge_y - p0.y;
            p0.y = edge_y;
            p0.x += y_excess * inv_slope;
        }

        p0.region_code.set(EdgeBit::left, p0.x < 0);
        p0.region_code.set(EdgeBit::right, p0.x > frame_buffer.w - 1);
        p0.region_code.set(EdgeBit::top, p0.y < 0);
        p0.region_code.set(EdgeBit::bottom, p0.y > frame_buffer.h - 1);

        line_inside = (p0.region_code | p1.region_code).none();
        line_outside = (p0.region_code & p1.region_code).any();
    }

    x0 = p0.x;
    y0 = p0.y;
    x1 = p1.x;
    y1 = p1.y;

    return line_inside;
}
