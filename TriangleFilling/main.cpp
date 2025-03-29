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

void draw_filled_triangle(std::vector<sf::Uint8>& frame_buffer, float x0, float y0, float x1, float y1, float x2, float y2);
void draw_flat_bottom_filled_triangle(std::vector<sf::Uint8>& frame_buffer, float x0, float y0, float x1, float y1, float x2, float y2, sf::Color color);
void draw_flat_top_filled_triangle(std::vector<sf::Uint8>& frame_buffer, float x0, float y0, float x1, float y1, float x2, float y2, sf::Color color);

sf::Color get_random_color() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<int> dist(0, 255);

    return sf::Color(dist(gen), dist(gen), dist(gen));
}

int main() {
    sf::RenderWindow window(sf::VideoMode(w, h), "Correct Top-Left Triangle Filling");

    std::vector<sf::Uint8> frame_buffer(w * h * 4);
    fill_frame_buffer(frame_buffer, sf::Color::Yellow);

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
}

void draw_triangle_1(std::vector<sf::Uint8>& frame_buffer) {
    const float x0 = 285.6405052859f;
    const float y0 = 198.4411134244f;

    const float x1 = 100.3340052859f;
    const float y1 = 198.4411134244f;

    const float x2 = 146.6572382321f;
    const float y2 = 58.3816139402001f;

    draw_filled_triangle(frame_buffer, x0, y0, x1, y1, x2, y2);
}

void draw_triangle_2(std::vector<sf::Uint8>& frame_buffer) {
    const float x0 = 264.0f;
    const float y0 = 0.0f;

    const float x1 = 358.6903545140f;
    const float y1 = 21.9999980926513671875f;

    const float x2 = 248.3387511207f;
    const float y2 = 22.0f;

    draw_filled_triangle(frame_buffer, x0, y0, x1, y1, x2, y2);
}

void draw_triangle_3(std::vector<sf::Uint8>& frame_buffer) {
    const float x0 = 12.75515676349f;
    const float y0 = 198.5f;

    const float x1 = 135.1306413607f;
    const float y1 = 56.7063726597f;

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

    const float x2 = 367.0f;
    const float y2 = 324.0f;

    draw_filled_triangle(frame_buffer, x0, y0, x1, y1, x2, y2);
}

void draw_triangle_6(std::vector<sf::Uint8>& frame_buffer) {
    const float x0 = 363.9982436391f;
    const float y0 = 159.6553802368f;

    const float x1 = 367.0f;
    const float y1 = 324.0f;

    const float x2 = 262.9930292912f;
    const float y2 = 375.789634232f;

    draw_filled_triangle(frame_buffer, x0, y0, x1, y1, x2, y2);
}

void draw_triangle_7(std::vector<sf::Uint8>& frame_buffer) {
    const float x0 = 367.0f;
    const float y0 = 324.0f;

    const float x1 = 262.9930292912f;
    const float y1 = 375.789634232f;

    const float x2 = 409.6016823266f;
    const float y2 = 451.2373621099f;

    draw_filled_triangle(frame_buffer, x0, y0, x1, y1, x2, y2);
}

void draw_triangle_8(std::vector<sf::Uint8>& frame_buffer) {
    const float x0 = 451.6123489859f;
    const float y0 = 276.3358111202f;

    const float x1 = 367.0f;
    const float y1 = 324.0f;

    const float x2 = 409.6016823266f;
    const float y2 = 451.2373621099f;

    draw_filled_triangle(frame_buffer, x0, y0, x1, y1, x2, y2);
}

void draw_triangle_9(std::vector<sf::Uint8>& frame_buffer) {
    const float x0 = 671.0229978627f;
    const float y0 = 18.3377211862f;

    const float x1 = 733.9925299673f;
    const float y1 = 150.6619830615f;

    const float x2 = 622.5835587075f;
    const float y2 = 150.9708484096f;

    draw_filled_triangle(frame_buffer, x0, y0, x1, y1, x2, y2);
}

void draw_triangle_10(std::vector<sf::Uint8>& frame_buffer) {
    const float x0 = 733.9925299673f;
    const float y0 = 150.6619830615f;

    const float x1 = 622.5835587075f;
    const float y1 = 150.9708484096f;

    const float x2 = 644.3167830776f;
    const float y2 = 233.7367509743f;

    draw_filled_triangle(frame_buffer, x0, y0, x1, y1, x2, y2);
}

void draw_triangle_11(std::vector<sf::Uint8>& frame_buffer) {
    const float x0 = 291.1065100094f;
    const float y0 = -37.3509609094f;

    const float x1 = 283.0f;
    const float y1 = 57.0f;

    const float x2 = 357.0f;
    const float y2 = 57.0f;

    draw_filled_triangle(frame_buffer, x0, y0, x1, y1, x2, y2);
}

void draw_triangle_12(std::vector<sf::Uint8>& frame_buffer) {
    const float x0 = 325.657620754f;
    const float y0 = -41.0f;

    const float x1 = 433.9619401172f;
    const float y1 = -41.0f;

    const float x2 = 388.3303817835f;
    const float y2 = 56.3531718472f;

    draw_filled_triangle(frame_buffer, x0, y0, x1, y1, x2, y2);
}

void draw_triangle_13(std::vector<sf::Uint8>& frame_buffer) {
    const float x0 = 471.9283607778f;
    const float y0 = -43.5498250651f;

    const float x1 = 431.1210373812f;
    const float y1 = -3.32132895079997f;

    const float x2 = 476.2695653944f;
    const float y2 = 55.7190538355f;

    draw_filled_triangle(frame_buffer, x0, y0, x1, y1, x2, y2);
}

void draw_triangle_14(std::vector<sf::Uint8>& frame_buffer) {
    const float x0 = 254.5909386425f;
    const float y0 = 509.69370691857f;

    const float x1 = 296.1611326583f;
    const float y1 = 639.0f;

    const float x2 = 372.0480753269f;
    const float y2 = 639.0f;

    draw_filled_triangle(frame_buffer, x0, y0, x1, y1, x2, y2);
}

void draw_triangle_15(std::vector<sf::Uint8>& frame_buffer) {
    const float x0 = 342.8840329319f;
    const float y0 = 501.35399949222f;

    const float x1 = 350.2591925953f;
    const float y1 = 600.0f;

    const float x2 = 434.0410063713f;
    const float y2 = 600.0f;

    draw_filled_triangle(frame_buffer, x0, y0, x1, y1, x2, y2);
}

void draw_triangle_16(std::vector<sf::Uint8>& frame_buffer) {
    const float x0 = 418.8752339453f;
    const float y0 = 532.15272658636f;

    const float x1 = 530.5869046584f;
    const float y1 = 532.15272658636f;

    const float x2 = 507.0256567422f;
    const float y2 = 638.70849180515f;

    draw_filled_triangle(frame_buffer, x0, y0, x1, y1, x2, y2);
}

void draw_triangle_17(std::vector<sf::Uint8>& frame_buffer) {
    const float x0 = 581.4790364904f;
    const float y0 = 533.27173635797f;

    const float x1 = 548.8692108751f;
    const float y1 = 615.07741958214f;

    const float x2 = 592.7238039439f;
    const float y2 = 643.18933821587f;

    draw_filled_triangle(frame_buffer, x0, y0, x1, y1, x2, y2);
}

void draw_triangle_18(std::vector<sf::Uint8>& frame_buffer) {
    const float x0 = 606.8521389828f;
    const float y0 = 534.03424793148f;

    const float x1 = 605.8312534277f;
    const float y1 = 599.0f;

    const float x2 = 632.9120444122f;
    const float y2 = 609.1123103102f;

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
    // NOTE: Проверяем на точное равенство, а не через epsilon, поскольку в противном случае
    // возможна некорректная растеризация: пропуск пиксела или двойная растеризация одного и того же
    // пиксела для двух смежных треугольников.
    const bool flat_bottom = (y1 == y2);
    const bool flat_top = (y0 == y1);
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

    // Клиппинг с верхней границей экрана.
    int y_start = 0;
    if (y0 < 0.0f) {
        // Корректируем начало и конец первой скан-линии.
        const float clip_height = 0.0f - y0;
        scan_line_start = scan_line_start + clip_height * slope_left_inv;
        scan_line_end = scan_line_end + clip_height * slope_right_inv;
    }
    else {
        // Определяем y-координату первой скан-линии (следуем правилу top-left).
        y_start = std::ceil(y0);

        // Корректируем начало и конец первой скан-линии.
        const float delta_y = y_start - y0;
        scan_line_start = scan_line_start + delta_y * slope_left_inv;
        scan_line_end = scan_line_end + delta_y * slope_right_inv;
    }

    // Определяем y-координату последней скан-линии (следуем правилу top-left).
    // NOTE: Если для текущего flat_bottom треугольника существует смежный flat_top,
    // то, поскольку последняя скан-линия flat_bottom треугольника определяется по правой нижней координате,
    // то первая скан-линия смежного flat_top треугольника должна определяться по правой верхней,
    // чтобы не было ни пропуска скан-линии ни наложения.
    int y_end = std::ceil(y2) - 1;
    if (y2 > h) {
        y_end = h - 1;
    }

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

    // Клиппинг с верхней границей экрана.
    // NOTE: Чтобы быть последовательными, ориентируемся на правую вершину, поскольку по ней определяем
    // высоту и по ней же определяем y-координату первой скан-линии. Более того, по правой же вершине определяем
    // для flat_bottom треугольника y-координату последней скан-линии.
    // 
    // Но фактически, в случае, когда y-координаты верхних вершин различаются на величину меньше epsilon,
    // нет разницы, на какую вершину ориентироваться, разница будет лишь в том, какой slope будет вычислен с
    // большей погрешность: левый или правый, что в свою очередь определяет, какая часть первой скан-линии
    // будет вычислена с большей погрешностью - левая или правая.
    int y_start = 0;
    if (y1 < 0.0f) {
        // Корректируем начало и конец первой скан-линии.
        const float clip_height = 0.0f - y1;
        scan_line_start = scan_line_start + clip_height * slope_left_inv;
        scan_line_end = scan_line_end + clip_height * slope_right_inv;
    }
    else {
        // Первую скан-линию определяем по правой верхней вершине, как описано в замечании
        // к растеризации flat_bottom треугольника.
        y_start = std::ceil(y1);

        // Корректируем начало и конец первой скан-линии.
        const float delta_y = y_start - y1;
        scan_line_start = scan_line_start + delta_y * slope_left_inv;
        scan_line_end = scan_line_end + delta_y * slope_right_inv;
    }

    int y_end = std::ceil(y2) - 1;
    if (y2 > h) {
        y_end = h - 1;
    }

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
