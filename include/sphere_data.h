#pragma once

#include <vector>
#include <math.h>
#include <map>
#include <glm/ext.hpp>
#include <glm/glm.hpp>

constexpr float kPI = 3.14159265359f;

struct Vertex
{
    glm::vec3 position;
};

inline void generateUVSphere(float radius, unsigned int stacks, unsigned int sectors, std::vector<Vertex>& vertices, std::vector<unsigned int>& indices)
{
	vertices.clear();
	indices.clear();

    // Generate vertices
    for (unsigned int i = 0; i <= stacks; ++i)
    {
        float v = (float)i / stacks;          // [0,1]
        float phi = kPI * v;                   // 0 -> PI

        float y = cos(phi);
        float r = sin(phi);

        for (unsigned int j = 0; j <= sectors; ++j) {
            float u = (float)j / sectors;     // [0,1]
            float theta = 2.0f * kPI * u;      // 0 -> 2PI

            float x = r * cos(theta);
            float z = r * sin(theta);

            Vertex vert;
            vert.position.x = radius * x;
            vert.position.y = radius * y;
            vert.position.z = radius * z;

            //vert.nx = x;
            //vert.ny = y;
            //vert.nz = z;

            //vert.u = u;
            //vert.v = 1.0f - v;

            vertices.push_back(vert);
        }
    }

    // Generate indices
    for (unsigned int i = 0; i < stacks; ++i) {
        unsigned int row1 = i * (sectors + 1);
        unsigned int row2 = (i + 1) * (sectors + 1);

        for (unsigned int j = 0; j < sectors; ++j) {
            // first triangle
            indices.push_back(row1 + j);
            indices.push_back(row2 + j);
            indices.push_back(row1 + j + 1);

            // second triangle
            indices.push_back(row1 + j + 1);
            indices.push_back(row2 + j);
            indices.push_back(row2 + j + 1);
        }
    }
}

static int getMiddlePoint(int p1, int p2, std::vector<glm::vec3>& positions, std::map<long long, int>& cache)
{
    long long key = ((long long)std::min(p1, p2) << 32) + std::max(p1, p2);
    auto it = cache.find(key);
    if (it != cache.end())
        return it->second;

    glm::vec3 middle = glm::normalize((positions[p1] + positions[p2]) * 0.5f);
    positions.push_back(middle);
    int index = (int)positions.size() - 1;
    cache[key] = index;
    return index;
}

inline void generateIcoSphere(float radius, int subdivisions, std::vector<Vertex>& vertices, std::vector<unsigned int>& indices)
{
    vertices.clear();
    indices.clear();

    const float t = (1.0f + sqrt(5.0f)) / 2.0f;

    // ---- create icosahedron ----
    std::vector<glm::vec3> pos = {
        {-1,  t,  0}, { 1,  t,  0}, {-1, -t,  0}, { 1, -t,  0},
        { 0, -1,  t}, { 0,  1,  t}, { 0, -1, -t}, { 0,  1, -t},
        { t,  0, -1}, { t,  0,  1}, {-t,  0, -1}, {-t,  0,  1}
    };

    for (auto& p : pos)
        p = glm::normalize(p);

    std::vector<unsigned int> faces = {
        0,11,5,  0,5,1,  0,1,7,  0,7,10, 0,10,11,
        1,5,9,   5,11,4, 11,10,2,10,7,6, 7,1,8,
        3,9,4,   3,4,2,  3,2,6,  3,6,8,  3,8,9,
        4,9,5,   2,4,11, 6,2,10, 8,6,7,  9,8,1
    };

    // ---- subdivide ----
    for (int s = 0; s < subdivisions; ++s) {
        std::map<long long, int> midpointCache;
        std::vector<unsigned int> newFaces;

        for (size_t i = 0; i < faces.size(); i += 3) {
            unsigned int a = faces[i];
            unsigned int b = faces[i + 1];
            unsigned int c = faces[i + 2];

            unsigned int ab = getMiddlePoint(a, b, pos, midpointCache);
            unsigned int bc = getMiddlePoint(b, c, pos, midpointCache);
            unsigned int ca = getMiddlePoint(c, a, pos, midpointCache);

            newFaces.insert(newFaces.end(), {
                a, ab, ca,
                b, bc, ab,
                c, ca, bc,
                ab, bc, ca
                });
        }

        faces.swap(newFaces);
    }

    // ---- build final vertices ----
    for (auto& p : pos) {
        glm::vec3 n = glm::normalize(p);
        glm::vec3 v = n * radius;

        float u = 0.5f + atan2(n.z, n.x) / (2.0f * kPI);
        float vv = 0.5f - asin(n.y) / kPI;

        vertices.push_back({v
            //n.x, n.y, n.z,
            //u, vv
            });
    }

    indices = faces;
}
