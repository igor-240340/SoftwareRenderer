#include <iostream>

#include <SFML/Graphics.hpp>

constexpr unsigned int w = 800;
constexpr unsigned int h = 600;

void set_pixel_color(std::vector<sf::Uint8>& frame_buffer, int x, int y, sf::Color color);
void fill_frame_buffer(std::vector<sf::Uint8>& frame_buffer, sf::Color color);
void draw_line_dda(std::vector<sf::Uint8>& frame_buffer, int x0, int y0, int x1, int y1, sf::Color color);

void test_draw_line_dda();

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

        draw_line_dda(frame_buffer, 33, 21, 33, 15, sf::Color::Black);

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
    const bool vertical = (x0 == x1);
    if (vertical) {
        const bool top_down = (y0 > y1);
        if (top_down) {
            std::swap(y0, y1);
            std::swap(x0, x1);
        }
    }
}

void test_draw_line_dda() {
}
