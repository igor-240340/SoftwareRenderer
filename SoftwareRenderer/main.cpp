#include <iostream>
#include <format>
#include <chrono>
#include <thread>
#include <random>
#include <array>
#include <limits>
#include <numbers>

#include <SFML/Graphics.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Vec2i.h"
#include "Vec3f.h"
#include "Vertex.h"
#include "Mat4f.h"

constexpr int w = 1024;
constexpr int h = 768;

std::vector<int> interpolate_x(Vec2i a, Vec2i b);
void draw_line(Vec2i a, Vec2i b, sf::VertexArray& frame_buffer, float intensity, Vertex ta, Vertex tb, Vertex tc);
void draw_triangle(Vertex a, Vertex b, Vertex c, sf::VertexArray& frame_buffer, float intensity, bool filled = false);
bool draw_rotate(const aiMesh* mesh, sf::VertexArray& frame_buffer, float angle_deg);
float get_depth_for_fragment(int coord_x, int coord_y, Vec3f ta, Vec3f tb, Vec3f tc);
void interpolate_uv(int fragment_coord_x, int fragment_coord_y, Vertex a, Vertex b, Vertex c, float& u, float& v);

//std::random_device random_device;
//std::mt19937 engine(random_device());
//std::uniform_int_distribution<> distrib(0, 255);

sf::RenderWindow* p_win;

std::array<float, w* h> depth_buffer;

sf::Image* p_image;

int main()
{
    depth_buffer.fill(-std::numeric_limits<float>::max());

    sf::RenderWindow window(sf::VideoMode(w, h), "Software Renderer");
    p_win = &window;
    sf::VertexArray frame_buffer(sf::Points, w * h);
    for (int i = 0; i < w * h; i++)
    {
        frame_buffer[i].color = sf::Color::White;
    }

    sf::Texture texture;
    //if (!texture.loadFromFile("data/boxer/textures/material_0_baseColor.jpeg"))
    //if (!texture.loadFromFile("data/african_head_diffuse.tga"))
    //if (!texture.loadFromFile("data/box_textured/textures/uv_map.jpg"))
        //if (!texture.loadFromFile("data/rubber_duck/textures/Duck_baseColor.png"))
        if (!texture.loadFromFile("data/pony_cartoon/textures/Body_SG1_baseColor.jpeg"))
        //if (!texture.loadFromFile("data/prime1_studios_joker/textures/tex_u1_v1_baseColor.jpeg"))
    {
        std::cerr << "Failed to load texture" << std::endl;
        return -1;
    }
    sf::Image image = texture.copyToImage();
    p_image = &image;

    //import_model_and_draw("data/rubber_duck/scene.gltf", frame_buffer);
    //import_model_and_draw("data/pony_cartoon/scene.gltf", frame_buffer);
    //import_model_and_draw("data/prime1_studios_joker/scene.gltf", frame_buffer);
    //import_model_and_draw("data/african_head.obj", frame_buffer);
    //import_model_and_draw("data/wheelretopopbr/scene.gltf", frame_buffer);

    /*
        Vec2i t0[3] = { Vec2i(10, 70),   Vec2i(50, 160),  Vec2i(70, 80) };
        draw_triangle(t0[0], t0[1], t0[2], frame_buffer, sf::Color::Red, true);
        draw_triangle(t0[0], t0[1], t0[2], frame_buffer, sf::Color::Red);

        Vec2i t1[3] = { Vec2i(180, 50),  Vec2i(150, 1),   Vec2i(70, 180) };
        draw_triangle(t1[0], t1[1], t1[2], frame_buffer, sf::Color::White, true);
        draw_triangle(t1[0], t1[1], t1[2], frame_buffer, sf::Color::White);

        Vec2i t2[3] = { Vec2i(180, 150), Vec2i(120, 160), Vec2i(130, 180) };
        draw_triangle(t2[0], t2[1], t2[2], frame_buffer, sf::Color::Green, true);
        draw_triangle(t2[0], t2[1], t2[2], frame_buffer, sf::Color::Green);
    */

    // Импорт меша.
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile("data/pony_cartoon/scene.gltf",
    //const aiScene* scene = importer.ReadFile("data/box_textured/scene.gltf",
    //const aiScene* scene = importer.ReadFile("data/pony_cartoon/scene.gltf",
    //const aiScene* scene = importer.ReadFile("data/boxer/scene.gltf",
    //const aiScene* scene = importer.ReadFile("data/rubber_duck/scene.gltf",
        aiProcess_CalcTangentSpace |
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_SortByPType |
        aiProcess_GenBoundingBoxes |
        aiProcess_PreTransformVertices);
    const aiAABB& aabb = scene->mMeshes[0]->mAABB;
    if (scene == nullptr)
    {
        std::cout << importer.GetErrorString() << std::endl;
        return false;
    }
    const aiMesh* mesh = scene->mMeshes[0];

    float angle = 0.0f;
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear(sf::Color::White);

        depth_buffer.fill(-std::numeric_limits<float>::max());
        for (int i = 0; i < w * h; i++)
        {
            frame_buffer[i].color = sf::Color::White;
        }

        draw_rotate(mesh, frame_buffer, angle += 1.5f);

        window.draw(frame_buffer);
        window.display();
    }

    return 0;
}

std::vector<int> interpolate_x(Vec2i a, Vec2i b)
{
    std::vector<int> x_coords;

    if (a.y == b.y)
        return x_coords;

    const int total_height = b.y - a.y;
    for (int y = a.y; y <= b.y; y++)
    {
        float y_way_percent = (y - a.y) / (float)total_height;
        //std::cout << "y_way_percent: " << y_way_percent << std::endl;
        const int x = a.x + (b.x - a.x) * y_way_percent;
        x_coords.push_back(x);
        //std::cout << "X~~: " << x << std::endl;
    }

    // TODO: Посмотреть, как повлияет на производительность std::move().
    return x_coords;
}

void draw_line(Vec2i a, Vec2i b, sf::VertexArray& frame_buffer, float intensity, Vertex ta, Vertex tb, Vertex tc) {
    if (a.x == b.x && a.y == b.y)
    {
        //std::cout << std::format("Line drawing from [{},{}] to [{},{}]\n", a.x, a.y, b.x, b.y);

        sf::Vector2f pos(a.x, a.y);
        int index = w * a.y + a.x; // Раскладываем строки буфера в горизонтальную линию.

        int source_fragment_coord_x = a.x;
        int source_fragment_coord_y = a.y;
        float source_fragment_depth = get_depth_for_fragment(source_fragment_coord_x, source_fragment_coord_y, ta.pos, tb.pos, tc.pos);
        float target_fragment_depth = depth_buffer.at(index);
        if (source_fragment_depth > target_fragment_depth)
        {
            float u;
            float v;
            interpolate_uv(source_fragment_coord_x, source_fragment_coord_y, ta, tb, tc, u, v);
            int tex_width = p_image->getSize().x;
            int tex_height = p_image->getSize().y;

            int tex_u = static_cast<int>(u * (tex_width - 1));
            int tex_v = static_cast<int>((1.0f - v) * (tex_height - 1));
            if (tex_v < 0) tex_v = 0; // FIXME: Выяснить.
            if (tex_v > tex_height - 1) tex_v = tex_height - 1; // FIXME: Выяснить.

            sf::Color tex_color = p_image->getPixel(tex_u, tex_v);
            tex_color = sf::Color(tex_color.r * intensity, tex_color.g * intensity, tex_color.b * intensity);

            pos.y = h - pos.y;
            frame_buffer[index].position = pos;
            frame_buffer[index].color = tex_color;
            depth_buffer[index] = source_fragment_depth;
        }

        //p_win->draw(frame_buffer);
        //p_win->display();
        //std::this_thread::sleep_for(std::chrono::milliseconds(100));
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

    /*if (swapped)
        std::cout << std::format("Line drawing from [{},{}] to [{},{}]\n", a.y, a.x, b.y, b.x);
    else
        std::cout << std::format("Line drawing from [{},{}] to [{},{}]\n", a.x, a.y, b.x, b.y);*/

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

        int source_fragment_coord_x = x;
        int source_fragment_coord_y = y;
        float source_fragment_depth = get_depth_for_fragment(source_fragment_coord_x, source_fragment_coord_y, ta.pos, tb.pos, tc.pos);
        float target_fragment_depth = depth_buffer.at(index);
        if (source_fragment_depth > target_fragment_depth)
        {
            float u;
            float v;
            interpolate_uv(source_fragment_coord_x, source_fragment_coord_y, ta, tb, tc, u, v);
            int tex_width = p_image->getSize().x;
            int tex_height = p_image->getSize().y;
            int tex_u = static_cast<int>(u * (tex_width - 1));
            int tex_v = static_cast<int>((1.0f - v) * (tex_height - 1));
            if (tex_v < 0) tex_v = 0; // FIXME: Выяснить.
            if (tex_v > tex_height - 1) tex_v = tex_height - 1; // FIXME: Выяснить.
            //if (tex_v > tex_width)
                //tex_v = tex_width-1;

            //std::cout << std::format("tex_u: {}, tex_v: {}\n", tex_u, tex_v);

            sf::Color tex_color = p_image->getPixel(tex_u, tex_v);
            tex_color = sf::Color(tex_color.r * intensity, tex_color.g * intensity, tex_color.b * intensity);

            pos.y = h - pos.y;
            frame_buffer[index].position = pos;
            frame_buffer[index].color = tex_color;
            //frame_buffer[index].color = color;
            depth_buffer[index] = source_fragment_depth;
        }

        //p_win->draw(frame_buffer);
        //p_win->display();
        //std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

bool draw_rotate(const aiMesh* mesh, sf::VertexArray& frame_buffer, float angle_deg)
{
    Mat4f persp_proj = Mat4f::create_perspective(50.0 * (std::numbers::pi / 180.0), w / (float)h, -1.0f, -50.0f);
    Mat4f view = Mat4f::create_viewport(w, h);
    Mat4f translation = Mat4f::create_translation(Vec3f{ 0.0f, -0.7f, -5.0f });
    //Mat4f rotation_x = Mat4f::create_rotation_x(angle_deg * (std::numbers::pi / 180.0));
    Mat4f rotation_y = Mat4f::create_rotation_y(angle_deg * (std::numbers::pi / 180.0));
    Mat4f rotation_z = Mat4f::create_rotation_z(angle_deg * (std::numbers::pi / 180.0));
    Mat4f rotation_x = Mat4f::create_rotation_x(0 * (std::numbers::pi / 180.0));
    //Mat4f rotation_y = Mat4f::create_rotation_y(0 * (std::numbers::pi / 180.0));
    //Mat4f rotation_z = Mat4f::create_rotation_z(0 * (std::numbers::pi / 180.0));

    for (unsigned int i = 0; i != mesh->mNumFaces; i++)
    {
        const aiFace& face = mesh->mFaces[i];
        const unsigned int idx[3] = { face.mIndices[0], face.mIndices[1], face.mIndices[2] };
        std::vector<Vertex> triangle_vertices;
        std::vector<Vec3f> triangle_vertices_orig;
        for (int j = 0; j != 3; j++)
        {
            const aiVector3D va = mesh->mVertices[idx[j]];

            Vec4f v4{ va.x, va.y, va.z };

            // Сначала translation, потом rotation.
            //Vec4f translated = translation * v4;
            //Vec4f rotated = rotation_z * translated;
            //Vec4f in_clip_space = persp_proj * rotated;

            // Сначала rotation, потом translation.
            Vec4f rotated = rotation_x * v4;
            rotated = rotation_y * rotated;
            rotated = rotation_z * rotated;
            Vec4f translated = translation * rotated;

            Vec3f tranformed_orig = translated;
            triangle_vertices_orig.push_back(Vec3f(tranformed_orig.x, tranformed_orig.y, tranformed_orig.z));

            Vec4f in_clip_space = persp_proj * translated;
            Vec4f in_ndc = in_clip_space / in_clip_space.w;
            Vec3f in_screen = view * in_ndc;

            aiVector3D uv = mesh->mTextureCoords[0][idx[j]];
            float u = uv.x;
            float v = uv.y;

            Vertex vt{ in_screen, u, v };
            triangle_vertices.push_back(vt);
        }

        Vec3f vector_a = triangle_vertices_orig[1] - triangle_vertices_orig[0];
        Vec3f vector_b = triangle_vertices_orig[2] - triangle_vertices_orig[0];
        Vec3f face_normal = Vec3f::cross(vector_a, vector_b).normalized();
        Vec3f light_dir(0.0f, 0.0f, -1.0f);
        float intensity = Vec3f::dot(face_normal, light_dir);
        if (intensity < 0)
        {
            draw_triangle(triangle_vertices[0], triangle_vertices[1], triangle_vertices[2], frame_buffer, std::abs(intensity), true);
        }
    }

    return true;
}

float get_depth_for_fragment(int coord_x, int coord_y, Vec3f ta, Vec3f tb, Vec3f tc)
{
    /*coord_x = 752;
    coord_y = 1280;

    ta.x = 400;
    ta.y = 212;
    ta.z = 0;

    tb.x = 2408;
    tb.y = 2000;
    tb.z = 0;

    tc.x = 400;
    tc.y = 2000;
    tc.z = 1.69f;*/

    float total_area = ((tc.x - ta.x) * (tb.y - ta.y) - (tb.x - ta.x) * (tc.y - ta.y)) / 2;
    float a_u = ((tc.x - ta.x) * (coord_y - ta.y) - (tc.y - ta.y) * (coord_x - ta.x)) / 2;
    float a_v = ((coord_x - ta.x) * (tb.y - ta.y) - (coord_y - ta.y) * (tb.x - ta.x)) / 2;

    float u = a_u / total_area;
    float v = a_v / total_area;
    float w = 1 - u - v;

    return w * ta.z + u * tb.z + v * tc.z;
}

void interpolate_uv(int fragment_coord_x, int fragment_coord_y, Vertex a, Vertex b, Vertex c, float& u, float& v)
{
    float total_area = ((c.pos.x - a.pos.x) * (b.pos.y - a.pos.y) - (b.pos.x - a.pos.x) * (c.pos.y - a.pos.y)) / 2;
    float a_u = ((c.pos.x - a.pos.x) * (fragment_coord_y - a.pos.y) - (c.pos.y - a.pos.y) * (fragment_coord_x - a.pos.x)) / 2;
    float a_v = ((fragment_coord_x - a.pos.x) * (b.pos.y - a.pos.y) - (fragment_coord_y - a.pos.y) * (b.pos.x - a.pos.x)) / 2;

    // TODO: Разобраться с барицентрическими координатами. Разобраться со знаками площадей.
    total_area = std::abs(total_area);
    a_u = std::abs(a_u);
    a_v = std::abs(a_v);

    float _u = a_u / total_area;
    float _v = a_v / total_area;
    float w = 1 - _u - _v;

    u = w * a.u + _u * b.u + _v * c.u;
    v = w * a.v + _u * b.v + _v * c.v;

    // FIXME: В некоторых случаях площадь a_v оказывается больше площади total_area из-за чего получается отрицательный знак.
    // Более того, u,v оказываются больше единицы.
    u = std::abs(u);
    v = std::abs(v);
    if (u > 1)
        u = 1;
    if (v > 1)
        v = 1;
}

void draw_triangle(Vertex a, Vertex b, Vertex c, sf::VertexArray& frame_buffer, float intensity, bool filled)
{
    if (!filled)
    {
        /*draw_line(a, b, frame_buffer, color);
        draw_line(b, c, frame_buffer, color);
        draw_line(c, a, frame_buffer, color);*/
        return;
    }

    // a=min(a.y,b.y,c.y).
    // c=max(a.y,b.y,c.y).
    if (a.pos.y > b.pos.y) std::swap(a, b);
    if (a.pos.y > c.pos.y) std::swap(a, c);
    if (b.pos.y > c.pos.y) std::swap(b, c);

    // Координаты x сторон треугольника.
    const std::vector<int> x_coords_ac = interpolate_x(Vec2i(a.pos.x, a.pos.y), Vec2i(c.pos.x, c.pos.y));
    //std::cout << "count(ac): " << x_coords_ac.size() << std::endl;

    // Если треугольник выродился в горизонтальную линию.
    if (x_coords_ac.empty())
    {
        //std::cout << std::format("smth wrong with a[{},{}], b[{},{}], c[{},{}]", a.x, a.y, b.x, b.y, c.x, c.y) << std::endl;
        return;
    }

    // Последний элемент ab равен первому элементу bc - удаляем его,
    // чтобы размеры ac и ab+bc были равны.
    std::vector<int> x_coords_ab = interpolate_x(Vec2i(a.pos.x, a.pos.y), Vec2i(b.pos.x, b.pos.y));
    const std::vector<int> x_coords_bc = interpolate_x(Vec2i(b.pos.x, b.pos.y), Vec2i(c.pos.x, c.pos.y));

    // Если ни одна линия не горизонтальная, то обе дают иксы, причем конец ab дублируется в начале bc.
    // В противном случае один из массивов будет пустым, а второй будет целиком формировать иксы.
    if (!x_coords_ab.empty() && !x_coords_bc.empty())
        x_coords_ab.pop_back();

    std::vector<int> x_coords_abc = x_coords_ab; // TODO: std::move().
    x_coords_abc.insert(x_coords_abc.end(), x_coords_bc.begin(), x_coords_bc.end());

    for (const auto& x : x_coords_abc)
    {
        //std::cout << x << std::endl;
    }

    int triangle_height = c.pos.y - a.pos.y;
    //std::cout << triangle_height << std::endl;

    for (int i = 0; i < triangle_height + 1; i++)
    {
        const int x_1 = x_coords_ac[i];
        const int x_2 = x_coords_abc[i];
        Vec2i p1(x_1, i + a.pos.y);
        Vec2i p2(x_2, i + a.pos.y);

        draw_line(p1, p2, frame_buffer, intensity, a, b, c);
    }
}