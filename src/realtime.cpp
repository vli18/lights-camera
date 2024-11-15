#include "realtime.h"

#include <QCoreApplication>
#include <QMouseEvent>
#include <QKeyEvent>
#include <iostream>
#include "settings.h"
#include "utils/shaderloader.h"
#include "camera/camera.h"
#include "shapes/cone.h"
#include "shapes/cube.h"
#include "shapes/cylinder.h"
#include "shapes/sphere.h"

// ================== Project 5: Lights, Camera

RenderData sceneData;
Camera camera(sceneData.cameraData, 0, 0);
std::vector<GLuint> vaos(4);
std::vector<GLuint> vbos(4);

Realtime::Realtime(QWidget *parent)
    : QOpenGLWidget(parent)
{
    m_prev_mouse_pos = glm::vec2(size().width()/2, size().height()/2);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    m_keyMap[Qt::Key_W]       = false;
    m_keyMap[Qt::Key_A]       = false;
    m_keyMap[Qt::Key_S]       = false;
    m_keyMap[Qt::Key_D]       = false;
    m_keyMap[Qt::Key_Control] = false;
    m_keyMap[Qt::Key_Space]   = false;

    // If you must use this function, do not edit anything above this
}

/** Helper Functions **/

void Realtime::setUpLights(std::string filepath, RenderData &renderData) {
    SceneParser::parse(settings.sceneFilePath, sceneData);

    m_ka = sceneData.globalData.ka;
    m_kd = sceneData.globalData.kd;
    m_ks = sceneData.globalData.ks;

    lightTypes.clear();
    lightDirs.clear();
    lightColors.clear();
    lightPos.clear();
    functions.clear();
    angles.clear();
    penumbras.clear();

    for (auto light : sceneData.lights) {
        switch (light.type) {
        case LightType::LIGHT_DIRECTIONAL: {
            lightTypes.push_back(0);
            lightDirs.push_back(light.dir);
            lightColors.push_back(light.color);
            lightPos.push_back(glm::vec4(999, 999, 999, 999));
            functions.push_back(glm::vec3(1.0f, 0.f, 0.f));
            angles.push_back(0.f);
            penumbras.push_back(0.f);
            break;
        }
        case LightType::LIGHT_POINT: {
            lightTypes.push_back(1);
            lightDirs.push_back(glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));
            lightColors.push_back(light.color);
            lightPos.push_back(light.pos);
            functions.push_back(light.function);
            angles.push_back(0.f);
            penumbras.push_back(0.f);
            break;
        }
        case LightType::LIGHT_SPOT: {
            lightTypes.push_back(2);
            lightDirs.push_back(light.dir);
            lightColors.push_back(light.color);
            lightPos.push_back(light.pos);
            functions.push_back(light.function);
            angles.push_back(light.angle);
            penumbras.push_back(light.penumbra);
            break;
        }
        default:
            break;
        }
    }
}

void Realtime::setUpShapes() {
    // Shape VAO/VBO generation
    Cube cube{};
    cube.updateParams(settings.shapeParameter1);
    std::vector<float> cubeVerts = cube.generateShape();

    Cone cone{};
    cone.updateParams(settings.shapeParameter1, settings.shapeParameter2);
    std::vector<float> coneVerts = cone.generateShape();

    Cylinder cylinder{};
    cylinder.updateParams(settings.shapeParameter1, settings.shapeParameter2);
    std::vector<float> cylinderVerts = cylinder.generateShape();

    Sphere sphere{};
    sphere.updateParams(settings.shapeParameter1, settings.shapeParameter2);
    std::vector<float> sphereVerts = sphere.generateShape();

    vertsList = {cubeVerts, coneVerts, cylinderVerts, sphereVerts};

    for (int i = 0; i < 4; i++) {
        glGenBuffers(1, &vbos[i]);
        glBindBuffer(GL_ARRAY_BUFFER, vbos[i]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertsList[i].size(), vertsList[i].data(), GL_STATIC_DRAW);

        glGenVertexArrays(1, &vaos[i]);
        glBindVertexArray(vaos[i]);

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(0, vertsList[i].size() / 2, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), nullptr);
        glVertexAttribPointer(1, vertsList[i].size() / 2, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void*>(3 * sizeof(GLfloat)));
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER,0);
    }
}

void Realtime::finish() {
    killTimer(m_timer);
    this->makeCurrent();

    // Students: anything requiring OpenGL calls when the program exits should be done here
    glDeleteProgram(m_shader);

    for (int i = 0; i < 4; i++) {
        glDeleteVertexArrays(1, &vaos[i]);
        glDeleteBuffers(1, &vbos[i]);
    }

    this->doneCurrent();
}

void Realtime::initializeGL() {
    m_devicePixelRatio = this->devicePixelRatio();

    m_timer = startTimer(1000/60);
    m_elapsedTimer.start();

    // Initializing GL.
    // GLEW (GL Extension Wrangler) provides access to OpenGL functions.
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "Error while initializing GL: " << glewGetErrorString(err) << std::endl;
    }
    std::cout << "Initialized GL: Version " << glewGetString(GLEW_VERSION) << std::endl;

    // Allows OpenGL to draw objects appropriately on top of one another
    glEnable(GL_DEPTH_TEST);
    // Tells OpenGL to only draw the front face
    glEnable(GL_CULL_FACE);
    // Tells OpenGL how big the screen is
    glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);

    // Students: anything requiring OpenGL calls when the program starts should be done here
    setUpLights(settings.sceneFilePath, sceneData);
    glClearColor(0,0,0,1);

    m_shader = ShaderLoader::createShaderProgram(":/resources/shaders/default.vert", ":/resources/shaders/default.frag");
    setUpShapes();

    initialized = true;
}

void Realtime::draw(RenderShapeData shape) {
    glm::vec4 cameraPos = camera.getData().pos;
    int numLights = sceneData.lights.size();

    PrimitiveType type = shape.primitive.type;
    GLuint vao;
    std::vector<float> verts;
    glm::mat4 ctm = shape.ctm;

    glm::vec4 cAmbient = shape.primitive.material.cAmbient;
    glm::vec4 cDiffuse = shape.primitive.material.cDiffuse;
    glm::vec4 cSpecular = shape.primitive.material.cSpecular;
    float shininess = shape.primitive.material.shininess;

    if (type == PrimitiveType::PRIMITIVE_CUBE) {
        vao = vaos[0];
        verts = vertsList[0];
    } else if (type == PrimitiveType::PRIMITIVE_CONE) {
        vao = vaos[1];
        verts = vertsList[1];
    } else if (type == PrimitiveType::PRIMITIVE_CYLINDER) {
        vao = vaos[2];
        verts = vertsList[2];
    } else if (type == PrimitiveType::PRIMITIVE_SPHERE) {
        vao = vaos[3];
        verts = vertsList[3];
    }

    glBindVertexArray(vao);
    glUseProgram(m_shader);

    // Camera
    GLint modelLoc = glGetUniformLocation(m_shader, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &ctm[0][0]);

    GLint viewLoc = glGetUniformLocation(m_shader, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &camera.getViewMatrix()[0][0]);

    GLint projLoc = glGetUniformLocation(m_shader, "proj");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, &camera.getProjMatrix()[0][0]);

    GLint cameraPosLoc = glGetUniformLocation(m_shader, "camera_pos");
    glUniform4f(cameraPosLoc, cameraPos[0], cameraPos[1], cameraPos[2], cameraPos[3]);

    // Global Properties
    GLint kaLoc = glGetUniformLocation(m_shader, "k_a");
    glUniform1f(kaLoc, m_ka);

    GLint kdLoc = glGetUniformLocation(m_shader, "k_d");
    glUniform1f(kdLoc, m_kd);

    GLint ksLoc = glGetUniformLocation(m_shader, "k_s");
    glUniform1f(ksLoc, m_ks);

    // Shape Properties
    GLint ambientLoc = glGetUniformLocation(m_shader, "cAmbient");
    glUniform4f(ambientLoc, cAmbient[0], cAmbient[1], cAmbient[2], cAmbient[3]);

    GLint diffuseLoc = glGetUniformLocation(m_shader, "cDiffuse");
    glUniform4f(diffuseLoc, cDiffuse[0], cDiffuse[1], cDiffuse[2], cDiffuse[3]);

    GLint specLoc = glGetUniformLocation(m_shader, "cSpecular");
    glUniform4f(specLoc, cSpecular[0], cSpecular[1], cSpecular[2], cSpecular[3]);

    GLint shininessLoc = glGetUniformLocation(m_shader, "shininess");
    glUniform1f(shininessLoc, shininess);

    // Lights
    GLint numLightsLoc = glGetUniformLocation(m_shader, "numLights");
    glUniform1i(numLightsLoc, numLights);

    for (int i = 0; i < sceneData.lights.size(); i++) {
        std::string str = "lightTypes[" + std::to_string(i) + "]";
        GLint lightTypesLoc = glGetUniformLocation(m_shader, str.c_str());
        glUniform1i(lightTypesLoc, lightTypes[i]);

        str = "lightPos[" + std::to_string(i) + "]";
        GLint lightPosLoc = glGetUniformLocation(m_shader, str.c_str());
        glUniform4f(lightPosLoc, lightPos[i][0],  lightPos[i][1],  lightPos[i][2],  lightPos[i][3]);

        str =  "lightColors[" + std::to_string(i) + "]";
        GLint lightColorsLoc= glGetUniformLocation(m_shader, str.c_str());
        glUniform4f(lightColorsLoc, lightColors[i][0], lightColors[i][1], lightColors[i][2], lightColors[i][3]);

        str =  "lightDirs[" + std::to_string(i) + "]";
        GLint lightDirsLoc = glGetUniformLocation(m_shader, str.c_str());
        glUniform4f(lightDirsLoc, lightDirs[i][0], lightDirs[i][1], lightDirs[i][2], lightDirs[i][3]);

        str =  "functions[" + std::to_string(i) + "]";
        GLint functionsLoc = glGetUniformLocation(m_shader, str.c_str());
        glUniform3f(functionsLoc, functions[i][0], functions[i][1], functions[i][2]);

        str =  "angles[" + std::to_string(i) + "]";
        GLint anglesLoc = glGetUniformLocation(m_shader, str.c_str());
        glUniform1f(anglesLoc, angles[i]);

        str =  "penumbras[" + std::to_string(i) + "]";
        GLint penumbrasLoc = glGetUniformLocation(m_shader, str.c_str());
        glUniform1f(penumbrasLoc, penumbras[i]);
    }

    glDrawArrays(GL_TRIANGLES, 0, verts.size() / 6);

    glBindVertexArray(0);
    glUseProgram(0);
}

void Realtime::paintGL() {
    // Students: anything requiring OpenGL calls every frame should be done here
    glViewport(0, 0, m_width*  m_devicePixelRatio, m_height * m_devicePixelRatio);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear buffers

    // Draw scene objects
    for (RenderShapeData &shape : sceneData.shapes) {
        draw(shape);
    }

    glViewport(0, 0, m_width*  m_devicePixelRatio, m_height * m_devicePixelRatio);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}


void Realtime::resizeGL(int w, int h) {
    // Tells OpenGL how big the screen is
    glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);

    // Students: anything requiring OpenGL calls when the program starts should be done here
    m_width = w;
    m_height = h;
    camera.updateWH(m_width, m_height);
}

void Realtime::sceneChanged() {
    sceneLoaded = true;
    update(); // asks for a PaintGL() call to occur

    // Reload the scene data
    setUpLights(settings.sceneFilePath, sceneData);

    // Update the camera data based on the scene's camera settings
    SceneCameraData cData = sceneData.cameraData;
    camera = Camera(cData, m_width, m_height);

//    // Trigger a redraw
//    update();
}

void Realtime::settingsChanged() {
    if (initialized) {
        setUpLights(settings.sceneFilePath, sceneData); // Update lights
        setUpShapes();
        update(); // asks for a PaintGL() call to occur
    }
}

// ================== Project 6: Action!

void Realtime::keyPressEvent(QKeyEvent *event) {
    m_keyMap[Qt::Key(event->key())] = true;
}

void Realtime::keyReleaseEvent(QKeyEvent *event) {
    m_keyMap[Qt::Key(event->key())] = false;
}

void Realtime::mousePressEvent(QMouseEvent *event) {
    if (event->buttons().testFlag(Qt::LeftButton)) {
        m_mouseDown = true;
        m_prev_mouse_pos = glm::vec2(event->position().x(), event->position().y());
    }
}

void Realtime::mouseReleaseEvent(QMouseEvent *event) {
    if (!event->buttons().testFlag(Qt::LeftButton)) {
        m_mouseDown = false;
    }
}

void Realtime::mouseMoveEvent(QMouseEvent *event) {
    if (m_mouseDown) {
        int posX = event->position().x();
        int posY = event->position().y();
        int deltaX = posX - m_prev_mouse_pos.x;
        int deltaY = posY - m_prev_mouse_pos.y;
        m_prev_mouse_pos = glm::vec2(posX, posY);

        // Use deltaX and deltaY here to rotate

        update(); // asks for a PaintGL() call to occur
    }
}

void Realtime::timerEvent(QTimerEvent *event) {
    int elapsedms   = m_elapsedTimer.elapsed();
    float deltaTime = elapsedms * 0.001f;
    m_elapsedTimer.restart();

    // Use deltaTime and m_keyMap here to move around

    update(); // asks for a PaintGL() call to occur
}

// DO NOT EDIT
void Realtime::saveViewportImage(std::string filePath) {
    // Make sure we have the right context and everything has been drawn
    makeCurrent();

    int fixedWidth = 1024;
    int fixedHeight = 768;

    // Create Frame Buffer
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Create a color attachment texture
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, fixedWidth, fixedHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    // Optional: Create a depth buffer if your rendering uses depth testing
    GLuint rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, fixedWidth, fixedHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Error: Framebuffer is not complete!" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return;
    }

    // Render to the FBO
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, fixedWidth, fixedHeight);

    // Clear and render your scene here
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    paintGL();

    // Read pixels from framebuffer
    std::vector<unsigned char> pixels(fixedWidth * fixedHeight * 3);
    glReadPixels(0, 0, fixedWidth, fixedHeight, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

    // Unbind the framebuffer to return to default rendering to the screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Convert to QImage
    QImage image(pixels.data(), fixedWidth, fixedHeight, QImage::Format_RGB888);
    QImage flippedImage = image.mirrored(); // Flip the image vertically

    // Save to file using Qt
    QString qFilePath = QString::fromStdString(filePath);
    if (!flippedImage.save(qFilePath)) {
        std::cerr << "Failed to save image to " << filePath << std::endl;
    }

    // Clean up
    glDeleteTextures(1, &texture);
    glDeleteRenderbuffers(1, &rbo);
    glDeleteFramebuffers(1, &fbo);
}
