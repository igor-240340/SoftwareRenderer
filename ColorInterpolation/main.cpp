#include <iostream>

#include <SFML/Graphics.hpp>

// NOTE: Компоненты не нормализованы и лежат в отрезке [0, 255].
struct Color3f {
    float r, g, b;
};

struct Vertex {
    float x, y;
    Color3f color;
};

constexpr unsigned int w = 800;
constexpr unsigned int h = 600;

void set_pixel_color(std::vector<sf::Uint8>& frame_buffer, int x, int y, sf::Color color);
void fill_frame_buffer(std::vector<sf::Uint8>& frame_buffer, sf::Color color);

void draw_filled_triangle(std::vector<sf::Uint8>& frame_buffer, Vertex v0, Vertex v1, Vertex v2);
void draw_flat_bottom_filled_triangle(std::vector<sf::Uint8>& frame_buffer, Vertex v0, Vertex v1, Vertex v2);
void draw_flat_top_filled_triangle(std::vector<sf::Uint8>& frame_buffer, Vertex v0, Vertex v1, Vertex v2);

void test_color_interpolation(std::vector<sf::Uint8>& frame_buffer);

// NOTE: Дублируем здесь все тесты из проекта TriangleFilling,
// поскольку добавили в код растеризации интерполяцию цвета.
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

// Оба general, но flat_top верхнего и flat_bottom нижнего не рисуются.
void draw_triangle_9(std::vector<sf::Uint8>& frame_buffer);
void draw_triangle_10(std::vector<sf::Uint8>& frame_buffer);

// Клиппинг с верхней границей окна.
void draw_triangle_11(std::vector<sf::Uint8>& frame_buffer); // flat_bottom.
void draw_triangle_12(std::vector<sf::Uint8>& frame_buffer); // flat_top.
void draw_triangle_13(std::vector<sf::Uint8>& frame_buffer); // general.

// Клиппинг с нижней границей окна.
void draw_triangle_14(std::vector<sf::Uint8>& frame_buffer); // flat_bottom.
void draw_triangle_15(std::vector<sf::Uint8>& frame_buffer); // flat_bottom. Низ находится ниже границы окна на единицу.
void draw_triangle_16(std::vector<sf::Uint8>& frame_buffer); // flat_top.
void draw_triangle_17(std::vector<sf::Uint8>& frame_buffer); // general.
void draw_triangle_18(std::vector<sf::Uint8>& frame_buffer); // general. Линия разделения в точности на границе окна.

// Клиппинг с левой границей экрана.
void draw_triangle_19(std::vector<sf::Uint8>& frame_buffer);
// Клиппинг с правой границей экрана.
void draw_triangle_20(std::vector<sf::Uint8>& frame_buffer);

// Trivial reject по верхней границе.
void draw_triangle_21(std::vector<sf::Uint8>& frame_buffer);
// Trivial reject по нижней границе.
void draw_triangle_22(std::vector<sf::Uint8>& frame_buffer);
// Trivial reject по левой границе.
void draw_triangle_23(std::vector<sf::Uint8>& frame_buffer);
// Trivial reject по правой границе.
void draw_triangle_24(std::vector<sf::Uint8>& frame_buffer);

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

void test_color_interpolation(std::vector<sf::Uint8>& frame_buffer) {
    // Референсный треугольник из папки /docs/color_interpolation.
    Vertex v0{ 413.260740594f, 308.9372855446f, {255.0f, 0.0f, 0.0f} };
    Vertex v1{ 367.9694492535f, 325.8407919207f, {0.0f, 255.0f, 0.0f} };
    Vertex v2{ 397.8374539603f, 343.7824865603f, {0.0f, 0.0f, 255.0f} };

    draw_filled_triangle(frame_buffer, v0, v1, v2);
}

void test_draw_filled_triangle(std::vector<sf::Uint8>& frame_buffer) {
    draw_triangle_1(frame_buffer); // flat_bottom.
    draw_triangle_2(frame_buffer); // general.
    draw_triangle_3(frame_buffer); // flat_top.
    draw_triangle_4(frame_buffer); // general.

    draw_triangle_5(frame_buffer);
    draw_triangle_6(frame_buffer);
    draw_triangle_7(frame_buffer);
    draw_triangle_8(frame_buffer);

    draw_triangle_9(frame_buffer);
    draw_triangle_10(frame_buffer);

    draw_triangle_11(frame_buffer);
    draw_triangle_12(frame_buffer);
    draw_triangle_13(frame_buffer);

    draw_triangle_14(frame_buffer);
    draw_triangle_15(frame_buffer);
    draw_triangle_16(frame_buffer);
    draw_triangle_17(frame_buffer);
    draw_triangle_18(frame_buffer);

    draw_triangle_19(frame_buffer);
    draw_triangle_20(frame_buffer);

    draw_triangle_21(frame_buffer);
    draw_triangle_22(frame_buffer);
    draw_triangle_23(frame_buffer);
    draw_triangle_24(frame_buffer);
}

void draw_triangle_1(std::vector<sf::Uint8>& frame_buffer) {
    Vertex v0{ 285.6405052859f, 198.4411134244f, {255.0f, 0.0f, 0.0f} };
    Vertex v1{ 100.3340052859f, 198.4411134244f, {0.0f, 255.0f, 0.0f} };
    Vertex v2{ 146.6572382321f, 58.3816139402001f, {0.0f, 0.0f, 255.0f} };

    draw_filled_triangle(frame_buffer, v0, v1, v2);
}

void draw_triangle_2(std::vector<sf::Uint8>& frame_buffer) {
    Vertex v0{ 264.0f, 0.0f, {255.0f, 0.0f, 0.0f} };
    Vertex v1{ 358.6903545140f, 21.9999980926513671875f, {0.0f, 255.0f, 0.0f} };
    Vertex v2{ 248.3387511207f, 22.0f, {0.0f, 0.0f, 255.0f} };

    draw_filled_triangle(frame_buffer, v0, v1, v2);
}

void draw_triangle_3(std::vector<sf::Uint8>& frame_buffer) {
    Vertex v0{ 12.75515676349f, 198.5f, {255.0f, 0.0f, 0.0f} };
    Vertex v1{ 135.1306413607f, 56.7063726597f, {0.0f, 255.0f, 0.0f} };
    Vertex v2{ 26.03492759676f, 56.7063726597f, {0.0f, 0.0f, 255.0f} };

    draw_filled_triangle(frame_buffer, v0, v1, v2);
}

void draw_triangle_4(std::vector<sf::Uint8>& frame_buffer) {
    Vertex v0{ 407.3869554916f, 169.36949404f, {255.0f, 0.0f, 0.0f} };
    Vertex v1{ 543.9762606744f, 256.786649357f, {0.0f, 255.0f, 0.0f} };
    Vertex v2{ 488.2478241598f, 16.3894722353f, {0.0f, 0.0f, 255.0f} };

    draw_filled_triangle(frame_buffer, v0, v1, v2);
}

void draw_triangle_5(std::vector<sf::Uint8>& frame_buffer) {
    Vertex v0{ 363.9982436391f, 159.6553802368f, {255.0f, 0.0f, 0.0f} };
    Vertex v1{ 451.6123489859f, 276.3358111202f, {0.0f, 255.0f, 0.0f} };
    Vertex v2{ 367.0f, 324.0f, {0.0f, 0.0f, 255.0f} };

    draw_filled_triangle(frame_buffer, v0, v1, v2);
}

void draw_triangle_6(std::vector<sf::Uint8>& frame_buffer) {
    Vertex v0{ 363.9982436391f, 159.6553802368f, {255.0f, 0.0f, 0.0f} };
    Vertex v1{ 367.0f, 324.0f, {0.0f, 255.0f, 0.0f} };
    Vertex v2{ 262.9930292912f, 375.789634232f, {0.0f, 0.0f, 255.0f} };

    draw_filled_triangle(frame_buffer, v0, v1, v2);
}

void draw_triangle_7(std::vector<sf::Uint8>& frame_buffer) {
    Vertex v0{ 367.0f, 324.0f, {255.0f, 0.0f, 0.0f} };
    Vertex v1{ 262.9930292912f, 375.789634232f, {0.0f, 255.0f, 0.0f} };
    Vertex v2{ 409.6016823266f, 451.2373621099f, {0.0f, 0.0f, 255.0f} };

    draw_filled_triangle(frame_buffer, v0, v1, v2);
}

void draw_triangle_8(std::vector<sf::Uint8>& frame_buffer) {
    Vertex v0{ 451.6123489859f, 276.3358111202f, {255.0f, 0.0f, 0.0f} };
    Vertex v1{ 367.0f, 324.0f, {0.0f, 255.0f, 0.0f} };
    Vertex v2{ 409.6016823266f, 451.2373621099f, {0.0f, 0.0f, 255.0f} };

    draw_filled_triangle(frame_buffer, v0, v1, v2);
}

void draw_triangle_9(std::vector<sf::Uint8>& frame_buffer) {
    Vertex v0{ 671.0229978627f, 18.3377211862f, {255.0f, 0.0f, 0.0f} };
    Vertex v1{ 733.9925299673f, 150.6619830615f, {0.0f, 255.0f, 0.0f} };
    Vertex v2{ 622.5835587075f, 150.9708484096f, {0.0f, 0.0f, 255.0f} };

    draw_filled_triangle(frame_buffer, v0, v1, v2);
}

void draw_triangle_10(std::vector<sf::Uint8>& frame_buffer) {
    Vertex v0{ 733.9925299673f, 150.6619830615f, {255.0f, 0.0f, 0.0f} };
    Vertex v1{ 622.5835587075f, 150.9708484096f, {0.0f, 255.0f, 0.0f} };
    Vertex v2{ 644.3167830776f, 233.7367509743f, {0.0f, 0.0f, 255.0f} };

    draw_filled_triangle(frame_buffer, v0, v1, v2);
}

void draw_triangle_11(std::vector<sf::Uint8>& frame_buffer) {
    Vertex v0{ 291.1065100094f, -37.3509609094f, {255.0f, 0.0f, 0.0f} };
    Vertex v1{ 283.0f, 57.0f, {0.0f, 255.0f, 0.0f} };
    Vertex v2{ 357.0f, 57.0f, {0.0f, 0.0f, 255.0f} };

    draw_filled_triangle(frame_buffer, v0, v1, v2);
}

void draw_triangle_12(std::vector<sf::Uint8>& frame_buffer) {
    Vertex v0{ 325.657620754f, -41.0f, {255.0f, 0.0f, 0.0f} };
    Vertex v1{ 433.9619401172f, -41.0f, {0.0f, 255.0f, 0.0f} };
    Vertex v2{ 388.3303817835f, 56.3531718472f, {0.0f, 0.0f, 255.0f} };

    draw_filled_triangle(frame_buffer, v0, v1, v2);
}

void draw_triangle_13(std::vector<sf::Uint8>& frame_buffer) {
    Vertex v0{ 471.9283607778f, -43.5498250651f, {255.0f, 0.0f, 0.0f} };
    Vertex v1{ 431.1210373812f, -3.32132895079997f, {0.0f, 255.0f, 0.0f} };
    Vertex v2{ 476.2695653944f, 55.7190538355f, {0.0f, 0.0f, 255.0f} };

    draw_filled_triangle(frame_buffer, v0, v1, v2);
}

void draw_triangle_14(std::vector<sf::Uint8>& frame_buffer) {
    Vertex v0{ 254.5909386425f, 509.69370691857f, {255.0f, 0.0f, 0.0f} };
    Vertex v1{ 296.1611326583f, 639.0f, {0.0f, 255.0f, 0.0f} };
    Vertex v2{ 372.0480753269f, 639.0f, {0.0f, 0.0f, 255.0f} };

    draw_filled_triangle(frame_buffer, v0, v1, v2);
}

void draw_triangle_15(std::vector<sf::Uint8>& frame_buffer) {
    Vertex v0{ 342.8840329319f, 501.35399949222f, {255.0f, 0.0f, 0.0f} };
    Vertex v1{ 350.2591925953f, 600.0f, {0.0f, 255.0f, 0.0f} };
    Vertex v2{ 434.0410063713f, 600.0f, {0.0f, 0.0f, 255.0f} };

    draw_filled_triangle(frame_buffer, v0, v1, v2);
}

void draw_triangle_16(std::vector<sf::Uint8>& frame_buffer) {
    Vertex v0{ 418.8752339453f, 532.15272658636f, {255.0f, 0.0f, 0.0f} };
    Vertex v1{ 530.5869046584f, 532.15272658636f, {0.0f, 255.0f, 0.0f} };
    Vertex v2{ 507.0256567422f, 638.70849180515f, {0.0f, 0.0f, 255.0f} };

    draw_filled_triangle(frame_buffer, v0, v1, v2);
}

void draw_triangle_17(std::vector<sf::Uint8>& frame_buffer) {
    Vertex v0{ 581.4790364904f, 533.27173635797f, {255.0f, 0.0f, 0.0f} };
    Vertex v1{ 548.8692108751f, 615.07741958214f, {0.0f, 255.0f, 0.0f} };
    Vertex v2{ 592.7238039439f, 643.18933821587f, {0.0f, 0.0f, 255.0f} };

    draw_filled_triangle(frame_buffer, v0, v1, v2);
}

void draw_triangle_18(std::vector<sf::Uint8>& frame_buffer) {
    Vertex v0{ 606.8521389828f, 534.03424793148f, {255.0f, 0.0f, 0.0f} };
    Vertex v1{ 605.8312534277f, 599.0f, {0.0f, 255.0f, 0.0f} };
    Vertex v2{ 632.9120444122f, 609.1123103102f, {0.0f, 0.0f, 255.0f} };

    draw_filled_triangle(frame_buffer, v0, v1, v2);
}

void draw_triangle_19(std::vector<sf::Uint8>& frame_buffer) {
    Vertex v0{ -241.0832003689f, 184.4208070112f, {255.0f, 0.0f, 0.0f} };
    Vertex v1{ 318.9126335423f, 389.8673086772f, {0.0f, 255.0f, 0.0f} };
    Vertex v2{ -85.75861916031f, 786.4452787002f, {0.0f, 0.0f, 255.0f} };

    draw_filled_triangle(frame_buffer, v0, v1, v2);
}

void draw_triangle_20(std::vector<sf::Uint8>& frame_buffer) {
    Vertex v0{ 1000.0f, -101.0f, {255.0f, 0.0f, 0.0f} };
    Vertex v1{ 1079.060554225f, 282.3138692418f, {0.0f, 255.0f, 0.0f} };
    Vertex v2{ 596.6828956352f, 346.2434384521f, {0.0f, 0.0f, 255.0f} };

    draw_filled_triangle(frame_buffer, v0, v1, v2);
}

void draw_triangle_21(std::vector<sf::Uint8>& frame_buffer) {
    Vertex v0{ 613.9784610366f, -6.2444807396f, {255.0f, 0.0f, 0.0f} };
    Vertex v1{ 566.4761801136f, -50.6061149898f, {0.0f, 255.0f, 0.0f} };
    Vertex v2{ 646.9552510988f, -92.6122642356f, {0.0f, 0.0f, 255.0f} };

    draw_filled_triangle(frame_buffer, v0, v1, v2);
}

void draw_triangle_22(std::vector<sf::Uint8>& frame_buffer) {
    Vertex v0{ 644.0225305711f, 646.92583968459f, {255.0f, 0.0f, 0.0f} };
    Vertex v1{ 709.8665221680f, 632.23623526015f /*599.0f*/, {0.0f, 255.0f, 0.0f} };
    Vertex v2{ 690.6836269784f, 603.029845286841f /*599.0f*/, {0.0f, 0.0f, 255.0f} };

    draw_filled_triangle(frame_buffer, v0, v1, v2);
}

void draw_triangle_23(std::vector<sf::Uint8>& frame_buffer) {
    Vertex v0{ -9.057240408918f /*0.0f*/, 31.7461100236f, {255.0f, 0.0f, 0.0f} };
    Vertex v1{ -6.249007415968f /*0.0f*/, 134.9095219599f, {0.0f, 255.0f, 0.0f} };
    Vertex v2{ -111.2917090746f, 96.9391045348f, {0.0f, 0.0f, 255.0f} };

    draw_filled_triangle(frame_buffer, v0, v1, v2);
}

void draw_triangle_24(std::vector<sf::Uint8>& frame_buffer) {
    Vertex v0{ 821.6681340053f /*w - 1*/, 368.8034453936f, {255.0f, 0.0f, 0.0f} };
    Vertex v1{ 806.5047362218f /*w - 1*/, 557.05452938627f, {0.0f, 255.0f, 0.0f} };
    Vertex v2{ 919.9969859492f, 513.86721311848f, {0.0f, 0.0f, 255.0f} };

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
    else { // Разделяем треугольник на flat_bottom и flat_top.
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

    // Цветовые компоненты начала и конца текущей скан-линии.
    // NOTE: Текущая скан-линия - это скан-линия, соответствующая текущему значению y.
    Color3f scan_line_start_color{ v0.color };

    // Slope'ы цветовых компонент при интерполяции по боковым рёбрам.
    Color3f color_slope_vert_left{
        (v1.color.r - v0.color.r) / height,
        (v1.color.g - v0.color.g) / height,
        (v1.color.b - v0.color.b) / height
    };

    Color3f color_slope_vert_right{
        (v2.color.r - v0.color.r) / height,
        (v2.color.g - v0.color.g) / height,
        (v2.color.b - v0.color.b) / height
    };

    // Slope'ы цветовых компонент при интерполяции вдоль скан-линий.
    // NOTE: Постоянны для всех скан-линий.
    Color3f color_slope_horiz{
        (color_slope_vert_right.r - color_slope_vert_left.r) / (slope_right_inv - slope_left_inv),
        (color_slope_vert_right.g - color_slope_vert_left.g) / (slope_right_inv - slope_left_inv),
        (color_slope_vert_right.b - color_slope_vert_left.b) / (slope_right_inv - slope_left_inv)
    };

    // Клиппинг с верхней границей экрана.
    int y_start = 0;
    if (v0.y < 0.0f) {
        // Корректируем начало и конец первой скан-линии.
        const float clip_height = 0.0f - v0.y;
        scan_line_start = scan_line_start + clip_height * slope_left_inv;
        scan_line_end = scan_line_end + clip_height * slope_right_inv;

        // Интерполируем цвет начала первой скан-линии.
        scan_line_start_color.r += color_slope_vert_left.r * clip_height;
        scan_line_start_color.g += color_slope_vert_left.g * clip_height;
        scan_line_start_color.b += color_slope_vert_left.b * clip_height;
    }
    else {
        // Определяем y-координату первой скан-линии (следуем правилу top-left).
        y_start = std::ceil(v0.y);

        // Корректируем начало и конец первой скан-линии.
        const float delta_y = y_start - v0.y;
        scan_line_start = scan_line_start + delta_y * slope_left_inv;
        scan_line_end = scan_line_end + delta_y * slope_right_inv;

        // Интерполируем цвет начала первой скан-линии.
        scan_line_start_color.r += color_slope_vert_left.r * delta_y;
        scan_line_start_color.g += color_slope_vert_left.g * delta_y;
        scan_line_start_color.b += color_slope_vert_left.b * delta_y;
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

        // Интерполируем цвет начала текущей скан-линии с учетом перехода к целочисленным координатам.
        // NOTE: Интерполяция цвета конца скан-линии не требуется, поскольку мы получим корректный цвет
        // естественным образом, итеративно интерполируясь вправо.
        Color3f cur_frag_color{ scan_line_start_color };
        const float scan_line_start_delta = static_cast<float>(scan_line_start_int) - scan_line_start;
        cur_frag_color.r += color_slope_horiz.r * scan_line_start_delta;
        cur_frag_color.g += color_slope_horiz.g * scan_line_start_delta;
        cur_frag_color.b += color_slope_horiz.b * scan_line_start_delta;

        for (int x = scan_line_start_int; x <= scan_line_end_int; x++) {
            sf::Color pixel_color{
                static_cast<sf::Uint8>(std::round(cur_frag_color.r)),
                static_cast<sf::Uint8>(std::round(cur_frag_color.g)),
                static_cast<sf::Uint8>(std::round(cur_frag_color.b))
            };
            set_pixel_color(frame_buffer, x, y, pixel_color);

            cur_frag_color.r += color_slope_horiz.r;
            cur_frag_color.g += color_slope_horiz.g;
            cur_frag_color.b += color_slope_horiz.b;
        }

        // Вычисляем начало и конец следующей скан-линии.
        scan_line_start += slope_left_inv;
        scan_line_end += slope_right_inv;

        // Интерполируем цвет начала следующей скан-линии.
        scan_line_start_color.r += color_slope_vert_left.r;
        scan_line_start_color.g += color_slope_vert_left.g;
        scan_line_start_color.b += color_slope_vert_left.b;
    }
}

void draw_flat_top_filled_triangle(std::vector<sf::Uint8>& frame_buffer, Vertex v0, Vertex v1, Vertex v2) {
    // Сортируем верхние вершины по x по возрастанию.
    if (v0.x > v1.x)
        std::swap(v0, v1);

    const float height = v2.y - v1.y; // Определяем высоту по правой верхней вершине.
    const float slope_left_inv = (v2.x - v0.x) / height;
    const float slope_right_inv = (v2.x - v1.x) / height;

    // Начинаем отрисовку с двух верхних вершин.
    float scan_line_start = v0.x;
    float scan_line_end = v1.x;

    // Цветовые компоненты начала и конца текущей скан-линии.
    Color3f scan_line_start_color{ v0.color };

    // Slope'ы цветовых компонент при интерполяции по боковым рёбрам.
    Color3f color_slope_vert_left{
        (v2.color.r - v0.color.r) / height,
        (v2.color.g - v0.color.g) / height,
        (v2.color.b - v0.color.b) / height
    };

    Color3f color_slope_vert_right{
        (v2.color.r - v1.color.r) / height,
        (v2.color.g - v1.color.g) / height,
        (v2.color.b - v1.color.b) / height
    };

    // Slope'ы цветовых компонент при интерполяции вдоль скан-линий.
    // NOTE: Постоянны для всех скан-линий.
    Color3f color_slope_horiz{
        (color_slope_vert_right.r - color_slope_vert_left.r) / (slope_right_inv - slope_left_inv),
        (color_slope_vert_right.g - color_slope_vert_left.g) / (slope_right_inv - slope_left_inv),
        (color_slope_vert_right.b - color_slope_vert_left.b) / (slope_right_inv - slope_left_inv)
    };

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

        // Интерполируем цвет начала первой скан-линии.
        scan_line_start_color.r += color_slope_vert_left.r * clip_height;
        scan_line_start_color.g += color_slope_vert_left.g * clip_height;
        scan_line_start_color.b += color_slope_vert_left.b * clip_height;
    }
    else {
        // Первую скан-линию определяем по правой верхней вершине, как описано в замечании
        // к растеризации flat_bottom треугольника.
        y_start = std::ceil(v1.y);

        // Корректируем начало и конец первой скан-линии.
        const float delta_y = y_start - v1.y;
        scan_line_start = scan_line_start + delta_y * slope_left_inv;
        scan_line_end = scan_line_end + delta_y * slope_right_inv;

        // Интерполируем цвет начала первой скан-линии.
        scan_line_start_color.r += color_slope_vert_left.r * delta_y;
        scan_line_start_color.g += color_slope_vert_left.g * delta_y;
        scan_line_start_color.b += color_slope_vert_left.b * delta_y;
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
        Color3f cur_frag_color{ scan_line_start_color };
        const float scan_line_start_delta = static_cast<float>(scan_line_start_int) - scan_line_start;
        cur_frag_color.r += color_slope_horiz.r * scan_line_start_delta;
        cur_frag_color.g += color_slope_horiz.g * scan_line_start_delta;
        cur_frag_color.b += color_slope_horiz.b * scan_line_start_delta;

        for (int x = scan_line_start_int; x <= scan_line_end_int; x++) {
            sf::Color pixel_color{
                static_cast<sf::Uint8>(std::round(cur_frag_color.r)),
                static_cast<sf::Uint8>(std::round(cur_frag_color.g)),
                static_cast<sf::Uint8>(std::round(cur_frag_color.b))
            };
            set_pixel_color(frame_buffer, x, y, pixel_color);

            cur_frag_color.r += color_slope_horiz.r;
            cur_frag_color.g += color_slope_horiz.g;
            cur_frag_color.b += color_slope_horiz.b;
        }

        // Вычисляем начало и конец следующей скан-линии.
        scan_line_start += slope_left_inv;
        scan_line_end += slope_right_inv;

        // Интерполируем цвет начала следующей скан-линии.
        scan_line_start_color.r += color_slope_vert_left.r;
        scan_line_start_color.g += color_slope_vert_left.g;
        scan_line_start_color.b += color_slope_vert_left.b;
    }
}
