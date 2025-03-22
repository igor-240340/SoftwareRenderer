#include <iostream>

#include <SFML/Graphics.hpp>

constexpr unsigned int w = 800;
constexpr unsigned int h = 600;

void set_pixel_color(std::vector<sf::Uint8>& frame_buffer, int x, int y, sf::Color color);
void fill_frame_buffer(std::vector<sf::Uint8>& frame_buffer, sf::Color color);

void test_draw_solid_triangle(std::vector<sf::Uint8>& frame_buffer);
void draw_triangle_1(std::vector<sf::Uint8>& frame_buffer);

void draw_filled_triangle(std::vector<sf::Uint8>& frame_buffer, float x0, float y0, float x1, float y1, float x2, float y2);
void draw_flat_bottom_triangle(std::vector<sf::Uint8>& frame_buffer, float x0, float y0, float x1, float y1, float x2, float y2, sf::Color color);
void draw_flat_top_triangle(std::vector<sf::Uint8>& frame_buffer, float x0, float y0, float x1, float y1, float x2, float y2, sf::Color color);

int main() {
    sf::RenderWindow window(sf::VideoMode(w, h), "Correct Top-Left Triangle Filling");

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

        test_draw_solid_triangle(frame_buffer);

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

void test_draw_solid_triangle(std::vector<sf::Uint8>& frame_buffer) {
    draw_triangle_1(frame_buffer);
}

void draw_triangle_1(std::vector<sf::Uint8>& frame_buffer) {
    const float x0 = 348.69399237195f;
    const float y0 = 100.42784623434f;

    const float x1 = 577.13312257433f;
    const float y1 = 336.30308604497f;

    const float x2 = 235.16316022857f;
    const float y2 = 336.30308604497f;

    draw_filled_triangle(frame_buffer, x0, y0, x1, y1, x2, y2);
}

void draw_filled_triangle(std::vector<sf::Uint8>& frame_buffer, float x0, float y0, float x1, float y1, float x2, float y2) {
}

void draw_flat_bottom_triangle(std::vector<sf::Uint8>& frame_buffer, float x0, float y0, float x1, float y1, float x2, float y2, sf::Color color) {
}

void draw_flat_top_triangle(std::vector<sf::Uint8>& frame_buffer, float x0, float y0, float x1, float y1, float x2, float y2, sf::Color color) {
}
