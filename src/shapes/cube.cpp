#include "Cube.h"

void Cube::updateParams(int param1) {
    m_vertexData = std::vector<float>();
    m_param1 = param1;
    setVertexData();
}

void Cube::makeTile(glm::vec3 topLeft,
                    glm::vec3 topRight,
                    glm::vec3 bottomLeft,
                    glm::vec3 bottomRight) {
    // Task 2: create a tile (i.e. 2 triangles) based on 4 given points.

    // triangle 1
    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, glm::normalize(glm::cross(bottomLeft - topLeft, bottomRight - topLeft)));
    insertVec3(m_vertexData, bottomLeft);
    insertVec3(m_vertexData, glm::normalize(glm::cross(bottomRight - bottomLeft, topLeft - bottomLeft)));
    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, glm::normalize(glm::cross(topLeft - bottomRight, bottomLeft - bottomRight)));

    // triangle 2
    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, glm::normalize(glm::cross(topRight - bottomRight, topLeft - bottomRight)));
    insertVec3(m_vertexData, topRight);
    insertVec3(m_vertexData, glm::normalize(glm::cross(topLeft - topRight, bottomRight - topRight)));
    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, glm::normalize(glm::cross(bottomRight - topLeft, topRight - topLeft)));
}

void Cube::makeFace(glm::vec3 topLeft, glm::vec3 topRight, glm::vec3 bottomLeft, glm::vec3 bottomRight) {
    float tileSize = 1.0f / m_param1;

    // Calculate direction vectors for rows and columns, adjust these to correct the face orientation
    glm::vec3 rowDir = (topRight - topLeft) * tileSize; // Revert to original rowDir
    glm::vec3 colDir = (bottomLeft - topLeft) * tileSize; // Revert to original colDir

    for (int row = 0; row < m_param1; row++) {
        for (int col = 0; col < m_param1; col++) {
            glm::vec3 tileTopLeft = topLeft + colDir * static_cast<float>(row) + rowDir * static_cast<float>(col);
            glm::vec3 tileTopRight = topLeft + colDir * static_cast<float>(row) + rowDir * static_cast<float>(col + 1);
            glm::vec3 tileBottomLeft = topLeft + colDir * static_cast<float>(row + 1) + rowDir * static_cast<float>(col);
            glm::vec3 tileBottomRight = topLeft + colDir * static_cast<float>(row + 1) + rowDir * static_cast<float>(col + 1);

            // Switch the order of vertices in makeTile to reverse the face orientation
            makeTile(tileTopLeft, tileTopRight, tileBottomLeft, tileBottomRight);
        }
    }
}



void Cube::setVertexData() {
    // Task 4: Use the makeFace() function to make all 6 sides of the cube

    glm::vec3 ooo = glm::vec3(-0.5f, -0.5f, -0.5f);
    glm::vec3 ooi = glm::vec3(-0.5f, -0.5f, 0.5f);
    glm::vec3 oio = glm::vec3(-0.5f, 0.5f, -0.5f);
    glm::vec3 oii = glm::vec3(-0.5f, 0.5f, 0.5f);
    glm::vec3 ioo = glm::vec3( 0.5f, -0.5f, -0.5f);
    glm::vec3 ioi = glm::vec3( 0.5f, -0.5f, 0.5f);
    glm::vec3 iio = glm::vec3( 0.5f, 0.5f, -0.5f);
    glm::vec3 iii = glm::vec3( 0.5f, 0.5f, 0.5f);

    // front face
    makeFace(oii, iii, ooi, ioi);
    // right face
    makeFace(iii, iio, ioi, ioo);
    // back face
    makeFace(iio, oio, ioo, ooo);
    // left face
    makeFace(oio, oii, ooo, ooi);
    // top face
    makeFace(oio, iio, oii, iii);
    // back face
    makeFace(ooi, ioi, ooo, ioo);
}

// Inserts a glm::vec3 into a vector of floats.
// This will come in handy if you want to take advantage of vectors to build your shape!
void Cube::insertVec3(std::vector<float> &data, glm::vec3 v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}
