#include <iostream>
#include <format>

#include <SFML/Graphics.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

constexpr int w = 1024;
constexpr int h = 768;

void draw_line(int x0, int y0, int x1, int y1, sf::VertexArray& frame_buffer, const sf::Color& color);
bool import_model_and_draw(const std::string& modelpath, sf::VertexArray& frame_buffer);

int main()
{
    sf::RenderWindow window(sf::VideoMode(w, h), "Software Renderer From Scratch");

    sf::VertexArray frame_buffer(sf::Points, w * h);

    import_model_and_draw("data/rubber_duck/scene.gltf", frame_buffer);

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear();
        window.draw(frame_buffer);
        window.display();
    }

    return 0;
}

void draw_line(int x0, int y0, int x1, int y1, sf::VertexArray& frame_buffer, const sf::Color& color) {
    bool swapped = false;
    if (std::abs(x0 - x1) < std::abs(y0 - y1))
    {
        std::swap(x0, y0);
        std::swap(x1, y1);
        swapped = true;
    }

    if (x0 > x1)
    {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    for (int x = x0; x <= x1; x++)
    {
        float x_way_percent = (x - x0) / (float)(x1 - x0);
        int y = (y1 - y0) * x_way_percent + y0;

        sf::Vector2f pos(x, y);
        int index = w * y + x; // Раскладываем строки буфера в горизонтальную линию.

        if (swapped)
        {
            pos.x = y;
            pos.y = x;

            index = w * x + y; // Был своп координат. Меняем, чтобы не перепутать строки со столбцами.
        }

        frame_buffer[index].position = pos;
        frame_buffer[index].color = color;
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

            draw_line(a_screen_x, (h - a_screen_y) - 1.0f, b_screen_x, (h - b_screen_y) - 1.0f, frame_buffer, sf::Color::White);
        }
    }

    return true;
}
