#pragma once

#include <vector>
#include <math.h>
#include <glm/ext.hpp>
#include <glm/glm.hpp>

struct Vertex
{
    glm::vec3 position;
};

inline void generateUVSphere(float radius, unsigned int stacks, unsigned int sectors, std::vector<Vertex>& vertices, std::vector<unsigned int>& indices)
{
	const float PI = 3.14159265359f;

	vertices.clear();
	indices.clear();

    // Generate vertices
    for (unsigned int i = 0; i <= stacks; ++i)
    {
        float v = (float)i / stacks;          // [0,1]
        float phi = PI * v;                   // 0 -> PI

        float y = cos(phi);
        float r = sin(phi);

        for (unsigned int j = 0; j <= sectors; ++j) {
            float u = (float)j / sectors;     // [0,1]
            float theta = 2.0f * PI * u;      // 0 -> 2PI

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
