#include "Sphere.h"
#include "glm/ext/scalar_constants.hpp"

void Sphere::updateParams(int param1, int param2) {
    m_vertexData = std::vector<float>();
    m_param1 = param1;
    m_param2 = param2;
    setVertexData();
}

void Sphere::makeTile(glm::vec3 topLeft,
                      glm::vec3 topRight,
                      glm::vec3 bottomLeft,
                      glm::vec3 bottomRight) {
    // Task 5: Implement the makeTile() function for a Sphere
    // Note: this function is very similar to the makeTile() function for Cube,
    //       but the normals are calculated in a different way!
    // Triangle 1
    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, glm::normalize(topLeft));
    insertVec3(m_vertexData, bottomLeft);
    insertVec3(m_vertexData, glm::normalize(bottomLeft));
    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, glm::normalize(bottomRight));

    // Triangle 2
    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, glm::normalize(bottomRight));
    insertVec3(m_vertexData, topRight);
    insertVec3(m_vertexData, glm::normalize(topRight));
    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, glm::normalize(topLeft));
}

void Sphere::makeWedge(float currentTheta, float nextTheta) {
    float phiStep = glm::pi<float>() / m_param1;
    for (int segment = 0; segment < m_param1; ++segment) {
        float currentPhi = segment * phiStep;
        float nextPhi = (segment + 1) * phiStep;

        glm::vec3 topRight = glm::vec3(0.5f * sin(currentPhi) * cos(currentTheta), 0.5f * cos(currentPhi), 0.5f * sin(currentPhi) * sin(currentTheta));
        glm::vec3 topLeft = glm::vec3(0.5f * sin(currentPhi) * cos(nextTheta), 0.5f * cos(currentPhi), 0.5f * sin(currentPhi) * sin(nextTheta));
        glm::vec3 bottomRight = glm::vec3(0.5f * sin(nextPhi) * cos(currentTheta), 0.5f * cos(nextPhi), 0.5f * sin(nextPhi) * sin(currentTheta));
        glm::vec3 bottomLeft = glm::vec3(0.5f * sin(nextPhi) * cos(nextTheta), 0.5f * cos(nextPhi), 0.5f * sin(nextPhi) * sin(nextTheta));

        makeTile(topLeft, topRight, bottomLeft, bottomRight);
    }
}

void Sphere::makeSphere() {
    float thetaStep = 2 * glm::pi<float>() / m_param2;
    for (int wedge = 0; wedge < m_param2; ++wedge) {
        float currentTheta = wedge * thetaStep;
        float nextTheta = (wedge + 1) * thetaStep;
        makeWedge(currentTheta, nextTheta);
    }
}

void Sphere::setVertexData() {
    // Uncomment these lines to make a wedge for Task 6, then comment them out for Task 7:

    //     float thetaStep = glm::radians(360.f / m_param2);
    //     float currentTheta = 0 * thetaStep;
    //     float nextTheta = 1 * thetaStep;
    //     makeWedge(currentTheta, nextTheta);

    // Uncomment these lines to make sphere for Task 7:

    makeSphere();
}

// Inserts a glm::vec3 into a vector of floats.
// This will come in handy if you want to take advantage of vectors to build your shape!
void Sphere::insertVec3(std::vector<float> &data, glm::vec3 v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}
