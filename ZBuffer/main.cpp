#include <iostream>
#include <ranges>
#include <numbers>

#include <SFML/Graphics.hpp>

#include "Vec3f.h"
#include "Vec4f.h"
#include "Mat4f.h"

struct Vertex {
    float x, y, z;
};

constexpr unsigned int w = 800;
constexpr unsigned int h = 600;

void set_pixel_color(std::vector<sf::Uint8>& frame_buffer, int x, int y, sf::Color color);
void clear_frame_buffer(std::vector<sf::Uint8>& frame_buffer, sf::Color color);
void clear_z_buffer(std::vector<float>& z_buffer, float value = 1.0f);
bool perform_depth_test(std::vector<float>& z_buffer, int frag_x, int frag_y, float frag_z);

void draw_filled_triangle(std::vector<sf::Uint8>& frame_buffer, std::vector<float>& z_buffer, Vertex v0, Vertex v1, Vertex v2, sf::Color color);
void draw_flat_bottom_filled_triangle(std::vector<sf::Uint8>& frame_buffer, std::vector<float>& z_buffer, Vertex v0, Vertex v1, Vertex v2, sf::Color color);
void draw_flat_top_filled_triangle(std::vector<sf::Uint8>& frame_buffer, std::vector<float>& z_buffer, Vertex v0, Vertex v1, Vertex v2, sf::Color color);

void test_z_buffer_1(std::vector<sf::Uint8>& frame_buffer, std::vector<float>& z_buffer);
void draw_red_triangle_1(std::vector<sf::Uint8>& frame_buffer, std::vector<float>& z_buffer, sf::Color color = sf::Color::Red);
void draw_blue_triangle_1(std::vector<sf::Uint8>& frame_buffer, std::vector<float>& z_buffer, sf::Color color = sf::Color::Blue);

void test_z_buffer_2(std::vector<sf::Uint8>& frame_buffer, std::vector<float>& z_buffer);
void draw_red_triangle_2(std::vector<sf::Uint8>& frame_buffer, std::vector<float>& z_buffer, sf::Color color = sf::Color::Red);
void draw_blue_triangle_2(std::vector<sf::Uint8>& frame_buffer, std::vector<float>& z_buffer, sf::Color color = sf::Color::Blue);

int main() {
    sf::RenderWindow window(sf::VideoMode(w, h), "Z-buffer");

    sf::Texture texture;
    if (!texture.create(w, h)) {
        std::cout << "SFML: Create texture fail.\n";
    }

    sf::Sprite sprite(texture);

    std::vector<sf::Uint8> frame_buffer(w * h * 4);
    std::vector<float> z_buffer(w * h);
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        clear_frame_buffer(frame_buffer, sf::Color::Black);
        clear_z_buffer(z_buffer);

        //test_z_buffer_1(frame_buffer, z_buffer);
        test_z_buffer_2(frame_buffer, z_buffer);

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

void clear_frame_buffer(std::vector<sf::Uint8>& frame_buffer, sf::Color color) {
    for (int i = 0; i < w * h; i++) {
        const int x = i % w;
        const int y = i / w;
        set_pixel_color(frame_buffer, x, y, color);
    }
}

void clear_z_buffer(std::vector<float>& z_buffer, float value) {
    std::ranges::fill(z_buffer, 1.0f);
}

bool perform_depth_test(std::vector<float>& z_buffer, int frag_x, int frag_y, float frag_z) {
    const int index = w * frag_y + frag_x;
    if (frag_z < z_buffer[index]) {
        z_buffer[index] = frag_z;
        return true;
    }

    return false;
}

void test_z_buffer_1(std::vector<sf::Uint8>& frame_buffer, std::vector<float>& z_buffer) {
    draw_red_triangle_1(frame_buffer, z_buffer);
    draw_blue_triangle_1(frame_buffer, z_buffer);
}

void draw_red_triangle_1(std::vector<sf::Uint8>& frame_buffer, std::vector<float>& z_buffer, sf::Color color) {
    Vec4f v0_pos_world{ 0.12858511992886f, 0.09014727201472f, -3.062135394734f };
    Vec4f v1_pos_world{ 0.0f, 0.1f, -4.0f };
    Vec4f v2_pos_world{ -0.0017940119725222f, -0.013699806553022f, -6.567130320242f };

    const Mat4f proj = Mat4f::create_perspective(
        static_cast<float>(45.0 * (std::numbers::pi / 180.0)), static_cast<float>(w) / h, 0.1f, 10.0f);

    Vec4f v0_pos_clip = proj * v0_pos_world;
    Vec4f v1_pos_clip = proj * v1_pos_world;
    Vec4f v2_pos_clip = proj * v2_pos_world;

    Vec4f v0_pos_ndc = v0_pos_clip / v0_pos_clip.w;
    Vec4f v1_pos_ndc = v1_pos_clip / v1_pos_clip.w;
    Vec4f v2_pos_ndc = v2_pos_clip / v2_pos_clip.w;

    const Mat4f viewport = Mat4f::create_viewport(w, h);
    Vec4f v0_pos_screen = viewport * v0_pos_ndc;
    Vec4f v1_pos_screen = viewport * v1_pos_ndc;
    Vec4f v2_pos_screen = viewport * v2_pos_ndc;

    Vertex v0{ v0_pos_screen.x, v0_pos_screen.y, v0_pos_screen.z };
    Vertex v1{ v1_pos_screen.x, v1_pos_screen.y, v1_pos_screen.z };
    Vertex v2{ v2_pos_screen.x, v2_pos_screen.y, v2_pos_screen.z };

    draw_filled_triangle(frame_buffer, z_buffer, v0, v1, v2, color);
}

void draw_blue_triangle_1(std::vector<sf::Uint8>& frame_buffer, std::vector<float>& z_buffer, sf::Color color) {
    Vec4f v0_pos_world{ 0.05348720083878f, 0.1339907277809f, -3.684037874323f };
    Vec4f v1_pos_world{ -0.02750651338714f, -0.03447517605658f, -3.722437583851f };
    Vec4f v2_pos_world{ 0.08517046970116f, 0.00457807936321f, -5.049799420868f };

    const Mat4f proj = Mat4f::create_perspective(
        static_cast<float>(45.0 * (std::numbers::pi / 180.0)), static_cast<float>(w) / h, 0.1f, 10.0f);

    Vec4f v0_pos_clip = proj * v0_pos_world;
    Vec4f v1_pos_clip = proj * v1_pos_world;
    Vec4f v2_pos_clip = proj * v2_pos_world;

    Vec4f v0_pos_ndc = v0_pos_clip / v0_pos_clip.w;
    Vec4f v1_pos_ndc = v1_pos_clip / v1_pos_clip.w;
    Vec4f v2_pos_ndc = v2_pos_clip / v2_pos_clip.w;

    const Mat4f viewport = Mat4f::create_viewport(w, h);
    Vec4f v0_pos_screen = viewport * v0_pos_ndc;
    Vec4f v1_pos_screen = viewport * v1_pos_ndc;
    Vec4f v2_pos_screen = viewport * v2_pos_ndc;

    Vertex v0{ v0_pos_screen.x, v0_pos_screen.y, v0_pos_screen.z };
    Vertex v1{ v1_pos_screen.x, v1_pos_screen.y, v1_pos_screen.z };
    Vertex v2{ v2_pos_screen.x, v2_pos_screen.y, v2_pos_screen.z };

    draw_filled_triangle(frame_buffer, z_buffer, v0, v1, v2, color);

    const int index = 281 * w + 407;
    std::cout << z_buffer[index] << '\n';
}

void test_z_buffer_2(std::vector<sf::Uint8>& frame_buffer, std::vector<float>& z_buffer) {
    draw_red_triangle_2(frame_buffer, z_buffer);
    draw_blue_triangle_2(frame_buffer, z_buffer);
}

void draw_red_triangle_2(std::vector<sf::Uint8>& frame_buffer, std::vector<float>& z_buffer, sf::Color color) {
    Vec4f v0_pos_world{ 1.840996095964f, -1.073676172127f, -3.628196093860f };
    Vec4f v1_pos_world{ 0.06124679400900f, -0.8294603178124f, -3.556538776620f };
    Vec4f v2_pos_world{ 0.5524953604496f, 1.379455366808f, -7.088986515136f };

    const Mat4f proj = Mat4f::create_perspective(
        static_cast<float>(45.0 * (std::numbers::pi / 180.0)), static_cast<float>(w) / h, 0.1f, 10.0f);

    Vec4f v0_pos_clip = proj * v0_pos_world;
    Vec4f v1_pos_clip = proj * v1_pos_world;
    Vec4f v2_pos_clip = proj * v2_pos_world;

    Vec4f v0_pos_ndc = v0_pos_clip / v0_pos_clip.w;
    Vec4f v1_pos_ndc = v1_pos_clip / v1_pos_clip.w;
    Vec4f v2_pos_ndc = v2_pos_clip / v2_pos_clip.w;

    const Mat4f viewport = Mat4f::create_viewport(w, h);
    Vec4f v0_pos_screen = viewport * v0_pos_ndc;
    Vec4f v1_pos_screen = viewport * v1_pos_ndc;
    Vec4f v2_pos_screen = viewport * v2_pos_ndc;

    Vertex v0{ v0_pos_screen.x, v0_pos_screen.y, v0_pos_screen.z };
    Vertex v1{ v1_pos_screen.x, v1_pos_screen.y, v1_pos_screen.z };
    Vertex v2{ v2_pos_screen.x, v2_pos_screen.y, v2_pos_screen.z };

    draw_filled_triangle(frame_buffer, z_buffer, v0, v1, v2, color);
}

void draw_blue_triangle_2(std::vector<sf::Uint8>& frame_buffer, std::vector<float>& z_buffer, sf::Color color) {
    Vec4f v0_pos_world{ 1.580934848009f, 0.5966194549801f, -3.802308656357f };
    Vec4f v1_pos_world{ -0.8612277692645f, 0.3735791812889f, -6.322245336638f };
    Vec4f v2_pos_world{ 1.131590337138f, -1.400787745470f, -3.985780117663f };

    const Mat4f proj = Mat4f::create_perspective(
        static_cast<float>(45.0 * (std::numbers::pi / 180.0)), static_cast<float>(w) / h, 0.1f, 10.0f);

    Vec4f v0_pos_clip = proj * v0_pos_world;
    Vec4f v1_pos_clip = proj * v1_pos_world;
    Vec4f v2_pos_clip = proj * v2_pos_world;

    Vec4f v0_pos_ndc = v0_pos_clip / v0_pos_clip.w;
    Vec4f v1_pos_ndc = v1_pos_clip / v1_pos_clip.w;
    Vec4f v2_pos_ndc = v2_pos_clip / v2_pos_clip.w;

    const Mat4f viewport = Mat4f::create_viewport(w, h);
    Vec4f v0_pos_screen = viewport * v0_pos_ndc;
    Vec4f v1_pos_screen = viewport * v1_pos_ndc;
    Vec4f v2_pos_screen = viewport * v2_pos_ndc;

    Vertex v0{ v0_pos_screen.x, v0_pos_screen.y, v0_pos_screen.z };
    Vertex v1{ v1_pos_screen.x, v1_pos_screen.y, v1_pos_screen.z };
    Vertex v2{ v2_pos_screen.x, v2_pos_screen.y, v2_pos_screen.z };

    draw_filled_triangle(frame_buffer, z_buffer, v0, v1, v2, color);

    const int index = 281 * w + 407;
    std::cout << z_buffer[index] << '\n';
}

void draw_filled_triangle(std::vector<sf::Uint8>& frame_buffer, std::vector<float>& z_buffer, Vertex v0, Vertex v1, Vertex v2, sf::Color color) {
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
        draw_flat_bottom_filled_triangle(frame_buffer, z_buffer, v0, v1, v2, color);
    else if (flat_top)
        draw_flat_top_filled_triangle(frame_buffer, z_buffer, v0, v1, v2, color);
    else { // Разделяем треугольник на flat_bottom и flat_top.
        const float inv_slope = (v2.x - v0.x) / (v2.y - v0.y); // Наклон самого длинного ребра.
        const float height_top_triangle = v1.y - v0.y;

        // Точка пересечения на длинном ребре при разделении треугольников.
        const float intersect_x = v0.x + height_top_triangle * inv_slope;
        const float intersect_y = v1.y;

        // Определяем z-атрибут точки пересечения, интерполируя вдоль самого длинного ребра.
        const float z_slope_vert = (v2.z - v0.z) / (v2.y - v0.y);
        const float intersect_z = v0.z + (height_top_triangle * z_slope_vert);

        Vertex intersect{ intersect_x, intersect_y, intersect_z };

        draw_flat_bottom_filled_triangle(frame_buffer, z_buffer, v0, intersect, v1, color);
        draw_flat_top_filled_triangle(frame_buffer, z_buffer, intersect, v1, v2, color);
    }
}

void draw_flat_bottom_filled_triangle(std::vector<sf::Uint8>& frame_buffer, std::vector<float>& z_buffer, Vertex v0, Vertex v1, Vertex v2, sf::Color color) {
    // Сортируем нижние вершины по x по возрастанию.
    if (v1.x > v2.x)
        std::swap(v1, v2);

    const float height = v2.y - v0.y; // Определяем высоту по правой нижней вершине.
    const float slope_left_inv = (v1.x - v0.x) / height;
    const float slope_right_inv = (v2.x - v0.x) / height;

    // Начинаем отрисовку с верхней вершины.
    float scan_line_start = v0.x;
    float scan_line_end = v0.x;

    // Z-атрибут начала текущей скан-линии.
    // NOTE: Текущая скан-линия - это скан-линия, соответствующая текущему значению y.
    float scan_line_start_z = v0.z;

    // Slope'ы z-атрибутов при интерполяции по боковым рёбрам.
    float z_slope_vert_left = (v1.z - v0.z) / height;
    float z_slope_vert_right = (v2.z - v0.z) / height;

    // Slope z-атрибута при интерполяции вдоль скан-линий.
    // NOTE: Постоянный для всех скан-линий.
    float z_slope_horiz = (z_slope_vert_right - z_slope_vert_left) / (slope_right_inv - slope_left_inv);

    // Клиппинг с верхней границей экрана.
    int y_start = 0;
    if (v0.y < 0.0f) {
        // Корректируем начало и конец первой скан-линии.
        const float clip_height = 0.0f - v0.y;
        scan_line_start = scan_line_start + clip_height * slope_left_inv;
        scan_line_end = scan_line_end + clip_height * slope_right_inv;

        // Интерполируем z-атрибут начала первой скан-линии.
        scan_line_start_z += z_slope_vert_left * clip_height;
    }
    else {
        // Определяем y-координату первой скан-линии (следуем правилу top-left).
        y_start = std::ceil(v0.y);

        // Корректируем начало и конец первой скан-линии.
        const float delta_y = y_start - v0.y;
        scan_line_start = scan_line_start + delta_y * slope_left_inv;
        scan_line_end = scan_line_end + delta_y * slope_right_inv;

        // Интерполируем z-атрибут начала первой скан-линии.
        scan_line_start_z += z_slope_vert_left * delta_y;
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

        // Интерполируем z-атрибут начала текущей скан-линии с учетом перехода к целочисленным координатам.
        // NOTE: Интерполяция конца скан-линии не требуется, поскольку мы получим корректный z-атрибут
        // естественным образом, итеративно интерполируясь вправо.
        float cur_frag_z = scan_line_start_z;
        const float scan_line_start_delta = static_cast<float>(scan_line_start_int) - scan_line_start;
        cur_frag_z += z_slope_horiz * scan_line_start_delta;
        for (int x = scan_line_start_int; x <= scan_line_end_int; x++) {
            if (perform_depth_test(z_buffer, x, y, cur_frag_z))
                set_pixel_color(frame_buffer, x, y, color);

            cur_frag_z += z_slope_horiz;
        }

        // Вычисляем начало и конец следующей скан-линии.
        scan_line_start += slope_left_inv;
        scan_line_end += slope_right_inv;

        // Интерполируем z-атрибут начала следующей скан-линии.
        scan_line_start_z += z_slope_vert_left;
    }
}

void draw_flat_top_filled_triangle(std::vector<sf::Uint8>& frame_buffer, std::vector<float>& z_buffer, Vertex v0, Vertex v1, Vertex v2, sf::Color color) {
    // Сортируем верхние вершины по x по возрастанию.
    if (v0.x > v1.x)
        std::swap(v0, v1);

    const float height = v2.y - v1.y; // Определяем высоту по правой верхней вершине.
    const float slope_left_inv = (v2.x - v0.x) / height;
    const float slope_right_inv = (v2.x - v1.x) / height;

    // Начинаем отрисовку с двух верхних вершин.
    float scan_line_start = v0.x;
    float scan_line_end = v1.x;

    // Z-атрибут начала текущей скан-линии.
    float scan_line_start_z = v0.z;

    // Slope'ы z-атрибутов при интерполяции по боковым рёбрам.
    float z_slope_vert_left = (v2.z - v0.z) / height;
    float z_slope_vert_right = (v2.z - v1.z) / height;

    // Slope z-атрибута при интерполяции вдоль скан-линий.
    // NOTE: Постоянный для всех скан-линий.
    float z_slope_horiz = (z_slope_vert_right - z_slope_vert_left) / (slope_right_inv - slope_left_inv);

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

        // Интерполируем z-атрибут начала первой скан-линии.
        scan_line_start_z += z_slope_vert_left * clip_height;
    }
    else {
        // Первую скан-линию определяем по правой верхней вершине, как описано в замечании
        // к растеризации flat_bottom треугольника.
        y_start = std::ceil(v1.y);

        // Корректируем начало и конец первой скан-линии.
        const float delta_y = y_start - v1.y;
        scan_line_start = scan_line_start + delta_y * slope_left_inv;
        scan_line_end = scan_line_end + delta_y * slope_right_inv;

        // Интерполируем z-атрибут начала первой скан-линии.
        scan_line_start_z += z_slope_vert_left * delta_y;
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

        // Интерполируем цвет начала текущей скан-линии с учетом перехода к целочисленным координатам.
        float cur_frag_z = scan_line_start_z;
        const float scan_line_start_delta = static_cast<float>(scan_line_start_int) - scan_line_start;
        cur_frag_z += z_slope_horiz * scan_line_start_delta;
        for (int x = scan_line_start_int; x <= scan_line_end_int; x++) {
            if (perform_depth_test(z_buffer, x, y, cur_frag_z))
                set_pixel_color(frame_buffer, x, y, color);

            cur_frag_z += z_slope_horiz;
        }

        // Вычисляем начало и конец следующей скан-линии.
        scan_line_start += slope_left_inv;
        scan_line_end += slope_right_inv;

        // Интерполируем z-атрибут начала следующей скан-линии.
        scan_line_start_z += z_slope_vert_left;
    }
}
