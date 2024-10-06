#include <iostream>
#include <format>
#include <chrono>
#include <thread>

#include <SFML/Graphics.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Vec2i.h"

constexpr int w = 1024;
constexpr int h = 768;

std::vector<int> interpolate_x(Vec2i a, Vec2i b);
void draw_line(Vec2i a, Vec2i b, sf::VertexArray& frame_buffer, const sf::Color& color);
void draw_triangle(Vec2i a, Vec2i b, Vec2i c, sf::VertexArray& frame_buffer, const sf::Color& color, bool filled = false);
bool import_model_and_draw(const std::string& modelpath, sf::VertexArray& frame_buffer);

sf::RenderWindow* p_win;

int main()
{
    sf::RenderWindow window(sf::VideoMode(w, h), "Software Renderer");
    p_win = &window;
    sf::VertexArray frame_buffer(sf::Points, w * h);

    //import_model_and_draw("data/rubber_duck/scene.gltf", frame_buffer);

    Vec2i t0[3] = { Vec2i(10, 70),   Vec2i(50, 160),  Vec2i(70, 80) };
    draw_triangle(t0[0], t0[1], t0[2], frame_buffer, sf::Color::Red, true);
    draw_triangle(t0[0], t0[1], t0[2], frame_buffer, sf::Color::Red);

    Vec2i t1[3] = { Vec2i(180, 50),  Vec2i(150, 1),   Vec2i(70, 180) };
    draw_triangle(t1[0], t1[1], t1[2], frame_buffer, sf::Color::White, true);
    draw_triangle(t1[0], t1[1], t1[2], frame_buffer, sf::Color::White);

    Vec2i t2[3] = { Vec2i(180, 150), Vec2i(120, 160), Vec2i(130, 180) };
    draw_triangle(t2[0], t2[1], t2[2], frame_buffer, sf::Color::Green, true);
    draw_triangle(t2[0], t2[1], t2[2], frame_buffer, sf::Color::Green);

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        //window.clear();
        //window.draw(frame_buffer);
        //window.display();
    }

    return 0;
}

std::vector<int> interpolate_x(Vec2i a, Vec2i b)
{
    std::vector<int> x_coords;

    const int total_height = b.y - a.y;
    for (int y = a.y; y <= b.y; y++)
    {
        float y_way_percent = (y - a.y) / (float)total_height;
        const int x = a.x + (b.x - a.x) * y_way_percent;
        x_coords.push_back(x);
    }

    // TODO: Посмотреть, как повлияет на производительность std::move().
    return x_coords;
}

void draw_line(Vec2i a, Vec2i b, sf::VertexArray& frame_buffer, const sf::Color& color) {
    if (a.x == b.x && a.y == b.y)
    {
        std::cout << std::format("Line drawing from [{},{}] to [{},{}]\n", a.x, a.y, b.x, b.y);

        sf::Vector2f pos(a.x, a.y);
        int index = w * a.y + a.x; // Раскладываем строки буфера в горизонтальную линию.

        pos.y = h - pos.y;
        frame_buffer[index].position = pos;
        frame_buffer[index].color = color;

        p_win->draw(frame_buffer);
        p_win->display();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        return;
    }

    bool swapped = false;
    if (std::abs(a.x - b.x) < std::abs(a.y - b.y))
    {
        std::swap(a.x, a.y);
        std::swap(b.x, b.y);
        swapped = true;
    }

    if (a.x > b.x)
    {
        std::swap(a.x, b.x);
        std::swap(a.y, b.y);
    }

    if (swapped)
        std::cout << std::format("Line drawing from [{},{}] to [{},{}]\n", a.y, a.x, b.y, b.x);
    else
        std::cout << std::format("Line drawing from [{},{}] to [{},{}]\n", a.x, a.y, b.x, b.y);

    for (int x = a.x; x <= b.x; x++)
    {
        float x_way_percent = (x - a.x) / (float)(b.x - a.x);
        int y = (b.y - a.y) * x_way_percent + a.y;

        sf::Vector2f pos(x, y);
        int index = w * y + x; // Раскладываем строки буфера в горизонтальную линию.

        if (swapped)
        {
            pos.x = y;
            pos.y = x;

            index = w * x + y; // Был своп координат. Меняем, чтобы не перепутать строки со столбцами.
        }

        pos.y = h - pos.y; // В виртуальной СК начало координат в левом нижнем углу, а в мировой - в левом верхнем.
        frame_buffer[index].position = pos;
        frame_buffer[index].color = color;

        p_win->draw(frame_buffer);
        p_win->display();
        //std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

bool import_model_and_draw(const std::string& modelpath, sf::VertexArray& frame_buffer)
{
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(modelpath,
        aiProcess_CalcTangentSpace |
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_SortByPType);

    if (scene == nullptr)
    {
        std::cout << importer.GetErrorString() << std::endl;
        return false;
    }

    const aiMesh* mesh = scene->mMeshes[0];
    for (unsigned int i = 0; i != mesh->mNumFaces; i++)
    {
        const aiFace& face = mesh->mFaces[i];
        const unsigned int idx[3] = { face.mIndices[0], face.mIndices[1], face.mIndices[2] };
        for (int j = 0; j != 3; j++)
        {
            const aiVector3D va = mesh->mVertices[idx[j]];
            const aiVector3D vb = mesh->mVertices[idx[(j + 1) % 3]];

            //std::cout << std::format("va({},{},{})", va.x, va.y, va.z) << std::endl;
            //std::cout << std::format("vb({},{},{})", vb.x, vb.y, vb.z) << std::endl;

            const int a_screen_x = (va.x + 1.0f) * (w / 2);
            const int a_screen_y = va.z * (h / 2);
            //const int a_screen_y = (va.z * h) - 1;
            //std::cout << std::format("a_screen({},{})", a_screen_x, a_screen_y) << std::endl;

            //std::cout << std::endl;

            const int b_screen_x = (vb.x + 1.0f) * (w / 2);
            const int b_screen_y = vb.z * (h / 2);
            //const int b_screen_y = (vb.z * h) - 1;
            //std::cout << std::format("b_screen({},{})", b_screen_x, b_screen_y) << std::endl;

            Vec2i a(a_screen_x, a_screen_y);
            Vec2i b(b_screen_x, b_screen_y);
            draw_line(a, b, frame_buffer, sf::Color::White);
        }
    }

    return true;
}

void draw_triangle(Vec2i a, Vec2i b, Vec2i c, sf::VertexArray& frame_buffer, const sf::Color& color, bool filled)
{
    if (!filled)
    {
        draw_line(a, b, frame_buffer, color);
        draw_line(b, c, frame_buffer, color);
        draw_line(c, a, frame_buffer, color);
        return;
    }

    // a=min(a.y,b.y,c.y).
    // c=max(a.y,b.y,c.y).
    if (a.y > b.y) std::swap(a, b);
    if (a.y > c.y) std::swap(a, c);
    if (b.y > c.y) std::swap(b, c);

    // Координаты x сторон треугольника.
    const std::vector<int> x_coords_ac = interpolate_x(a, c);
    std::cout << "count(ac): " << x_coords_ac.size() << std::endl;

    // Последний элемент ab равен первому элементу bc - удаляем его,
    // чтобы размеры ac и ab+bc были равны.
    std::vector<int> x_coords_ab = interpolate_x(a, b);
    x_coords_ab.pop_back();
    const std::vector<int> x_coords_bc = interpolate_x(b, c);
    std::cout << "count(ab+bc): " << x_coords_ab.size() + x_coords_bc.size() << std::endl;
    std::vector<int> x_coords_abc = x_coords_ab; // TODO: std::move().
    x_coords_abc.insert(x_coords_abc.end(), x_coords_bc.begin(), x_coords_bc.end());

    for (const auto& x : x_coords_abc)
    {
        std::cout << x << std::endl;
    }

    int triangle_height = c.y - a.y;
    std::cout << triangle_height << std::endl;

    for (int i = 0; i < triangle_height + 1; i++) {
        const int x_1 = x_coords_ac[i];
        const int x_2 = x_coords_abc[i];
        Vec2i p1(x_1, i + a.y);
        Vec2i p2(x_2, i + a.y);
        draw_line(p1, p2, frame_buffer, color);
    }
}