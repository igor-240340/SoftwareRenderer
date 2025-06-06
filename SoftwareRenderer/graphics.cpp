#include "graphics.h"

void draw_polygon_wireframe(Polygon polygon_screen, FrameBuffer& frame_buffer) {
    const Vec3f& v0_pos = polygon_screen.vertices[0].pos;
    const Vec3f& v1_pos = polygon_screen.vertices[1].pos;
    const Vec3f& v2_pos = polygon_screen.vertices[2].pos;

    int x0 = static_cast<int>(std::round(v0_pos.x));
    int y0 = static_cast<int>(std::round(v0_pos.y));
    int x1 = static_cast<int>(std::round(v1_pos.x));
    int y1 = static_cast<int>(std::round(v1_pos.y));
    int x2 = static_cast<int>(std::round(v2_pos.x));
    int y2 = static_cast<int>(std::round(v2_pos.y));

    draw_line_dda(x0, y0, x1, y1, sf::Color::White, frame_buffer);
    draw_line_dda(x0, y0, x2, y2, sf::Color::White, frame_buffer);
    draw_line_dda(x1, y1, x2, y2, sf::Color::White, frame_buffer);
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
