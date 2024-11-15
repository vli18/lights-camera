#include "Cone.h"

void Cone::updateParams(int param1, int param2) {
    m_vertexData = std::vector<float>();
    m_param1 = param1;
    m_param2 = param2;
    setVertexData();
}

void Cone::setVertexData() {
    m_param1 = std::max(m_param1, 1);
    m_param2 = std::max(m_param2, 3);

    makeConeSurface();
    makeConeBase();
}

void Cone::makeConeSurface() {
    glm::vec3 apex(0.0f, 0.5f, 0.0f); // Apex of the cone
    float radius = 0.5f;              // Base radius
    float height = 1.0f;              // Height of the cone

    // Slope vector for the implicit cone
    float slope = radius / height;

    // Precompute base vertices and normals for all segments
    std::vector<glm::vec3> baseVertices(m_param2);
    std::vector<glm::vec3> baseNormals(m_param2);

    for (int i = 0; i < m_param2; ++i) {
        float angle = 2.0f * M_PI * i / m_param2;
        baseVertices[i] = glm::vec3(radius * cos(angle), -0.5f, radius * sin(angle));
        baseNormals[i] = glm::normalize(glm::vec3(baseVertices[i].x, slope, baseVertices[i].z));
    }

    // Loop over the circular base subdivisions
    for (int i = 0; i < m_param2; ++i) {
        int nextIndex = (i + 1) % m_param2; // Wrap around at the end

        glm::vec3 base1 = baseVertices[i];
        glm::vec3 base2 = baseVertices[nextIndex];
        glm::vec3 normal1 = baseNormals[i];
        glm::vec3 normal2 = baseNormals[nextIndex];

        // Loop over vertical subdivisions (latitude tessellation)
        for (int j = 0; j < m_param1; ++j) {
            float t1 = static_cast<float>(j) / m_param1;         // Interpolation factor for segment 1
            float t2 = static_cast<float>(j + 1) / m_param1;     // Interpolation factor for segment 2

            // Interpolated vertices along the height
            glm::vec3 interp1 = glm::mix(base1, apex, t1);
            glm::vec3 interp2 = glm::mix(base2, apex, t1);
            glm::vec3 interp3 = glm::mix(base1, apex, t2);
            glm::vec3 interp4 = glm::mix(base2, apex, t2);

            // Use the same normals as the base for the entire vertical subdivision
            glm::vec3 normalInterp1 = glm::normalize(glm::mix(normal1, normal1, t1));
            glm::vec3 normalInterp2 = glm::normalize(glm::mix(normal2, normal2, t1));
            glm::vec3 normalInterp3 = glm::normalize(glm::mix(normal1, normal1, t2));
            glm::vec3 normalInterp4 = glm::normalize(glm::mix(normal2, normal2, t2));

            // First triangle (interp1 -> interp2 -> interp3)
            insertVec3(m_vertexData, interp1);
            insertVec3(m_vertexData, normalInterp1);
            insertVec3(m_vertexData, interp3);
            insertVec3(m_vertexData, normalInterp3);
            insertVec3(m_vertexData, interp2);
            insertVec3(m_vertexData, normalInterp2);

            // Second triangle (interp3 -> interp4 -> interp2)
            insertVec3(m_vertexData, interp3);
            insertVec3(m_vertexData, normalInterp3);
            insertVec3(m_vertexData, interp4);
            insertVec3(m_vertexData, normalInterp4);
            insertVec3(m_vertexData, interp2);
            insertVec3(m_vertexData, normalInterp2);
        }
    }

    // Add the cone tip normals (reusing the base normals)
    for (int i = 0; i < m_param2; ++i) {
        int nextIndex = (i + 1) % m_param2;

        glm::vec3 base1 = baseVertices[i];
        glm::vec3 base2 = baseVertices[nextIndex];
        glm::vec3 normal1 = baseNormals[i];
        glm::vec3 normal2 = baseNormals[nextIndex];

        // Tip triangle (apex -> base2 -> base1)
        insertVec3(m_vertexData, apex);
        insertVec3(m_vertexData, normal1); // Use the same normal as base1
        insertVec3(m_vertexData, base2);
        insertVec3(m_vertexData, normal2);
        insertVec3(m_vertexData, base1);
        insertVec3(m_vertexData, normal1);
    }
}

void Cone::makeConeBase() {
    glm::vec3 center(0.0f, -0.5f, 0.0f); // Center of the base
    float radius = 0.5f;                 // Base radius
    glm::vec3 normal(0.0f, -1.0f, 0.0f); // Normal of the base

    // Loop over the concentric rings defined by param1
    for (int ring = 0; ring < m_param1; ++ring) {
        float currentRadius = radius * ring / m_param1;
        float nextRadius = radius * (ring + 1) / m_param1;

        // Loop to create quads or triangles between the rings
        for (int i = 0; i < m_param2; ++i) {
            float angle = 2.0f * M_PI * i / m_param2;
            float nextAngle = 2.0f * M_PI * (i + 1) / m_param2;

            // Define vertices on the current and next rings
            glm::vec3 v1(currentRadius * cos(angle), -0.5f, currentRadius * sin(angle));
            glm::vec3 v2(nextRadius * cos(angle), -0.5f, nextRadius * sin(angle));
            glm::vec3 v3(currentRadius * cos(nextAngle), -0.5f, currentRadius * sin(nextAngle));
            glm::vec3 v4(nextRadius * cos(nextAngle), -0.5f, nextRadius * sin(nextAngle));

            if (ring == 0) {
                // Centermost triangle fan
                insertVec3(m_vertexData, center);
                insertVec3(m_vertexData, normal);
                insertVec3(m_vertexData, v2);
                insertVec3(m_vertexData, normal);
                insertVec3(m_vertexData, v4);
                insertVec3(m_vertexData, normal);
            } else {
                // Outer quads split into two triangles
                // Triangle 1: v1 -> v2 -> v3
                insertVec3(m_vertexData, v1);
                insertVec3(m_vertexData, normal);
                insertVec3(m_vertexData, v2);
                insertVec3(m_vertexData, normal);
                insertVec3(m_vertexData, v3);
                insertVec3(m_vertexData, normal);

                // Triangle 2: v3 -> v2 -> v4
                insertVec3(m_vertexData, v3);
                insertVec3(m_vertexData, normal);
                insertVec3(m_vertexData, v2);
                insertVec3(m_vertexData, normal);
                insertVec3(m_vertexData, v4);
                insertVec3(m_vertexData, normal);
            }
        }
    }
}

void Cone::insertVec3(std::vector<float> &data, glm::vec3 v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}
