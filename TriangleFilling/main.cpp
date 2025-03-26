#include <iostream>
#include <random>

#include <SFML/Graphics.hpp>

constexpr float epsilon = 1e-06;

constexpr unsigned int w = 800;
constexpr unsigned int h = 600;

void set_pixel_color(std::vector<sf::Uint8>& frame_buffer, int x, int y, sf::Color color);
void fill_frame_buffer(std::vector<sf::Uint8>& frame_buffer, sf::Color color);

void test_draw_filled_triangle(std::vector<sf::Uint8>& frame_buffer);
void draw_triangle_1(std::vector<sf::Uint8>& frame_buffer);
void draw_triangle_2(std::vector<sf::Uint8>& frame_buffer);
void draw_triangle_3(std::vector<sf::Uint8>& frame_buffer);
void draw_triangle_4(std::vector<sf::Uint8>& frame_buffer);

// Смежные.
void draw_triangle_5(std::vector<sf::Uint8>& frame_buffer);
void draw_triangle_6(std::vector<sf::Uint8>& frame_buffer);
void draw_triangle_7(std::vector<sf::Uint8>& frame_buffer);
void draw_triangle_8(std::vector<sf::Uint8>& frame_buffer);

void draw_filled_triangle(std::vector<sf::Uint8>& frame_buffer, float x0, float y0, float x1, float y1, float x2, float y2);
void draw_flat_bottom_filled_triangle(std::vector<sf::Uint8>& frame_buffer, float x0, float y0, float x1, float y1, float x2, float y2, sf::Color color);
void draw_flat_top_filled_triangle(std::vector<sf::Uint8>& frame_buffer, float x0, float y0, float x1, float y1, float x2, float y2, sf::Color color);

sf::Color get_random_color() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(0, 255);

    return sf::Color(dist(gen), dist(gen), dist(gen));
}

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

        test_draw_filled_triangle(frame_buffer);

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

void test_draw_filled_triangle(std::vector<sf::Uint8>& frame_buffer) {
    draw_triangle_1(frame_buffer); // flat_bottom.
    draw_triangle_2(frame_buffer); // flat_bottom.
    draw_triangle_3(frame_buffer); // flat_top.

    draw_triangle_4(frame_buffer); // general.

    draw_triangle_5(frame_buffer);
    draw_triangle_6(frame_buffer);
    draw_triangle_7(frame_buffer);
    draw_triangle_8(frame_buffer);
}

void draw_triangle_1(std::vector<sf::Uint8>& frame_buffer) {
    const float x0 = 285.64885f;
    const float y0 = 198.5106210655f;

    const float x1 = 100.34235f;
    const float y1 = 198.5106210655f;

    const float x2 = 146.66558f;
    const float y2 = 58.45112f;

    draw_filled_triangle(frame_buffer, x0, y0, x1, y1, x2, y2);
}

void draw_triangle_2(std::vector<sf::Uint8>& frame_buffer) {
    const float x0 = 242.9755968939f;
    const float y0 = 125.8927598072f;

    const float x1 = 353.3272002871f;
    const float y1 = 125.8927599072f; // В десятичном виде y0 не равен y1, но после конвертации в бинарный float становятся в точности равны.

    const float x2 = 244.0495784354f;
    const float y2 = 6.14381792539996f;

    draw_filled_triangle(frame_buffer, x0, y0, x1, y1, x2, y2);
}

void draw_triangle_3(std::vector<sf::Uint8>& frame_buffer) {
    const float x0 = 12.75515676349f;
    const float y0 = 198.5f;

    const float x1 = 135.1306413607f;
    const float y1 = 56.7063731597f;

    const float x2 = 26.03492759676f;
    const float y2 = 56.7063726597f;

    draw_filled_triangle(frame_buffer, x0, y0, x1, y1, x2, y2);
}

void draw_triangle_4(std::vector<sf::Uint8>& frame_buffer) {
    const float x0 = 407.3869554916f;
    const float y0 = 169.36949404f;

    const float x1 = 543.9762606744f;
    const float y1 = 256.786649357f;

    const float x2 = 488.2478241598f;
    const float y2 = 16.3894722353f;

    draw_filled_triangle(frame_buffer, x0, y0, x1, y1, x2, y2);
}

void draw_triangle_5(std::vector<sf::Uint8>& frame_buffer) {
    const float x0 = 363.9982436391f;
    const float y0 = 159.6553802368f;

    const float x1 = 451.6123489859f;
    const float y1 = 276.3358111202f;

    const float x2 = 366.7336551233f;
    const float y2 = 324.3480015879f;

    draw_filled_triangle(frame_buffer, x0, y0, x1, y1, x2, y2);
}

void draw_triangle_6(std::vector<sf::Uint8>& frame_buffer) {
    const float x0 = 363.9982436391f;
    const float y0 = 159.6553802368f;

    const float x1 = 366.7336551233f;
    const float y1 = 324.3480015879f;

    const float x2 = 262.9930292912f;
    const float y2 = 375.789634232f;

    draw_filled_triangle(frame_buffer, x0, y0, x1, y1, x2, y2);
}

void draw_triangle_7(std::vector<sf::Uint8>& frame_buffer) {
    const float x0 = 366.7336551233f;
    const float y0 = 324.3480015879f;

    const float x1 = 262.9930292912f;
    const float y1 = 375.789634232f;

    const float x2 = 409.6016823266f;
    const float y2 = 451.2373621099f;

    draw_filled_triangle(frame_buffer, x0, y0, x1, y1, x2, y2);
}

void draw_triangle_8(std::vector<sf::Uint8>& frame_buffer) {
    const float x0 = 451.6123489859f;
    const float y0 = 276.3358111202f;

    const float x1 = 366.7336551233f;
    const float y1 = 324.3480015879f;

    const float x2 = 409.6016823266f;
    const float y2 = 451.2373621099f;

    draw_filled_triangle(frame_buffer, x0, y0, x1, y1, x2, y2);
}

void draw_filled_triangle(std::vector<sf::Uint8>& frame_buffer, float x0, float y0, float x1, float y1, float x2, float y2) {
    // Сортируем вершины по Y по возврастанию.
    if (y0 > y1) {
        std::swap(y0, y1);
        std::swap(x0, x1);
    }
    if (y1 > y2) {
        std::swap(y1, y2);
        std::swap(x1, x2);
    }
    if (y0 > y1) {
        std::swap(y0, y1);
        std::swap(x0, x1);
    }

    // Классифицируем треугольник.
    const bool flat_bottom = std::abs(y1 - y2) < epsilon;
    const bool flat_top = std::abs(y0 - y1) < epsilon;
    if (flat_bottom) {
        draw_flat_bottom_filled_triangle(frame_buffer, x0, y0, x1, y1, x2, y2, get_random_color());
    }
    else if (flat_top) {
        draw_flat_top_filled_triangle(frame_buffer, x0, y0, x1, y1, x2, y2, get_random_color());
    }
    // Разделяем треугольник на flat_bottom и flat_top.
    else {
        const float inv_slope = (x2 - x0) / (y2 - y0); // Наклон самой длинной грани.
        const float height_top_triangle = y1 - y0;

        // Точка пересечения на длинной грани при разделении треугольников.
        const float intersect_x = x0 + height_top_triangle * inv_slope;
        const float intersect_y = y1;

        draw_flat_bottom_filled_triangle(frame_buffer, x0, y0, intersect_x, intersect_y, x1, y1, get_random_color());
        draw_flat_top_filled_triangle(frame_buffer, intersect_x, intersect_y, x1, y1, x2, y2, get_random_color());
    }
}

void draw_flat_bottom_filled_triangle(std::vector<sf::Uint8>& frame_buffer, float x0, float y0, float x1, float y1, float x2, float y2, sf::Color color) {
    // Сортируем нижние вершины по X по возрастанию.
    if (x1 > x2) {
        std::swap(x1, x2);
        std::swap(y1, y2);
    }

    const float height = y2 - y0; // Определяем высоту по правой нижней вершине.
    const float slope_left_inv = (x1 - x0) / height;
    const float slope_right_inv = (x2 - x0) / height;

    // Начинаем отрисовку с верхней вершины.
    float scan_line_start = x0;
    float scan_line_end = x0;

    // Определяем y-координату первой скан-линии (следуем правилу top-left).
    const int y_start = std::ceil(y0);

    // Корректируем начало и конец первой скан-линии.
    const float delta_y = y_start - y0;
    scan_line_start = scan_line_start + delta_y * slope_left_inv;
    scan_line_end = scan_line_end + delta_y * slope_right_inv;

    // Определяем y-координату последней скан-линии (следуем правилу top-left).
    // NOTE: Если для текущего flat_bottom треугольника существует смежный flat_top,
    // то, поскольку последняя скан-линия flat_bottom треугольника определяется по правой нижней координате,
    // то первая скан-линия смежного flat_top треугольника должна определяться по правой верхней,
    // чтобы не было ни пропуска скан-линии ни наложения.
    const int y_end = std::ceil(y2) - 1;

    for (int y = y_start; y <= y_end; y++) {
        // Вычисляем целочисленные значения начала и конца текущей скан-линии,
        // следуя правилу top-left.
        const int scan_line_start_int = std::ceil(scan_line_start);
        const int scan_line_end_int = std::ceil(scan_line_end) - 1;

        //std::cout << y << "," << scan_line_start_int << "," << scan_line_end_int << std::endl;

        for (int x = scan_line_start_int; x <= scan_line_end_int; x++) {
            set_pixel_color(frame_buffer, x, y, color);
        }

        // Вычисляем начало и конец следующей скан-линии.
        scan_line_start += slope_left_inv;
        scan_line_end += slope_right_inv;
    }
}

void draw_flat_top_filled_triangle(std::vector<sf::Uint8>& frame_buffer, float x0, float y0, float x1, float y1, float x2, float y2, sf::Color color) {
    // Сортируем верхние вершины по X по возрастанию.
    if (x0 > x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    const float height = y2 - y1; // Определяем высоту по правой верхней вершине.
    const float slope_left_inv = (x2 - x0) / height;
    const float slope_right_inv = (x2 - x1) / height;

    // Начинаем отрисовку с двух верхних вершин.
    float scan_line_start = x0;
    float scan_line_end = x1;

    // Первую скан-линию определяем по правой верхней вершине, как описано в замечании
    // к растеризации flat_bottom треугольника.
    const int y_start = std::ceil(y1);

    // Корректируем начало и конец первой скан-линии.
    const float delta_y = y_start - y1;
    scan_line_start = scan_line_start + delta_y * slope_left_inv;
    scan_line_end = scan_line_end + delta_y * slope_right_inv;

    const int y_end = std::ceil(y2) - 1;

    for (int y = y_start; y <= y_end; y++) {
        // Вычисляем целочисленные значения начала и конца текущей скан-линии,
        // следуя правилу top-left.
        const int scan_line_start_int = std::ceil(scan_line_start);
        const int scan_line_end_int = std::ceil(scan_line_end) - 1;

        //std::cout << y << "," << scan_line_start_int << "," << scan_line_end_int << std::endl;

        for (int x = scan_line_start_int; x <= scan_line_end_int; x++) {
            set_pixel_color(frame_buffer, x, y, color);
        }

        // Вычисляем начало и конец следующей скан-линии.
        scan_line_start += slope_left_inv;
        scan_line_end += slope_right_inv;
    }
}
