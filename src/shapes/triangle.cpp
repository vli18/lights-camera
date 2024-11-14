#include "Triangle.h"

void Triangle::updateParams() {
    m_vertexData = std::vector<float>();
    setVertexData();
}

void Triangle::setVertexData() {
    // Task 1: update m_vertexData with the vertices and normals
    //         needed to tesselate a triangle
    // Note: you may find the insertVec3 function useful in adding your points into m_vertexData
    glm::vec3 v1(-0.5, 0.5, 0.0);
    glm::vec3 v2(-0.5, -0.5, 0.0);
    glm::vec3 v3(0.5, -0.5, 0.0);

    glm::vec3 normal = glm::normalize(glm::cross(v2 - v1, v3 - v1));

    insertVec3(m_vertexData, v1);
    insertVec3(m_vertexData, normal);

    insertVec3(m_vertexData, v2);
    insertVec3(m_vertexData, normal);

    insertVec3(m_vertexData, v3);
    insertVec3(m_vertexData, normal);
}

// Inserts a glm::vec3 into a vector of floats.
// This will come in handy if you want to take advantage of vectors to build your shape!
void Triangle::insertVec3(std::vector<float> &data, glm::vec3 v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}
