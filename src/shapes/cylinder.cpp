#include "Cylinder.h"

void Cylinder::updateParams(int param1, int param2) {
    m_vertexData = std::vector<float>();
    m_param1 = param1;
    m_param2 = param2;
    setVertexData();
}

void Cylinder::setVertexData() {
    // TODO for Project 5: Lights, Camera
    // ensure that the tessellation parameters are not too low
    m_param1 = std::max(m_param1, 1);
    m_param2 = std::max(m_param2, 3);

    // generate the sides
    makeSides();

    // generate the top and bottom caps
    makeCap(glm::vec3(0.0f, 0.5f, 0.0f), true);  // Top cap
    makeCap(glm::vec3(0.0f, -0.5f, 0.0f), false); // Bottom cap
}

// Inserts a glm::vec3 into a vector of floats.
// This will come in handy if you want to take advantage of vectors to build your shape!
void Cylinder::insertVec3(std::vector<float> &data, glm::vec3 v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}

void Cylinder::makeSides() {
    float radius = 0.5f;
    float height = 1.0f;

    for (int i = 0; i < m_param2; ++i) {
        float angle = 2.0f * M_PI * i / m_param2;
        float nextAngle = 2.0f * M_PI * (i + 1) / m_param2;

        for (int j = 0; j < m_param1; ++j) {
            float y = -0.5f + height * j / m_param1;
            float nextY = -0.5f + height * (j + 1) / m_param1;

            // vertices
            glm::vec3 v1(radius * cos(angle), y, radius * sin(angle));
            glm::vec3 v2(radius * cos(nextAngle), y, radius * sin(nextAngle));
            glm::vec3 v3(radius * cos(angle), nextY, radius * sin(angle));
            glm::vec3 v4(radius * cos(nextAngle), nextY, radius * sin(nextAngle));

            // calculate normals
            glm::vec3 normal1 = glm::normalize(glm::vec3(cos(angle), 0, sin(angle)));
            glm::vec3 normal2 = glm::normalize(glm::vec3(cos(nextAngle), 0, sin(nextAngle)));

            // first triangle
            insertVec3(m_vertexData, v1);
            insertVec3(m_vertexData, normal1);
            insertVec3(m_vertexData, v3);
            insertVec3(m_vertexData, normal1);
            insertVec3(m_vertexData, v2);
            insertVec3(m_vertexData, normal2);

            // second triangle
            insertVec3(m_vertexData, v3);
            insertVec3(m_vertexData, normal1);
            insertVec3(m_vertexData, v4);
            insertVec3(m_vertexData, normal2);
            insertVec3(m_vertexData, v2);
            insertVec3(m_vertexData, normal2);
        }
    }
}

void Cylinder::makeCap(const glm::vec3& center, bool isTopCap) {
    float radius = 0.5f;
    glm::vec3 normal = isTopCap ? glm::vec3(0.0f, 1.0f, 0.0f) : glm::vec3(0.0f, -1.0f, 0.0f);

    // For param1 = 1, create a single triangle fan from the center
    if (m_param1 == 1) {
        for (int i = 0; i < m_param2; ++i) {
            float angle = 2.0f * M_PI * i / m_param2;
            float nextAngle = 2.0f * M_PI * (i + 1) / m_param2;

            glm::vec3 v1 = center + glm::vec3(radius * cos(angle), 0, radius * sin(angle));
            glm::vec3 v2 = center + glm::vec3(radius * cos(nextAngle), 0, radius * sin(nextAngle));

            if (isTopCap) {
                // Top center triangle
                insertVec3(m_vertexData, center);
                insertVec3(m_vertexData, normal);
                insertVec3(m_vertexData, v2);
                insertVec3(m_vertexData, normal);
                insertVec3(m_vertexData, v1);
                insertVec3(m_vertexData, normal);
            } else {
                // Bottom center triangle
                insertVec3(m_vertexData, center);
                insertVec3(m_vertexData, normal);
                insertVec3(m_vertexData, v1);
                insertVec3(m_vertexData, normal);
                insertVec3(m_vertexData, v2);
                insertVec3(m_vertexData, normal);
            }
        }
    } else {
        for (int ring = 0; ring < m_param1; ++ring) {
            float currentRadius = radius * ring / m_param1;
            float nextRadius = radius * (ring + 1) / m_param1;

            for (int i = 0; i < m_param2; ++i) {
                float angle = 2.0f * M_PI * i / m_param2;
                float nextAngle = 2.0f * M_PI * (i + 1) / m_param2;

                // Define vertices of the quad on the cap
                glm::vec3 v1 = center + glm::vec3(currentRadius * cos(angle), 0, currentRadius * sin(angle));
                glm::vec3 v2 = center + glm::vec3(nextRadius * cos(angle), 0, nextRadius * sin(angle));
                glm::vec3 v3 = center + glm::vec3(currentRadius * cos(nextAngle), 0, currentRadius * sin(nextAngle));
                glm::vec3 v4 = center + glm::vec3(nextRadius * cos(nextAngle), 0, nextRadius * sin(nextAngle));

                if (isTopCap) {
                    // Top cap triangles (counter-clockwise from above)
                    if (ring == 0) {
                        glm::vec3 v1 = center + glm::vec3(nextRadius * cos(angle), 0, nextRadius * sin(angle));
                        glm::vec3 v2 = center + glm::vec3(nextRadius * cos(nextAngle), 0, nextRadius * sin(nextAngle));

                        insertVec3(m_vertexData, center);
                        insertVec3(m_vertexData, normal);
                        insertVec3(m_vertexData, v2);
                        insertVec3(m_vertexData, normal);
                        insertVec3(m_vertexData, v1);
                        insertVec3(m_vertexData, normal);
                    } else {
                        // Outer triangles
                        insertVec3(m_vertexData, v2);
                        insertVec3(m_vertexData, normal);
                        insertVec3(m_vertexData, v1);
                        insertVec3(m_vertexData, normal);
                        insertVec3(m_vertexData, v4);
                        insertVec3(m_vertexData, normal);

                        insertVec3(m_vertexData, v4);
                        insertVec3(m_vertexData, normal);
                        insertVec3(m_vertexData, v1);
                        insertVec3(m_vertexData, normal);
                        insertVec3(m_vertexData, v3);
                        insertVec3(m_vertexData, normal);
                    }
                } else {
                    // Bottom cap triangles (counter-clockwise from below)
                    if (ring == 0) {
                        // Centermost triangle
                        glm::vec3 v1 = center + glm::vec3(nextRadius * cos(angle), 0, nextRadius * sin(angle));
                        glm::vec3 v2 = center + glm::vec3(nextRadius * cos(nextAngle), 0, nextRadius * sin(nextAngle));

                        insertVec3(m_vertexData, center);
                        insertVec3(m_vertexData, normal);
                        insertVec3(m_vertexData, v1);
                        insertVec3(m_vertexData, normal);
                        insertVec3(m_vertexData, v2);
                        insertVec3(m_vertexData, normal);
                    } else {
                        // Outer triangles
                        insertVec3(m_vertexData, v1);
                        insertVec3(m_vertexData, normal);
                        insertVec3(m_vertexData, v2);
                        insertVec3(m_vertexData, normal);
                        insertVec3(m_vertexData, v4);
                        insertVec3(m_vertexData, normal);

                        insertVec3(m_vertexData, v1);
                        insertVec3(m_vertexData, normal);
                        insertVec3(m_vertexData, v4);
                        insertVec3(m_vertexData, normal);
                        insertVec3(m_vertexData, v3);
                        insertVec3(m_vertexData, normal);
                    }
                }
            }
        }
    }
}
