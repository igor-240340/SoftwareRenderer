#pragma once

#include "Vertex4.h"

struct Polygon {
    std::array<Vertex4, 3> vertices;
    bool is_culled;
};
