#include "sceneparser.h"
#include "scenefilereader.h"
#include <glm/gtx/transform.hpp>

#include <chrono>
#include <iostream>

void traverseDFS(const SceneNode* node, const glm::mat4 &currentTransform, RenderData &renderData) {
    if (node == nullptr) {
        return;
    }
    glm::mat4 ctm = currentTransform;
    //building cummulative transform
    for (const auto& transformation : node->transformations) {
        switch (transformation->type) {
        case TransformationType::TRANSFORMATION_TRANSLATE:
            ctm = glm::translate(ctm, transformation->translate);
            break;
        case TransformationType::TRANSFORMATION_SCALE:
            ctm = glm::scale(ctm, transformation->scale);
            break;
        case TransformationType::TRANSFORMATION_ROTATE:
            ctm = glm::rotate(ctm, transformation->angle, transformation->rotate);
            break;
        case TransformationType::TRANSFORMATION_MATRIX:
            ctm = ctm * transformation->matrix;
            break;
        default:
            break;
        }
    }
    // constructing RenderShapeData object using the primitive and its corresponding CTM
    // append the RenderShapeData onto renderData.shapes
    for (const auto &primitive : node->primitives) {
        RenderShapeData shapeData;
        shapeData.primitive = *primitive;
        shapeData.ctm = ctm;
        renderData.shapes.push_back(shapeData);
    }
    // constructing SceneLightData object for renderData.lights
    for (const auto &light : node->lights) {
        SceneLightData lightData;
        lightData.id = light->id;
        lightData.type = light->type;
        lightData.color = light->color;
        lightData.function = light->function;
        lightData.dir = ctm * light->dir;
        switch (light->type) {
        case LightType::LIGHT_DIRECTIONAL:
            lightData.dir = glm::normalize(ctm * light->dir);
            break;
        case LightType::LIGHT_POINT:
            lightData.pos = ctm * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
            break;
        case LightType::LIGHT_SPOT:
            lightData.pos = ctm * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
            lightData.dir = glm::normalize(ctm * light->dir);
            lightData.penumbra = light->penumbra;
            lightData.angle = light->angle;
            break;
        default:
            break;
        }
        renderData.lights.push_back(lightData);
    }
    // recursively descending tree
    for (const SceneNode* child : node->children) {
        traverseDFS(child, ctm, renderData);
    }
}

bool SceneParser::parse(std::string filepath, RenderData &renderData) {
    ScenefileReader fileReader = ScenefileReader(filepath);
    bool success = fileReader.readJSON();
    if (!success) {
        return false;
    }

    // TODO: Use your Lab 5 code here
    renderData.globalData = fileReader.getGlobalData();
    renderData.cameraData = fileReader.getCameraData();

    SceneNode* rootNode = fileReader.getRootNode();
    glm::mat4 identity = glm::mat4(1.0f); //initialize identity matrix

    renderData.shapes.clear();
    renderData.lights.clear();

    traverseDFS(rootNode, identity, renderData);

    return true;
}
