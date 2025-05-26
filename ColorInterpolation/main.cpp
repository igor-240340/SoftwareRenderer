#include <iostream>

#include <SFML/Graphics.hpp>

struct Colorf {
    float r, g, b;
};

struct Vertex {
    float x, y;
    Colorf color;
};

constexpr unsigned int w = 800;
constexpr unsigned int h = 600;

void set_pixel_color(std::vector<sf::Uint8>& frame_buffer, int x, int y, sf::Color color);
void fill_frame_buffer(std::vector<sf::Uint8>& frame_buffer, sf::Color color);

void draw_filled_triangle(std::vector<sf::Uint8>& frame_buffer, Vertex v0, Vertex v1, Vertex v2);
void draw_flat_bottom_filled_triangle(std::vector<sf::Uint8>& frame_buffer, Vertex v0, Vertex v1, Vertex v2);
void draw_flat_top_filled_triangle(std::vector<sf::Uint8>& frame_buffer, Vertex v0, Vertex v1, Vertex v2);

void test_color_interpolation(std::vector<sf::Uint8>& frame_buffer);

int main() {
    sf::RenderWindow window(sf::VideoMode(w, h), "Triangle Color Interpolation");

    std::vector<sf::Uint8> frame_buffer(w * h * 4);
    fill_frame_buffer(frame_buffer, sf::Color::Black);

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

        test_color_interpolation(frame_buffer);

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

void test_color_interpolation(std::vector<sf::Uint8>& frame_buffer) {
    Vertex v0{ 413.260740594f, 308.9372855446f, {255.0f, 0.0f, 0.0f} };
    Vertex v1{ 367.9694492535f, 325.8407919207f, {0.0f, 255.0f, 0.0f} };
    Vertex v2{ 397.8374539603f, 343.7824865603f, {0.0f, 0.0f, 255.0f} };

    draw_filled_triangle(frame_buffer, v0, v1, v2);
}

void draw_filled_triangle(std::vector<sf::Uint8>& frame_buffer, Vertex v0, Vertex v1, Vertex v2) {
    // Сортируем вершины по Y по возрастанию.
    if (v0.y > v1.y)
        std::swap(v0, v1);
    if (v1.y > v2.y)
        std::swap(v1, v2);
    if (v0.y > v1.y)
        std::swap(v0, v1);

    // Trivial reject по верхней/нижней границе.
    // NOTE: Нет смысла исключать ноль для верхней границы, поскольку в итоге все-равно
    // получим ceil(0)-1 для y-координаты нижней скан-линии и растеризации не будет.
    // Для нижней же границы ноль нужно исключить, чтобы не было пропуска пикселов по нижней стороне смежного треугольника.
    if (v2.y <= 0.0f || v0.y > (h - 1))
        return;

    // Trivial reject по левой границе.
    // NOTE: Нет смысла исключать ноль для левой границы, поскольку в итоге все-равно
    // получим ceil(0)-1 для концов всех скан-линий и растеризации не будет.
    if ((v0.x <= 0.0f) && (v1.x <= 0.0f) && (v2.x <= 0.0f))
        return;

    // Trivial reject по правой границе.
    // NOTE: Важно исключить ноль, чтобы не было пропуска пикселов по правой стороне смежного треугольника.
    if ((v0.x > (w - 1)) && (v1.x > (w - 1)) && (v2.x > (w - 1)))
        return;

    // Классифицируем треугольник.
    // NOTE: Проверяем на точное равенство, а не через epsilon, поскольку в противном случае
    // возможна некорректная растеризация: пропуск пиксела или двойная растеризация одного и того же
    // пиксела для двух смежных треугольников.
    const bool flat_bottom = (v1.y == v2.y);
    const bool flat_top = (v0.y == v1.y);
    if (flat_bottom)
        draw_flat_bottom_filled_triangle(frame_buffer, v0, v1, v2);
    else if (flat_top)
        draw_flat_top_filled_triangle(frame_buffer, v0, v1, v2);
    // Разделяем треугольник на flat_bottom и flat_top.
    else {
        const float inv_slope = (v2.x - v0.x) / (v2.y - v0.y); // Наклон самого длинного ребра.
        const float height_top_triangle = v1.y - v0.y;

        // Точка пересечения на длинном ребре при разделении треугольников.
        const float intersect_x = v0.x + height_top_triangle * inv_slope;
        const float intersect_y = v1.y;

        // Определяем цвет точки пересечения, интерполируя компоненты вдоль самого длинного ребра.
        const float r_slope_vert = (v2.color.r - v0.color.r) / (v2.y - v0.y);
        const float g_slope_vert = (v2.color.g - v0.color.g) / (v2.y - v0.y);
        const float b_slope_vert = (v2.color.b - v0.color.b) / (v2.y - v0.y);

        const float intersect_r = v0.color.r + (height_top_triangle * r_slope_vert);
        const float intersect_g = v0.color.g + (height_top_triangle * g_slope_vert);
        const float intersect_b = v0.color.b + (height_top_triangle * b_slope_vert);

        Vertex intersect{ intersect_x, intersect_y, {intersect_r, intersect_g, intersect_b} };

        draw_flat_bottom_filled_triangle(frame_buffer, v0, intersect, v1);
        draw_flat_top_filled_triangle(frame_buffer, intersect, v1, v2);
    }
}

void draw_flat_bottom_filled_triangle(std::vector<sf::Uint8>& frame_buffer, Vertex v0, Vertex v1, Vertex v2) {
    // Сортируем нижние вершины по x по возрастанию.
    if (v1.x > v2.x)
        std::swap(v1, v2);

    const float height = v2.y - v0.y; // Определяем высоту по правой нижней вершине.
    const float slope_left_inv = (v1.x - v0.x) / height;
    const float slope_right_inv = (v2.x - v0.x) / height;

    // Начинаем отрисовку с верхней вершины.
    float scan_line_start = v0.x;
    float scan_line_end = v0.x;

    // Клиппинг с верхней границей экрана.
    int y_start = 0;
    if (v0.y < 0.0f) {
        // Корректируем начало и конец первой скан-линии.
        const float clip_height = 0.0f - v0.y;
        scan_line_start = scan_line_start + clip_height * slope_left_inv;
        scan_line_end = scan_line_end + clip_height * slope_right_inv;
    }
    else {
        // Определяем y-координату первой скан-линии (следуем правилу top-left).
        y_start = std::ceil(v0.y);

        // Корректируем начало и конец первой скан-линии.
        const float delta_y = y_start - v0.y;
        scan_line_start = scan_line_start + delta_y * slope_left_inv;
        scan_line_end = scan_line_end + delta_y * slope_right_inv;
    }

    // Определяем y-координату последней скан-линии (следуем правилу top-left).
    // NOTE: Если для текущего flat_bottom треугольника существует снизу смежный flat_top,
    // то поскольку последняя скан-линия flat_bottom треугольника определяется по правой нижней координате,
    // то первая скан-линия смежного flat_top треугольника должна определяться по правой верхней,
    // чтобы не было ни пропуска скан-линии ни наложения.
    int y_end = std::ceil(v2.y) - 1;
    if (v2.y > h)
        y_end = h - 1;

    for (int y = y_start; y <= y_end; y++) {
        // Вычисляем целочисленные значения начала и конца текущей скан-линии, следуя правилу top-left.
        // Если начало скан-линии находится за левой границей окна, прижимаем к нулю.
        // Если конец скан-линии находится за правой границей окна плюс один пиксел, то прижимаем к правой границе.
        const int scan_line_start_int = (scan_line_start < 0.0f) ? 0 : std::ceil(scan_line_start);
        const int scan_line_end_int = (scan_line_end > w) ? (w - 1) : (std::ceil(scan_line_end) - 1);

        for (int x = scan_line_start_int; x <= scan_line_end_int; x++) {
            set_pixel_color(frame_buffer, x, y, sf::Color::Red);
        }

        // Вычисляем начало и конец следующей скан-линии.
        scan_line_start += slope_left_inv;
        scan_line_end += slope_right_inv;
    }
}

void draw_flat_top_filled_triangle(std::vector<sf::Uint8>& frame_buffer, Vertex v0, Vertex v1, Vertex v2) {
    // Сортируем верхние вершины по X по возрастанию.
    if (v0.x > v1.x)
        std::swap(v0, v1);

    const float height = v2.y - v1.y; // Определяем высоту по правой верхней вершине.
    const float slope_left_inv = (v2.x - v0.x) / height;
    const float slope_right_inv = (v2.x - v1.x) / height;

    // Начинаем отрисовку с двух верхних вершин.
    float scan_line_start = v0.x;
    float scan_line_end = v1.x;

    // Клиппинг с верхней границей экрана.
    // NOTE: Чтобы быть последовательными, ориентируемся на правую вершину, поскольку по ней определяем
    // высоту и по ней же определяем y-координату первой скан-линии. Более того, по правой же вершине определяем
    // для flat_bottom треугольника y-координату последней скан-линии.
    int y_start = 0;
    if (v1.y < 0.0f) {
        // Корректируем начало и конец первой скан-линии.
        const float clip_height = 0.0f - v1.y;
        scan_line_start = scan_line_start + clip_height * slope_left_inv;
        scan_line_end = scan_line_end + clip_height * slope_right_inv;
    }
    else {
        // Первую скан-линию определяем по правой верхней вершине, как описано в замечании
        // к растеризации flat_bottom треугольника.
        y_start = std::ceil(v1.y);

        // Корректируем начало и конец первой скан-линии.
        const float delta_y = y_start - v1.y;
        scan_line_start = scan_line_start + delta_y * slope_left_inv;
        scan_line_end = scan_line_end + delta_y * slope_right_inv;
    }

    int y_end = std::ceil(v2.y) - 1;
    if (v2.y > h)
        y_end = h - 1;

    for (int y = y_start; y <= y_end; y++) {
        // Вычисляем целочисленные значения начала и конца текущей скан-линии, следуя правилу top-left.
        // Если начало скан-линии находится за левой границей окна, прижимаем к нулю.
        // Если конец скан-линии находится за правой границей окна плюс один пиксел, то прижимаем к правой границе.
        const int scan_line_start_int = (scan_line_start < 0.0f) ? 0 : std::ceil(scan_line_start);
        const int scan_line_end_int = (scan_line_end > w) ? (w - 1) : (std::ceil(scan_line_end) - 1);

        for (int x = scan_line_start_int; x <= scan_line_end_int; x++) {
            set_pixel_color(frame_buffer, x, y, sf::Color::Red);
        }

        // Вычисляем начало и конец следующей скан-линии.
        scan_line_start += slope_left_inv;
        scan_line_end += slope_right_inv;
    }
}
