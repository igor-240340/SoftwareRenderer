#include <iostream>
#include <format>
#include <chrono>
#include <thread>
#include <random>
#include <array>
#include <limits>
#include <numbers>
#include <bitset>

#include <SFML/Graphics.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Vec2i.h"
#include "Vec3f.h"
#include "Vertex.h"
#include "Mat4f.h"
#include "Camera.h"
#include "Vertex4.h"
#include "Polygon.h"

constexpr int w = 1024;
constexpr int h = 768;

void print_float_as_hex(float* a) {
    uint32_t int_rep = *(uint32_t*)a;
    std::cout << std::hex << std::setfill('0') << std::setw(8) << int_rep << std::endl;
}

std::vector<int> interpolate_x(Vec2i a, Vec2i b);
void draw_line(Vec2i a, Vec2i b, sf::VertexArray& frame_buffer, float intensity, Vertex ta, Vertex tb, Vertex tc);
void draw_triangle(Vertex a, Vertex b, Vertex c, sf::VertexArray& frame_buffer, float intensity, bool filled = false);
bool draw_rotate(const aiMesh* mesh, sf::VertexArray& frame_buffer, float angle_deg);
float get_depth_for_fragment(int coord_x, int coord_y, Vec3f ta, Vec3f tb, Vec3f tc);
void interpolate_uv(int fragment_coord_x, int fragment_coord_y, Vertex a, Vertex b, Vertex c, float& u, float& v);

void draw_mesh(const aiMesh* mesh, sf::VertexArray& frame_buffer, float angle_deg);
bool is_backfaced(const Polygon& polygon_in_cam_space);
void draw_line_color(Vec2i a, Vec2i b, sf::VertexArray& frame_buffer, const sf::Color& color);

void draw_mesh_(const aiMesh* mesh, std::vector<sf::Uint8>& frame_buffer, const Vec3f& rotation, const Vec3f& translation);
void draw_triangle_(std::vector<sf::Uint8>& frame_buffer, Vertex a, Vertex b, Vertex c);
void draw_line_dda(std::vector<sf::Uint8>& frame_buffer, int x0, int y0, int x1, int y1, sf::Color color);
bool clip_line_coh_suth(float& x0, float& y0, float& x1, float& y1);
void set_pixel_color(std::vector<sf::Uint8>& frame_buffer, int x, int y, sf::Color color);
void fill_frame_buffer(std::vector<sf::Uint8>& frame_buffer, sf::Color color);

//std::random_device random_device;
//std::mt19937 engine(random_device());
//std::uniform_int_distribution<> distrib(0, 255);

sf::RenderWindow* p_win;

std::array<float, w* h> depth_buffer;

sf::Image* p_image;

Camera cam{
    Vec3f(0.0f, 0.0f, 2.4142f), // У модели в GeoGebra позиция камеры выбрана не вполне удачно - не в нуле.
    (90.0 + 0.0) * (std::numbers::pi / 180.0),
    (90.0 + 0.0) * (std::numbers::pi / 180.0)
};

//Camera cam{
//    Vec3f(1.0f, 2.0f, 2.0f),
//    (90.0 + 10.0) * (std::numbers::pi / 180.0),
//    (90.0 + 45.0) * (std::numbers::pi / 180.0)
//};

//Camera cam{
//    Vec3f(2.0f, 0.0f, -3.0f),
//    (180.0 + 90.0) * (std::numbers::pi / 180.0),
//    (90.0) * (std::numbers::pi / 180.0)
//};

//Camera cam{
//    Vec3f(-6.0f, 4.0f, 2.0f),
//    106.7194 * (std::numbers::pi / 180.0),
//    109.7616 * (std::numbers::pi / 180.0)
//};

int main() {
    sf::RenderWindow window(sf::VideoMode(w, h), "Software Renderer");

    std::vector<sf::Uint8> frame_buffer(w * h * 4);
    fill_frame_buffer(frame_buffer, sf::Color::White);

    sf::Texture texture;
    if (!texture.create(w, h)) {
        std::cout << "SFML: Create texture fail.\n";
    }

    sf::Sprite sprite(texture);

    Assimp::Importer importer;
    //const aiScene* scene = importer.ReadFile("data/box_textured/scene.gltf",
    //const aiScene* scene = importer.ReadFile("data/fuel_barrel/scene.gltf",
    //const aiScene* scene = importer.ReadFile("data/fuel_barrel/scene.gltf",
    const aiScene* scene = importer.ReadFile("data/pony_cartoon/scene.gltf",
        aiProcess_CalcTangentSpace
        | aiProcess_Triangulate
        | aiProcess_JoinIdenticalVertices
        | aiProcess_SortByPType
        | aiProcess_GenBoundingBoxes
        | aiProcess_PreTransformVertices);
    const aiAABB& aabb = scene->mMeshes[0]->mAABB;
    if (scene == nullptr) {
        std::cout << importer.GetErrorString() << std::endl;
        return false;
    }
    const aiMesh* mesh = scene->mMeshes[0];

    float angle = 0.0f;
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        fill_frame_buffer(frame_buffer, sf::Color::White);

        angle += 0.01f;
        //draw_mesh_(mesh, frame_buffer, Vec3f::zero, Vec3f::zero);

        //draw_mesh_(mesh, frame_buffer, Vec3f(0.0f, 0.0f, 0.0f), Vec3f(-6.0f, 0.0f, 0.0f));
        // Слегка повернули, чтобы боковые не отсеклись по backface culling.
        //draw_mesh_(mesh, frame_buffer, Vec3f(0.0f, -0.5f, 0.0f), Vec3f(-5.0f, 0.0f, -5.5f));
        //draw_mesh_(mesh, frame_buffer, Vec3f(0.0f, 0.5f, 0.0f), Vec3f(5.0f, 0.0f, -5.5f));
        //draw_mesh_(mesh, frame_buffer, Vec3f(0.0f, 0.0f, 0.0f), Vec3f(0.0f, 3.0f, -5.5f));
        //draw_mesh_(mesh, frame_buffer, Vec3f(0.0f, 0.0f, 0.0f), Vec3f(0.0f, -3.0f, -5.5f));
        //draw_mesh_(mesh, frame_buffer, Vec3f(0.0f, angle * 30.0f, 0.0f), Vec3f(0.0f, std::sin(angle) * 4.5f, -6.0f));
        draw_mesh_(mesh, frame_buffer, Vec3f(angle * 30.0f, 0.0f, 0.0f), Vec3f(std::sin(angle) * 5.0f, -2.5f, -6.0f));
        //draw_mesh_(mesh, frame_buffer, Vec3f(25.0f, 10.0f, 0.0f), Vec3f(std::cos(angle) * 5.0f, std::sin(angle) * 5.0f, std::cos(angle) * 2.0 - 7.0f));

        texture.update(frame_buffer.data());

        window.clear();
        window.draw(sprite);
        window.display();
    }

    return 0;
}

bool draw_rotate(const aiMesh* mesh, sf::VertexArray& frame_buffer, float angle_deg) {
    Mat4f persp_proj = Mat4f::create_perspective(45.0 * (std::numbers::pi / 180.0), w / (float)h, -10.0f, -50.0f);
    Mat4f viewport = Mat4f::create_viewport(w, h);

    //Mat4f translation = Mat4f::create_translation(Vec3f{ -8.4496f, 0.0f, -6.9425f });
    Mat4f translation = Mat4f::create_identity();
    Mat4f rotation_x = Mat4f::create_identity();
    //Mat4f rotation_x = Mat4f::create_rotation_x(0 * (std::numbers::pi / 180.0));
    //Mat4f rotation_y = Mat4f::create_rotation_y(30.0 * (std::numbers::pi / 180.0));
    Mat4f rotation_y = Mat4f::create_identity();
    Mat4f rotation_z = Mat4f::create_identity();
    //Mat4f rotation_z = Mat4f::create_rotation_z(angle_deg * (std::numbers::pi / 180.0));

    for (unsigned int i = 0; i != mesh->mNumFaces; i++) {
        const aiFace& face = mesh->mFaces[i];
        const unsigned int idx[3] = { face.mIndices[0], face.mIndices[1], face.mIndices[2] };
        std::vector<Vertex> triangle_vertices;
        std::vector<Vec3f> triangle_vertices_orig;
        for (int j = 0; j != 3; j++) {
            const aiVector3D va = mesh->mVertices[idx[j]];

            Vec4f v4{ va.x, va.y, va.z };

            // Сначала translation, потом rotation.
            //Vec4f translated = translation * v4;
            //Vec4f rotated = rotation_y * translated;
            //Vec4f in_clip_space = persp_proj * rotated;

            // Сначала rotation, потом translation.
            Vec4f rotated = rotation_y * v4;
            //rotated = rotation_y * rotated;
            //rotated = rotation_z * rotated;
            Vec4f translated = translation * rotated;

            Mat4f view_mat = cam.get_view_mat();
            Vec4f in_cam_space = view_mat * translated;

            // Для корректного расчета освещенности.
            Vec3f tranformed_orig = in_cam_space;
            triangle_vertices_orig.push_back(Vec3f(tranformed_orig.x, tranformed_orig.y, tranformed_orig.z));

            Vec4f in_clip_space = persp_proj * in_cam_space;
            Vec4f in_ndc = in_clip_space / in_clip_space.w;
            Vec3f in_screen = viewport * in_ndc;

            aiVector3D uv = mesh->mTextureCoords[0][idx[j]];
            float u = uv.x;
            float v = uv.y;

            Vertex vt{ in_screen, u, v };
            triangle_vertices.push_back(vt);
        }

        //

        Vec3f vector_a = triangle_vertices_orig[1] - triangle_vertices_orig[0];
        Vec3f vector_b = triangle_vertices_orig[2] - triangle_vertices_orig[0];
        Vec3f face_normal = Vec3f::cross(vector_a, vector_b).get_normalized();
        //Vec3f face_normal = Vec3f::cross(vector_b, vector_a).get_normalized();
        Vec3f light_dir = Vec3f(0.0f, 0.0f, -1.0f);
        float intensity = Vec3f::dot(face_normal, light_dir);
        if (intensity < 0) {
            draw_triangle(triangle_vertices[0], triangle_vertices[1], triangle_vertices[2], frame_buffer, std::abs(intensity), true);
        }
    }

    return true;
}

void draw_mesh_(const aiMesh* mesh, std::vector<sf::Uint8>& frame_buffer, const Vec3f& rotation, const Vec3f& translation) {
    //Mat4f persp_proj_mat = Mat4f::create_perspective(45.0 * (std::numbers::pi / 180.0), w / (float)h, -2.0f, -8.0f);
    Mat4f persp_proj_mat = Mat4f::create_perspective(45.0 * (std::numbers::pi / 180.0), w / (float)h, -2.0f, -10.0f);
    Mat4f viewport_mat = Mat4f::create_viewport(w, h);

    Mat4f translation_mat = Mat4f::create_translation(translation);
    Mat4f rotation_x_mat = Mat4f::create_rotation_x(rotation.x * (std::numbers::pi / 180.0));
    Mat4f rotation_y_mat = Mat4f::create_rotation_y(rotation.y * (std::numbers::pi / 180.0));
    Mat4f rotation_z_mat = Mat4f::create_rotation_z(rotation.z * (std::numbers::pi / 180.0));

    const float far_clipping_plane_z = -10.0f;
    const float near_clipping_plane_z = -8.0f;

    std::vector<Polygon> polygons;
    polygons.reserve(mesh->mNumFaces);
    for (unsigned int i = 0; i != mesh->mNumFaces; i++) {
        const aiFace& face = mesh->mFaces[i];
        const unsigned int indices[3] = { face.mIndices[0], face.mIndices[1], face.mIndices[2] };

        Polygon polygon{};
        for (unsigned int j = 0; j != 3; j++) {
            const aiVector3D orig_vertex = mesh->mVertices[indices[j]];
            const aiVector3D uv = mesh->mTextureCoords[0][indices[j]];

            Vertex4 vertex{
                Vec4f{orig_vertex.x, orig_vertex.y, orig_vertex.z},
                uv.x,
                uv.y
            };

            vertex.pos = rotation_x_mat * vertex.pos;
            vertex.pos = rotation_y_mat * vertex.pos;
            vertex.pos = rotation_z_mat * vertex.pos;

            vertex.pos = translation_mat * vertex.pos;

            Mat4f view_mat = cam.get_view_mat();
            vertex.pos = view_mat * vertex.pos;

            polygon.vertices[j] = vertex;
        }

        // Backface culling in camera space.
        /*if (is_backfaced(polygon))
            continue;*/

            // Far plane culling.
        if (polygon.vertices[0].pos.z <= far_clipping_plane_z &&
            polygon.vertices[1].pos.z <= far_clipping_plane_z &&
            polygon.vertices[2].pos.z <= far_clipping_plane_z)
            continue;

        // Near plane culling.
        if (polygon.vertices[0].pos.z >= near_clipping_plane_z &&
            polygon.vertices[1].pos.z >= near_clipping_plane_z &&
            polygon.vertices[2].pos.z >= near_clipping_plane_z)
            continue;

        // BEGIN: Left X plane culling.
        const float d = 1.0f / std::tanf(45.0 * (std::numbers::pi / 180.0) / 2.0f);
        const float left_plane_k = w / (h * d); // aspect_ratio / d;

        int verts_out_count = 0;
        verts_out_count += polygon.vertices[0].pos.x <= (polygon.vertices[0].pos.z * left_plane_k);
        verts_out_count += polygon.vertices[1].pos.x <= (polygon.vertices[1].pos.z * left_plane_k);
        verts_out_count += polygon.vertices[2].pos.x <= (polygon.vertices[2].pos.z * left_plane_k);
        if (verts_out_count == 3)
            continue;
        // END: Left X plane culling.

        // BEGIN: Right X plane culling.
        const float right_plane_k = -left_plane_k;

        verts_out_count = 0;
        verts_out_count += polygon.vertices[0].pos.x >= (polygon.vertices[0].pos.z * right_plane_k);
        verts_out_count += polygon.vertices[1].pos.x >= (polygon.vertices[1].pos.z * right_plane_k);
        verts_out_count += polygon.vertices[2].pos.x >= (polygon.vertices[2].pos.z * right_plane_k);
        if (verts_out_count == 3)
            continue;
        // END: Right X plane culling.

        // BEGIN: Top Y plane culling.
        const float top_plane_k = -(1.0f / d); // Поскольку z-координаты вершин уже отрицательные.

        verts_out_count = 0;
        verts_out_count += polygon.vertices[0].pos.y >= (polygon.vertices[0].pos.z * top_plane_k);
        verts_out_count += polygon.vertices[1].pos.y >= (polygon.vertices[1].pos.z * top_plane_k);
        verts_out_count += polygon.vertices[2].pos.y >= (polygon.vertices[2].pos.z * top_plane_k);
        if (verts_out_count == 3)
            continue;
        // END: Top Y plane culling.

        // BEGIN: Bottom Y plane culling.
        const float bottom_plane_k = -top_plane_k;

        verts_out_count = 0;
        verts_out_count += polygon.vertices[0].pos.y <= (polygon.vertices[0].pos.z * bottom_plane_k);
        verts_out_count += polygon.vertices[1].pos.y <= (polygon.vertices[1].pos.z * bottom_plane_k);
        verts_out_count += polygon.vertices[2].pos.y <= (polygon.vertices[2].pos.z * bottom_plane_k);
        if (verts_out_count == 3)
            continue;
        // END: Bottom Y plane culling.

        // BEGIN: Near plane clipping.
        // NOTE: Определяем параметр t прямой, при котором точка лежит и на прямой и на ближней плоскости.

        // Определяем количество вершин снаружи.
        verts_out_count = 0;
        verts_out_count += polygon.vertices[0].pos.z > near_clipping_plane_z;
        verts_out_count += polygon.vertices[1].pos.z > near_clipping_plane_z;
        verts_out_count += polygon.vertices[2].pos.z > near_clipping_plane_z;

        // Простой случай: две вершины снаружи - обновляем координаты "торчащих" вершин.
        // Куллинг мы выполнили выше, поэтому, если две вершины торчат за ближней плоскостью,
        // значит третья вершина точно внутри.
        if (verts_out_count == 2) {
            // Предполагаем по умолчанию такой расклад.
            int vert_in_index = 0;
            int vert_out_1_index = 1;
            int vert_out_2_index = 2;

            // Если вторая вершина внутри, значит две другие - снаружи.
            if (polygon.vertices[1].pos.z < near_clipping_plane_z) {
                vert_in_index = 1;
                vert_out_1_index = 0;
                vert_out_2_index = 2;
            }
            // Если третья вершина внутри, значит две другие - снаружи.
            else if (polygon.vertices[2].pos.z < near_clipping_plane_z) {
                vert_in_index = 2;
                vert_out_1_index = 0;
                vert_out_2_index = 1;
            }

            // Ищем первую точку пересечения и обновляем первую внешнюю вершину.
            const Vec3f vert_in_pos = polygon.vertices[vert_in_index].pos;
            const Vec3f vert_out_1_pos = polygon.vertices[vert_out_1_index].pos;
            const Vec3f from_in_to_out_1 = vert_out_1_pos - vert_in_pos;
            float line_param_t = (near_clipping_plane_z - vert_in_pos.z) / from_in_to_out_1.z;
            const Vec3f intersection_point_1 = vert_in_pos + from_in_to_out_1 * line_param_t;
            polygon.vertices[vert_out_1_index].pos = intersection_point_1;

            // Ищем вторую точку пересечения и обновляем вторую внешнюю вершину.
            const Vec3f vert_out_2_pos = polygon.vertices[vert_out_2_index].pos;
            const Vec3f from_in_to_out_2 = vert_out_2_pos - vert_in_pos;
            line_param_t = (near_clipping_plane_z - vert_in_pos.z) / from_in_to_out_2.z;
            const Vec3f intersection_point_2 = vert_in_pos + from_in_to_out_2 * line_param_t;
            polygon.vertices[vert_out_2_index].pos = intersection_point_2;
        }
        // Случай посложней: у одного полигона обновляем одну координату
        // и добавляем еще один полигон.
        else if (verts_out_count == 1) {
            // Предполагаем по умолчанию такой расклад.
            int vert_out_index = 0;
            int vert_in_1_index = 1;
            int vert_in_2_index = 2;

            // Если вторая вершина снаружи, значит две другие - внутри.
            if (polygon.vertices[1].pos.z > near_clipping_plane_z) {
                vert_out_index = 1;
                vert_in_1_index = 0;
                vert_in_2_index = 2;
            }
            // Если третья вершина снаружи, значит две другие - внутри.
            else if (polygon.vertices[2].pos.z > near_clipping_plane_z) {
                vert_out_index = 2;
                vert_in_1_index = 0;
                vert_in_2_index = 1;
            }

            // Ищем первую точку пересечения.
            const Vec3f vert_out_pos = polygon.vertices[vert_out_index].pos;
            const Vec3f vert_in_1_pos = polygon.vertices[vert_in_1_index].pos;
            Vec3f from_in_to_out = vert_out_pos - vert_in_1_pos;
            float line_param_t = (near_clipping_plane_z - vert_in_1_pos.z) / from_in_to_out.z;
            const Vec3f intersection_point_1 = vert_in_1_pos + from_in_to_out * line_param_t;
            polygon.vertices[vert_out_index].pos = intersection_point_1;

            // Ищем вторую точку пересечения.
            const Vec3f vert_in_2_pos = polygon.vertices[vert_in_2_index].pos;
            from_in_to_out = vert_out_pos - vert_in_2_pos;
            line_param_t = (near_clipping_plane_z - vert_in_2_pos.z) / from_in_to_out.z;
            const Vec3f intersection_point_2 = vert_in_2_pos + from_in_to_out * line_param_t;

            Polygon new_poly{};
            new_poly.vertices[0] = Vertex4{ vert_in_2_pos };
            new_poly.vertices[1] = Vertex4{ intersection_point_1 };
            new_poly.vertices[2] = Vertex4{ intersection_point_2 };

            polygons.push_back(new_poly);
        }
        polygons.push_back(polygon);
        // END: Near plane clipping.

        /*
        // To clip space.
        polygon.vertices[0].pos = persp_proj * polygon.vertices[0].pos;
        polygon.vertices[1].pos = persp_proj * polygon.vertices[1].pos;
        polygon.vertices[2].pos = persp_proj * polygon.vertices[2].pos;

        // To ndc.
        polygon.vertices[0].pos = polygon.vertices[0].pos / polygon.vertices[0].pos.w;
        polygon.vertices[1].pos = polygon.vertices[1].pos / polygon.vertices[1].pos.w;
        polygon.vertices[2].pos = polygon.vertices[2].pos / polygon.vertices[2].pos.w;

        // To screen space.
        polygon.vertices[0].pos = viewport_mat * polygon.vertices[0].pos;
        polygon.vertices[1].pos = viewport_mat * polygon.vertices[1].pos;
        polygon.vertices[2].pos = viewport_mat * polygon.vertices[2].pos;

        Vertex v0{ polygon.vertices[0].pos, polygon.vertices[0].u, polygon.vertices[0].v };
        Vertex v1{ polygon.vertices[1].pos, polygon.vertices[1].u, polygon.vertices[1].v };
        Vertex v2{ polygon.vertices[2].pos, polygon.vertices[2].u, polygon.vertices[2].v };
        draw_triangle(v0, v1, v2, frame_buffer, 1.0f, false);
        */
    }

    // Теперь у нас есть список полигонов в пространстве камеры.
    // Все полигоны делятся на классы:
    // 1. Исходные полигоны модели, которые не подверглись ни клиппингу ни куллингу.
    // 2. Усеченные полигоны модели (полигоны, две вершины которых оказались за пределами плоскости отсечения).
    // 3. Усеченные полигоны модели + новые полигоны (одна вершина за пределами плоскости отсечения).
    // 4. Полностью отброшенные полигоны (все вершины за пределами плоскости отсечения).
    // 5. Полностью отброшенные полигоны (нелицевые полигоны).

    for (Polygon& polygon : polygons) {
        // To clip space.
        polygon.vertices[0].pos = persp_proj_mat * polygon.vertices[0].pos;
        polygon.vertices[1].pos = persp_proj_mat * polygon.vertices[1].pos;
        polygon.vertices[2].pos = persp_proj_mat * polygon.vertices[2].pos;

        // To ndc.
        polygon.vertices[0].pos = polygon.vertices[0].pos / polygon.vertices[0].pos.w;
        polygon.vertices[1].pos = polygon.vertices[1].pos / polygon.vertices[1].pos.w;
        polygon.vertices[2].pos = polygon.vertices[2].pos / polygon.vertices[2].pos.w;

        // To screen space.
        polygon.vertices[0].pos = viewport_mat * polygon.vertices[0].pos;
        polygon.vertices[1].pos = viewport_mat * polygon.vertices[1].pos;
        polygon.vertices[2].pos = viewport_mat * polygon.vertices[2].pos;

        Vertex v0{ polygon.vertices[0].pos, polygon.vertices[0].u, polygon.vertices[0].v };
        Vertex v1{ polygon.vertices[1].pos, polygon.vertices[1].u, polygon.vertices[1].v };
        Vertex v2{ polygon.vertices[2].pos, polygon.vertices[2].u, polygon.vertices[2].v };

        draw_triangle_(frame_buffer, v0, v1, v2);
    }
}

void draw_mesh(const aiMesh* mesh, sf::VertexArray& frame_buffer, float angle_deg) {
    Mat4f persp_proj = Mat4f::create_perspective(45.0 * (std::numbers::pi / 180.0), w / (float)h, -10.0f, -50.0f);
    Mat4f viewport_mat = Mat4f::create_viewport(w, h);

    Mat4f translation = Mat4f::create_identity();
    Mat4f rotation_x = Mat4f::create_identity();
    //Mat4f rotation_x = Mat4f::create_rotation_x(15.0 * (std::numbers::pi / 180.0));
    //Mat4f rotation_x = Mat4f::create_rotation_x(angle_deg * (std::numbers::pi / 180.0));
    Mat4f rotation_y = Mat4f::create_identity();
    //Mat4f rotation_y = Mat4f::create_rotation_y(15.0 * (std::numbers::pi / 180.0));
    //Mat4f rotation_y = Mat4f::create_rotation_y(angle_deg * (std::numbers::pi / 180.0));
    Mat4f rotation_z = Mat4f::create_identity();

    const float far_clipping_plane_z = -50.0f;
    const float near_clipping_plane_z = -10.0f;

    std::vector<Polygon> polygons;
    polygons.reserve(mesh->mNumFaces);
    for (unsigned int i = 0; i != mesh->mNumFaces; i++) {
        const aiFace& face = mesh->mFaces[i];
        const unsigned int indices[3] = { face.mIndices[0], face.mIndices[1], face.mIndices[2] };

        Polygon polygon{};
        for (unsigned int j = 0; j != 3; j++) {
            const aiVector3D orig_vertex = mesh->mVertices[indices[j]];
            const aiVector3D uv = mesh->mTextureCoords[0][indices[j]];

            Vertex4 vertex{
                Vec4f{orig_vertex.x, orig_vertex.y, orig_vertex.z},
                uv.x,
                uv.y
            };

            vertex.pos = rotation_x * vertex.pos;
            vertex.pos = rotation_y * vertex.pos;
            vertex.pos = rotation_z * vertex.pos;

            vertex.pos = translation * vertex.pos;

            Mat4f view_mat = cam.get_view_mat();
            vertex.pos = view_mat * vertex.pos;

            polygon.vertices[j] = vertex;
        }

        // Backface culling in camera space.
        /*if (is_backfaced(polygon))
            continue;*/

            // Far plane culling.
        if (polygon.vertices[0].pos.z <= far_clipping_plane_z &&
            polygon.vertices[1].pos.z <= far_clipping_plane_z &&
            polygon.vertices[2].pos.z <= far_clipping_plane_z)
            continue;

        // Near plane culling.
        if (polygon.vertices[0].pos.z >= near_clipping_plane_z &&
            polygon.vertices[1].pos.z >= near_clipping_plane_z &&
            polygon.vertices[2].pos.z >= near_clipping_plane_z)
            continue;

        // X planes culling.
        // Y planes culling.

        // BEGIN: Near plane clipping.
        // NOTE: Определяем параметр t прямой, при котором точка лежит и на прямой и на ближней плоскости.

        // Определяем количество вершин снаружи.
        int verts_out_count = 0;
        verts_out_count += polygon.vertices[0].pos.z > near_clipping_plane_z;
        verts_out_count += polygon.vertices[1].pos.z > near_clipping_plane_z;
        verts_out_count += polygon.vertices[2].pos.z > near_clipping_plane_z;

        // Простой случай: две вершины снаружи - обновляем координаты "торчащих" вершин.
        // Куллинг мы выполнили выше, поэтому, если две вершины торчат за ближней плоскостью,
        // значит третья вершина точно внутри.
        if (verts_out_count == 2) {
            // Предполагаем по умолчанию такой расклад.
            int vert_in_index = 0;
            int vert_out_1_index = 1;
            int vert_out_2_index = 2;

            // Если вторая вершина внутри, значит две другие - снаружи.
            if (polygon.vertices[1].pos.z < near_clipping_plane_z) {
                vert_in_index = 1;
                vert_out_1_index = 0;
                vert_out_2_index = 2;
            }
            // Если третья вершина внутри, значит две другие - снаружи.
            else if (polygon.vertices[2].pos.z < near_clipping_plane_z) {
                vert_in_index = 2;
                vert_out_1_index = 0;
                vert_out_2_index = 1;
            }

            // Ищем первую точку пересечения и обновляем первую внешнюю вершину.
            const Vec3f vert_in_pos = polygon.vertices[vert_in_index].pos;
            const Vec3f vert_out_1_pos = polygon.vertices[vert_out_1_index].pos;
            const Vec3f from_in_to_out_1 = vert_out_1_pos - vert_in_pos;
            float line_param_t = (near_clipping_plane_z - vert_in_pos.z) / from_in_to_out_1.z;
            const Vec3f intersection_point_1 = vert_in_pos + from_in_to_out_1 * line_param_t;
            polygon.vertices[vert_out_1_index].pos = intersection_point_1;

            // Ищем вторую точку пересечения и обновляем вторую внешнюю вершину.
            const Vec3f vert_out_2_pos = polygon.vertices[vert_out_2_index].pos;
            const Vec3f from_in_to_out_2 = vert_out_2_pos - vert_in_pos;
            line_param_t = (near_clipping_plane_z - vert_in_pos.z) / from_in_to_out_2.z;
            const Vec3f intersection_point_2 = vert_in_pos + from_in_to_out_2 * line_param_t;
            polygon.vertices[vert_out_2_index].pos = intersection_point_2;
        }
        // Случай посложней: у одного полигона обновляем одну координату
        // и добавляем еще один полигон.
        else if (verts_out_count == 1) {
            // Предполагаем по умолчанию такой расклад.
            int vert_out_index = 0;
            int vert_in_1_index = 1;
            int vert_in_2_index = 2;

            // Если вторая вершина снаружи, значит две другие - внутри.
            if (polygon.vertices[1].pos.z > near_clipping_plane_z) {
                vert_out_index = 1;
                vert_in_1_index = 0;
                vert_in_2_index = 2;
            }
            // Если третья вершина снаружи, значит две другие - внутри.
            else if (polygon.vertices[2].pos.z > near_clipping_plane_z) {
                vert_out_index = 2;
                vert_in_1_index = 0;
                vert_in_2_index = 1;
            }

            // Ищем первую точку пересечения.
            const Vec3f vert_out_pos = polygon.vertices[vert_out_index].pos;
            const Vec3f vert_in_1_pos = polygon.vertices[vert_in_1_index].pos;
            Vec3f from_in_to_out = vert_out_pos - vert_in_1_pos;
            float line_param_t = (near_clipping_plane_z - vert_in_1_pos.z) / from_in_to_out.z;
            const Vec3f intersection_point_1 = vert_in_1_pos + from_in_to_out * line_param_t;
            polygon.vertices[vert_out_index].pos = intersection_point_1;

            // Ищем вторую точку пересечения.
            const Vec3f vert_in_2_pos = polygon.vertices[vert_in_2_index].pos;
            from_in_to_out = vert_out_pos - vert_in_2_pos;
            line_param_t = (near_clipping_plane_z - vert_in_2_pos.z) / from_in_to_out.z;
            const Vec3f intersection_point_2 = vert_in_2_pos + from_in_to_out * line_param_t;

            Polygon new_poly{};
            new_poly.vertices[0] = Vertex4{ vert_in_2_pos };
            new_poly.vertices[1] = Vertex4{ intersection_point_1 };
            new_poly.vertices[2] = Vertex4{ intersection_point_2 };

            polygons.push_back(new_poly);
        }
        polygons.push_back(polygon);
        // END: Near plane clipping.

        /*
        // To clip space.
        polygon.vertices[0].pos = persp_proj * polygon.vertices[0].pos;
        polygon.vertices[1].pos = persp_proj * polygon.vertices[1].pos;
        polygon.vertices[2].pos = persp_proj * polygon.vertices[2].pos;

        // To ndc.
        polygon.vertices[0].pos = polygon.vertices[0].pos / polygon.vertices[0].pos.w;
        polygon.vertices[1].pos = polygon.vertices[1].pos / polygon.vertices[1].pos.w;
        polygon.vertices[2].pos = polygon.vertices[2].pos / polygon.vertices[2].pos.w;

        // To screen space.
        polygon.vertices[0].pos = viewport_mat * polygon.vertices[0].pos;
        polygon.vertices[1].pos = viewport_mat * polygon.vertices[1].pos;
        polygon.vertices[2].pos = viewport_mat * polygon.vertices[2].pos;

        Vertex v0{ polygon.vertices[0].pos, polygon.vertices[0].u, polygon.vertices[0].v };
        Vertex v1{ polygon.vertices[1].pos, polygon.vertices[1].u, polygon.vertices[1].v };
        Vertex v2{ polygon.vertices[2].pos, polygon.vertices[2].u, polygon.vertices[2].v };
        draw_triangle(v0, v1, v2, frame_buffer, 1.0f, false);
        */
    }

    // Теперь у нас есть список полигонов в пространстве камеры.
    // Все полигоны делятся на классы:
    // 1. Исходные полигоны модели, которые не подверглись ни клиппингу ни куллингу.
    // 2. Усеченные полигоны модели (полигоны, две вершины которых оказались за пределами плоскости отсечения).
    // 3. Усеченные полигоны модели + новые полигоны (одна вершина за пределами плоскости отсечения).
    // 4. Полностью отброшенные полигоны (все вершины за пределами плоскости отсечения).
    // 5. Полностью отброшенные полигоны (нелицевые полигоны).

    for (Polygon& polygon : polygons) {
        // To clip space.
        polygon.vertices[0].pos = persp_proj * polygon.vertices[0].pos;
        polygon.vertices[1].pos = persp_proj * polygon.vertices[1].pos;
        polygon.vertices[2].pos = persp_proj * polygon.vertices[2].pos;

        // To ndc.
        polygon.vertices[0].pos = polygon.vertices[0].pos / polygon.vertices[0].pos.w;
        polygon.vertices[1].pos = polygon.vertices[1].pos / polygon.vertices[1].pos.w;
        polygon.vertices[2].pos = polygon.vertices[2].pos / polygon.vertices[2].pos.w;

        // To screen space.
        polygon.vertices[0].pos = viewport_mat * polygon.vertices[0].pos;
        polygon.vertices[1].pos = viewport_mat * polygon.vertices[1].pos;
        polygon.vertices[2].pos = viewport_mat * polygon.vertices[2].pos;

        Vertex v0{ polygon.vertices[0].pos, polygon.vertices[0].u, polygon.vertices[0].v };
        Vertex v1{ polygon.vertices[1].pos, polygon.vertices[1].u, polygon.vertices[1].v };
        Vertex v2{ polygon.vertices[2].pos, polygon.vertices[2].u, polygon.vertices[2].v };
        draw_triangle(v0, v1, v2, frame_buffer, 1.0f, false);
    }
}

bool is_backfaced(const Polygon& polygon_in_cam_space) {
    Vec3f v0 = polygon_in_cam_space.vertices[0].pos;
    Vec3f v1 = polygon_in_cam_space.vertices[1].pos;
    Vec3f v2 = polygon_in_cam_space.vertices[2].pos;

    Vec3f a = v1 - v0;
    Vec3f b = v2 - v0;

    Vec3f normal = Vec3f::cross(a, b);
    Vec3f view{ 0.0f, 0.0f, 1.0f };

    // TODO: Решить, что делать, когда результат близок к нулю,
    // но тем не менее все еще больше эпсилон.
    float proj = Vec3f::dot(normal, view);
    //print_float_as_hex(&proj);

    return  proj <= 0.0f;
}

std::vector<int> interpolate_x(Vec2i a, Vec2i b) {
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

void draw_line_color(Vec2i a, Vec2i b, sf::VertexArray& frame_buffer, const sf::Color& color) {
    if (a.x == b.x && a.y == b.y) {
        //std::cout << std::format("Line drawing from [{},{}] to [{},{}]\n", a.x, a.y, b.x, b.y);

        sf::Vector2f pos(a.x, a.y);
        int index = w * a.y + a.x; // Раскладываем строки буфера в горизонтальную линию.

        pos.y = h - pos.y;
        frame_buffer[index].position = pos;
        frame_buffer[index].color = color;

        //p_win->draw(frame_buffer);
        //p_win->display();
        //std::this_thread::sleep_for(std::chrono::milliseconds(100));
        return;
    }

    bool swapped = false;
    if (std::abs(a.x - b.x) < std::abs(a.y - b.y)) {
        std::swap(a.x, a.y);
        std::swap(b.x, b.y);
        swapped = true;
    }

    if (a.x > b.x) {
        std::swap(a.x, b.x);
        std::swap(a.y, b.y);
    }

    /*if (swapped)
        std::cout << std::format("Line drawing from [{},{}] to [{},{}]\n", a.y, a.x, b.y, b.x);
    else
        std::cout << std::format("Line drawing from [{},{}] to [{},{}]\n", a.x, a.y, b.x, b.y);*/

    for (int x = a.x; x <= b.x; x++) {
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

        //p_win->draw(frame_buffer);
        //p_win->display();
        //std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void draw_line(Vec2i a, Vec2i b, sf::VertexArray& frame_buffer, float intensity, Vertex ta, Vertex tb, Vertex tc) {
    if (a.x == b.x && a.y == b.y) {
        //std::cout << std::format("Line drawing from [{},{}] to [{},{}]\n", a.x, a.y, b.x, b.y);

        sf::Vector2f pos(a.x, a.y);
        int index = w * a.y + a.x; // Раскладываем строки буфера в горизонтальную линию.

        int source_fragment_coord_x = a.x;
        int source_fragment_coord_y = a.y;
        float source_fragment_depth = get_depth_for_fragment(source_fragment_coord_x, source_fragment_coord_y, ta.pos, tb.pos, tc.pos);
        float target_fragment_depth = depth_buffer.at(index);
        if (source_fragment_depth > target_fragment_depth) {
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
    if (std::abs(a.x - b.x) < std::abs(a.y - b.y)) {
        std::swap(a.x, a.y);
        std::swap(b.x, b.y);
        swapped = true;
    }

    if (a.x > b.x) {
        std::swap(a.x, b.x);
        std::swap(a.y, b.y);
    }

    /*if (swapped)
        std::cout << std::format("Line drawing from [{},{}] to [{},{}]\n", a.y, a.x, b.y, b.x);
    else
        std::cout << std::format("Line drawing from [{},{}] to [{},{}]\n", a.x, a.y, b.x, b.y);*/

    for (int x = a.x; x <= b.x; x++) {
        float x_way_percent = (x - a.x) / (float)(b.x - a.x);
        int y = (b.y - a.y) * x_way_percent + a.y;

        sf::Vector2f pos(x, y);
        int index = w * y + x; // Раскладываем строки буфера в горизонтальную линию.

        if (swapped) {
            pos.x = y;
            pos.y = x;

            index = w * x + y; // Был своп координат. Меняем, чтобы не перепутать строки со столбцами.
        }

        int source_fragment_coord_x = x;
        int source_fragment_coord_y = y;
        float source_fragment_depth = get_depth_for_fragment(source_fragment_coord_x, source_fragment_coord_y, ta.pos, tb.pos, tc.pos);
        float target_fragment_depth = depth_buffer.at(index);
        if (source_fragment_depth > target_fragment_depth) {
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

float get_depth_for_fragment(int coord_x, int coord_y, Vec3f ta, Vec3f tb, Vec3f tc) {
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

void interpolate_uv(int fragment_coord_x, int fragment_coord_y, Vertex a, Vertex b, Vertex c, float& u, float& v) {
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

void draw_triangle(Vertex a, Vertex b, Vertex c, sf::VertexArray& frame_buffer, float intensity, bool filled) {
    if (!filled) {
        draw_line_color(Vec2i(a.pos.x, a.pos.y), Vec2i(b.pos.x, b.pos.y), frame_buffer, sf::Color::Black);
        draw_line_color(Vec2i(b.pos.x, b.pos.y), Vec2i(c.pos.x, c.pos.y), frame_buffer, sf::Color::Black);
        draw_line_color(Vec2i(c.pos.x, c.pos.y), Vec2i(a.pos.x, a.pos.y), frame_buffer, sf::Color::Black);
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
    if (x_coords_ac.empty()) {
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

    for (const auto& x : x_coords_abc) {
        //std::cout << x << std::endl;
    }

    int triangle_height = c.pos.y - a.pos.y;
    //std::cout << triangle_height << std::endl;

    for (int i = 0; i < triangle_height + 1; i++) {
        const int x_1 = x_coords_ac[i];
        const int x_2 = x_coords_abc[i];
        Vec2i p1(x_1, i + a.pos.y);
        Vec2i p2(x_2, i + a.pos.y);

        draw_line(p1, p2, frame_buffer, intensity, a, b, c);
    }
}

void draw_triangle_(std::vector<sf::Uint8>& frame_buffer, Vertex a, Vertex b, Vertex c) {
    Vertex a_copy = a;
    Vertex b_copy = b;
    Vertex c_copy = c;

    if (clip_line_coh_suth(a_copy.pos.x, a_copy.pos.y, b_copy.pos.x, b_copy.pos.y))
        draw_line_dda(frame_buffer, std::round(a_copy.pos.x), std::round(a_copy.pos.y), std::round(b_copy.pos.x), std::round(b_copy.pos.y), sf::Color::Black);

    b_copy = b;
    if (clip_line_coh_suth(b_copy.pos.x, b_copy.pos.y, c_copy.pos.x, c_copy.pos.y))
        draw_line_dda(frame_buffer, std::round(b_copy.pos.x), std::round(b_copy.pos.y), std::round(c_copy.pos.x), std::round(c_copy.pos.y), sf::Color::Black);

    c_copy = c;
    a_copy = a;
    if (clip_line_coh_suth(c_copy.pos.x, c_copy.pos.y, a_copy.pos.x, a_copy.pos.y))
        draw_line_dda(frame_buffer, std::round(c_copy.pos.x), std::round(c_copy.pos.y), std::round(a_copy.pos.x), std::round(a_copy.pos.y), sf::Color::Black);
}

void draw_line_dda(std::vector<sf::Uint8>& frame_buffer, int x0, int y0, int x1, int y1, sf::Color color) {
    // Vertical.
    if (x0 == x1) {
        // Make ascending.
        if (y0 > y1) {
            std::swap(y0, y1);
            std::swap(x0, x1);
        }

        for (int y = y0; y <= y1; y++) {
            set_pixel_color(frame_buffer, x0, y, color);
        }
    }
    // Horizontal.
    else if (y0 == y1) {
        // Make ascending.
        if (x0 > x1) {
            std::swap(x0, x1);
        }

        for (int x = x0; x <= x1; x++) {
            set_pixel_color(frame_buffer, x, y0, color);
        }
    }

    int dy = y1 - y0;
    int dx = x1 - x0;

    // Non-steep.
    if (std::abs(dy) <= std::abs(dx)) {
        // Make ascending.
        if (x0 > x1) {
            std::swap(x0, x1);
            std::swap(y0, y1);
            dx = -dx;
            dy = -dy;
        }

        const float slope = static_cast<float>(dy) / dx;

        float y_accum = y0;
        for (int x = x0; x <= x1; x++) {
            set_pixel_color(frame_buffer, x, std::round(y_accum), color);
            y_accum += slope;
        }
    }
    // Steep.
    else {
        // Make ascending.
        if (y0 > y1) {
            std::swap(y0, y1);
            std::swap(x0, x1);
            dy = -dy;
            dx = -dx;
        }

        const float inv_slope = static_cast<float>(dx) / dy;

        float x_accum = x0;
        for (int y = y0; y <= y1; y++) {
            set_pixel_color(frame_buffer, std::round(x_accum), y, color);
            x_accum += inv_slope;
        }
    }
}

bool clip_line_coh_suth(float& x0, float& y0, float& x1, float& y1) {
    enum EdgeBit {
        left = 3,
        right = 2,
        top = 1,
        bottom = 0
    };

    struct Point {
        float& x;
        float& y;
        std::bitset<4> region_code;
    };

    Point p1{ x1, y1 };
    p1.region_code.set(EdgeBit::left, p1.x < 0);
    p1.region_code.set(EdgeBit::right, p1.x > w - 1);
    p1.region_code.set(EdgeBit::top, p1.y < 0);
    p1.region_code.set(EdgeBit::bottom, p1.y > h - 1);

    Point p0{ x0, y0 };
    p0.region_code.set(EdgeBit::left, p0.x < 0);
    p0.region_code.set(EdgeBit::right, p0.x > w - 1);
    p0.region_code.set(EdgeBit::top, p0.y < 0);
    p0.region_code.set(EdgeBit::bottom, p0.y > h - 1);

    bool line_inside = (p0.region_code | p1.region_code).none();
    bool line_outside = (p0.region_code & p1.region_code).any();
    while (!line_inside && !line_outside) {
        // Make sure the first point is the one that is outside.
        if (p0.region_code.none()) {
            std::swap(p0.x, p1.x);
            std::swap(p0.y, p1.y);
            std::swap(p0.region_code, p1.region_code);
        }

        // Find the first edge outside of which the point is.
        EdgeBit first_edge;
        for (int i = EdgeBit::left; i >= EdgeBit::bottom; i--) {
            if (p0.region_code[i]) {
                first_edge = static_cast<EdgeBit>(i);
                break;
            }
        }

        if (first_edge == EdgeBit::left || first_edge == EdgeBit::right) {
            float edge_x = first_edge == EdgeBit::left ? 0 : w - 1;
            float slope = (p1.y - p0.y) / (p1.x - p0.x);

            float x_excess = edge_x - p0.x;
            p0.x = edge_x;
            p0.y += x_excess * slope;
        }
        else {
            float edge_y = first_edge == EdgeBit::top ? 0 : h - 1;
            float inv_slope = (p1.x - p0.x) / (p1.y - p0.y);

            float y_excess = edge_y - p0.y;
            p0.y = edge_y;
            p0.x += y_excess * inv_slope;
        }

        p0.region_code.set(EdgeBit::left, p0.x < 0);
        p0.region_code.set(EdgeBit::right, p0.x > w - 1);
        p0.region_code.set(EdgeBit::top, p0.y < 0);
        p0.region_code.set(EdgeBit::bottom, p0.y > h - 1);

        line_inside = (p0.region_code | p1.region_code).none();
        line_outside = (p0.region_code & p1.region_code).any();
    }

    return line_inside ? true : false;
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