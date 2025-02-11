#include <iostream>
#include <array>
#include <bitset>

#include <SFML/Graphics.hpp>

constexpr unsigned int w = 800;
constexpr unsigned int h = 600;

void set_pixel_color(std::vector<sf::Uint8>& frame_buffer, int x, int y, sf::Color color);
void fill_frame_buffer(std::vector<sf::Uint8>& frame_buffer, sf::Color color);
void draw_line_dda(std::vector<sf::Uint8>& frame_buffer, int x0, int y0, int x1, int y1, sf::Color color);

bool clip_line_coh_suth(float& x0, float& y0, float& x1, float& y1);

void test_clipping(std::vector<sf::Uint8>& frame_buffer);

int main() {
    sf::RenderWindow window(sf::VideoMode(w, h), "Cohen-Sutherland 2D Line Clipping");

    std::vector<sf::Uint8> frame_buffer(w * h * 4);
    fill_frame_buffer(frame_buffer, sf::Color::White);

    sf::Texture texture;
    if (!texture.create(w, h)) {
        std::cout << "SFML: Create texture fail.\n";
    }

    sf::Sprite sprite(texture);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        test_clipping(frame_buffer);

        texture.update(frame_buffer.data());

        window.clear();
        window.draw(sprite);
        window.display();
    }

    return 0;
}

void set_pixel_color(std::vector<sf::Uint8>& frame_buffer, int x, int y, sf::Color color) {
    const int index = (y * w + x) * 4;

    frame_buffer[index] = color.r;
    frame_buffer[index + 1] = color.g;
    frame_buffer[index + 2] = color.b;
    frame_buffer[index + 3] = color.a;
}

void fill_frame_buffer(std::vector<sf::Uint8>& frame_buffer, sf::Color color) {
    for (int i = 0; i < w * h; i++) {
        const int x = i % w;
        const int y = i / w;
        set_pixel_color(frame_buffer, x, y, color);
    }
}

void draw_line_dda(std::vector<sf::Uint8>& frame_buffer, int x0, int y0, int x1, int y1, sf::Color color) {
    // Vertical.
    if (x0 == x1) {
        // Make ascending.
        if (y0 > y1) {
            std::swap(y0, y1);
            std::swap(x0, x1);
        }

        for (int y = y0; y <= y1; y++) {
            set_pixel_color(frame_buffer, x0, y, color);
        }
    }
    // Horizontal.
    else if (y0 == y1) {
        // Make ascending.
        if (x0 > x1) {
            std::swap(x0, x1);
        }

        for (int x = x0; x <= x1; x++) {
            set_pixel_color(frame_buffer, x, y0, color);
        }
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

        float y_accum = y0;
        for (int x = x0; x <= x1; x++) {
            set_pixel_color(frame_buffer, x, std::round(y_accum), color);
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

        float x_accum = x0;
        for (int y = y0; y <= y1; y++) {
            set_pixel_color(frame_buffer, std::round(x_accum), y, color);
            x_accum += inv_slope;
        }
    }
}

bool clip_line_coh_suth(float& x0, float& y0, float& x1, float& y1) {
    enum EdgeBit {
        left = 3,
        right = 2,
        top = 1,
        bottom = 0
    };

    struct Point {
        float& x;
        float& y;
        std::bitset<4> region_code;
    };

    Point p1{ x1, y1 };
    p1.region_code.set(EdgeBit::left, p1.x < 0);
    p1.region_code.set(EdgeBit::right, p1.x > w - 1);
    p1.region_code.set(EdgeBit::top, p1.y < 0);
    p1.region_code.set(EdgeBit::bottom, p1.y > h - 1);

    Point p0{ x0, y0 };
    p0.region_code.set(EdgeBit::left, p0.x < 0);
    p0.region_code.set(EdgeBit::right, p0.x > w - 1);
    p0.region_code.set(EdgeBit::top, p0.y < 0);
    p0.region_code.set(EdgeBit::bottom, p0.y > h - 1);

    bool line_inside = (p0.region_code | p1.region_code).none();
    bool line_outside = (p0.region_code & p1.region_code).any();
    while (!line_inside && !line_outside) {
        // Make sure the first point is the one that is outside.
        if (p0.region_code.none()) {
            std::swap(p0.x, p1.x);
            std::swap(p0.y, p1.y);
            std::swap(p0.region_code, p1.region_code);
        }

        // Find the first edge outside of which the point is.
        EdgeBit first_edge;
        for (int i = EdgeBit::left; i >= EdgeBit::bottom; i--) {
            if (p0.region_code[i]) {
                first_edge = static_cast<EdgeBit>(i);
                break;
            }
        }

        if (first_edge == EdgeBit::left || first_edge == EdgeBit::right) {
            float edge_x = first_edge == EdgeBit::left ? 0 : w - 1;
            float slope = (p1.y - p0.y) / (p1.x - p0.x);

            float x_excess = edge_x - p0.x;
            p0.x = edge_x;
            p0.y += x_excess * slope;
        }
        else {
            float edge_y = first_edge == EdgeBit::top ? 0 : h - 1;
            float inv_slope = (p1.x - p0.x) / (p1.y - p0.y);

            float y_excess = edge_y - p0.y;
            p0.y = edge_y;
            p0.x += y_excess * inv_slope;
        }

        p0.region_code.set(EdgeBit::left, p0.x < 0);
        p0.region_code.set(EdgeBit::right, p0.x > w - 1);
        p0.region_code.set(EdgeBit::top, p0.y < 0);
        p0.region_code.set(EdgeBit::bottom, p0.y > h - 1);

        line_inside = (p0.region_code | p1.region_code).none();
        line_outside = (p0.region_code & p1.region_code).any();
    }

    return line_inside ? true : false;
}

void test_clipping(std::vector<sf::Uint8>& frame_buffer) {
    struct Line {
        float x0;
        float y0;

        float x1;
        float y1;
    };

    std::array<Line, 18> test_line_array{ {
            {700.0f, 10.0f, 600.0f, 20.0f}, // Trivially accepted.
            {-100.0f, -200.0f, 100.0f, -200.0f}, // Trivially rejected.
            {-200.0f, -100.0f, -200.0f, 700.0f}, // Trivially rejected.
            {-100.0f, 800.0f, 900.0f, 800.0f}, // Trivially rejected.
            {1000.0f, -100.0f, 1000.0f, 100.0f}, // Trivially rejected.
            {-50.0f, 100.0f, 50.0f, 100.0f},
            {200.0f, 50.0f, 200.0f, -50.0f},
            {850.0f, 100.0f, 750.0f, 100.0f},
            {200.0f, 550.0f, 200.0f, 650.0f},
            {-50.0f, 150.0f, 850.0f, 150.0f},
            {250.0f, -50.0f, 250.0f, 650.0f},
            {-150.0f, 50.0f, 50.0f, -150.0f},
            {-25.0f, 75.0f, 75.0f, -25.0f},
            {724.0f, -25.0f, 824.0f, 75.0f},
            {724.0f, 624.0f, 824.0f, 524.0f},
            {-25.0f, 524.0f, 75.0f, 624.0f},
            {-100.0f, -150.0f, 899.0f, 749.0f}, // The worst case 1.
            {-100.0f, 749.0f, 899.0f, -150.0f} // The worst case 2.
        } };

    for (Line& line : test_line_array) {
        if (clip_line_coh_suth(line.x0, line.y0, line.x1, line.y1)) {
            draw_line_dda(
                frame_buffer,
                std::round(line.x0), std::round(line.y0),
                std::round(line.x1), std::round(line.y1),
                sf::Color::Black);
        }
    }
}
