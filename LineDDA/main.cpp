#include <iostream>

#include <SFML/Graphics.hpp>

constexpr unsigned int w = 800;
constexpr unsigned int h = 600;

void set_pixel_color(std::vector<sf::Uint8>& frame_buffer, int x, int y, sf::Color color);
void fill_frame_buffer(std::vector<sf::Uint8>& frame_buffer, sf::Color color);
void draw_line_dda(std::vector<sf::Uint8>& frame_buffer, int x0, int y0, int x1, int y1, sf::Color color);

void test_draw_line_dda(std::vector<sf::Uint8>& frame_buffer);
void draw_axis(std::vector<sf::Uint8>& frame_buffer);
void draw_line_1(std::vector<sf::Uint8>& frame_buffer);
void draw_line_2(std::vector<sf::Uint8>& frame_buffer);
void draw_line_3(std::vector<sf::Uint8>& frame_buffer);
void draw_line_4(std::vector<sf::Uint8>& frame_buffer);
void draw_line_5(std::vector<sf::Uint8>& frame_buffer);
void draw_line_6(std::vector<sf::Uint8>& frame_buffer);
void draw_line_7(std::vector<sf::Uint8>& frame_buffer);
void draw_line_8(std::vector<sf::Uint8>& frame_buffer);
void draw_line_9(std::vector<sf::Uint8>& frame_buffer);
void draw_line_10(std::vector<sf::Uint8>& frame_buffer);
void draw_line_11(std::vector<sf::Uint8>& frame_buffer);
void draw_line_12(std::vector<sf::Uint8>& frame_buffer);
void draw_line_13(std::vector<sf::Uint8>& frame_buffer);
void draw_line_14(std::vector<sf::Uint8>& frame_buffer);

int main() {
    sf::RenderWindow window(sf::VideoMode(w, h), "Line Drawing DDA");

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

        test_draw_line_dda(frame_buffer);

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

void test_draw_line_dda(std::vector<sf::Uint8>& frame_buffer) {
    draw_axis(frame_buffer);

    draw_line_1(frame_buffer);
    draw_line_2(frame_buffer);
    draw_line_3(frame_buffer);  // Должен свопнуть точки.
    draw_line_4(frame_buffer);  // Должен свопнуть точки.
    draw_line_5(frame_buffer);  // Должен свопнуть точки.
    draw_line_6(frame_buffer);  // Должен свопнуть точки.
    draw_line_7(frame_buffer);  // Должен свопнуть точки.
    draw_line_8(frame_buffer);  // Должен свопнуть точки.
    draw_line_9(frame_buffer);  // Должен свопнуть точки.
    draw_line_10(frame_buffer);  // Должен свопнуть точки.
    draw_line_11(frame_buffer);
    draw_line_12(frame_buffer);
    draw_line_13(frame_buffer);
    draw_line_14(frame_buffer);
}

void draw_axis(std::vector<sf::Uint8>& frame_buffer) {
    for (int x = 0; x < w; x++) {
        set_pixel_color(frame_buffer, x, h / 2 - 1, sf::Color::Red);
    }

    for (int y = 0; y < h; y++) {
        set_pixel_color(frame_buffer, w / 2, y, sf::Color::Green);
    }
}

void draw_line_1(std::vector<sf::Uint8>& frame_buffer) {
    draw_line_dda(frame_buffer, 402, 299, 404, 299, sf::Color::Black);
}

void draw_line_2(std::vector<sf::Uint8>& frame_buffer) {
    draw_line_dda(frame_buffer, 402, 298, 404, 297, sf::Color::Black);
}

void draw_line_3(std::vector<sf::Uint8>& frame_buffer) {
    draw_line_dda(frame_buffer, 401, 297, 409, 282, sf::Color::Black);
}

void draw_line_4(std::vector<sf::Uint8>& frame_buffer) {
    draw_line_dda(frame_buffer, 401, 297, 401, 291, sf::Color::Black);
}

void draw_line_5(std::vector<sf::Uint8>& frame_buffer) {
    draw_line_dda(frame_buffer, 399, 297, 394, 285, sf::Color::Black);
}

void draw_line_6(std::vector<sf::Uint8>& frame_buffer) {
    draw_line_dda(frame_buffer, 395, 294, 385, 284, sf::Color::Black);
}

void draw_line_7(std::vector<sf::Uint8>& frame_buffer) {
    draw_line_dda(frame_buffer, 399, 298, 387, 292, sf::Color::Black);
}

void draw_line_8(std::vector<sf::Uint8>& frame_buffer) {
    draw_line_dda(frame_buffer, 392, 299, 378, 299, sf::Color::Black);
}

void draw_line_9(std::vector<sf::Uint8>& frame_buffer) {
    draw_line_dda(frame_buffer, 399, 300, 382, 304, sf::Color::Black);
}

void draw_line_10(std::vector<sf::Uint8>& frame_buffer) {
    draw_line_dda(frame_buffer, 395, 304, 388, 311, sf::Color::Black);
}

void draw_line_11(std::vector<sf::Uint8>& frame_buffer) {
    draw_line_dda(frame_buffer, 399, 301, 391, 314, sf::Color::Black);
}

void draw_line_12(std::vector<sf::Uint8>& frame_buffer) {
    draw_line_dda(frame_buffer, 401, 300, 396, 315, sf::Color::Black);
}

void draw_line_13(std::vector<sf::Uint8>& frame_buffer) {
    draw_line_dda(frame_buffer, 401, 301, 410, 321, sf::Color::Black);
}

void draw_line_14(std::vector<sf::Uint8>& frame_buffer) {
    draw_line_dda(frame_buffer, 402, 300, 403, 300, sf::Color::Black);
}
