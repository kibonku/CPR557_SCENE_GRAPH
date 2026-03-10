#include "my_application.h"

// Render factory
#include "my_simple_render_factory.h"
#include "my_camera.h"
#include "my_keyboard_controller.h"

// use radian rather degree for angle
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// Std
#include <stdexcept>
#include <array>
#include <chrono>
#include <iostream>

// ============================================================
//  Shape vertex helpers
// ============================================================

// --- Octahedron (8 triangles = 24 vertices) ---
static std::vector<MyModel::Vertex> makeOctahedron(glm::vec3 col)
{
    glm::vec3 T = { 0.0f,  1.0f,  0.0f };
    glm::vec3 B = { 0.0f, -1.0f,  0.0f };
    glm::vec3 F = { 0.0f,  0.0f,  1.0f };
    glm::vec3 K = { 0.0f,  0.0f, -1.0f };
    glm::vec3 R = { 1.0f,  0.0f,  0.0f };
    glm::vec3 L = {-1.0f,  0.0f,  0.0f };

    // Give each pair of opposite faces a distinct shade
    glm::vec3 c0 = col;
    glm::vec3 c1 = col * 0.85f;
    glm::vec3 c2 = col * 0.70f;
    glm::vec3 c3 = col * 0.55f;

    return {
        // Upper hemisphere
        {T, c0}, {F, c1}, {R, c1},
        {T, c0}, {R, c1}, {K, c2},
        {T, c0}, {K, c2}, {L, c1},
        {T, c0}, {L, c1}, {F, c3},
        // Lower hemisphere
        {B, c0}, {R, c1}, {F, c3},
        {B, c0}, {K, c2}, {R, c1},
        {B, c0}, {L, c1}, {K, c2},
        {B, c0}, {F, c1}, {L, c1},
    };
}

// --- Cube (6 faces × 2 triangles = 36 vertices) ---
static std::vector<MyModel::Vertex> makeCube(glm::vec3 col)
{
    float h = 0.5f;

    glm::vec3 v000 = {-h, -h, -h};
    glm::vec3 v100 = { h, -h, -h};
    glm::vec3 v110 = { h,  h, -h};
    glm::vec3 v010 = {-h,  h, -h};
    glm::vec3 v001 = {-h, -h,  h};
    glm::vec3 v101 = { h, -h,  h};
    glm::vec3 v111 = { h,  h,  h};
    glm::vec3 v011 = {-h,  h,  h};

    glm::vec3 c  = col;
    glm::vec3 cm = col * 0.80f; // mid shade
    glm::vec3 cd = col * 0.60f; // dark shade

    return {
        // Front  (z = +h)
        {v001, c},  {v101, c},  {v111, c},
        {v001, c},  {v111, c},  {v011, c},
        // Back   (z = -h)
        {v100, cd}, {v000, cd}, {v010, cd},
        {v100, cd}, {v010, cd}, {v110, cd},
        // Left   (x = -h)
        {v000, cm}, {v001, cm}, {v011, cm},
        {v000, cm}, {v011, cm}, {v010, cm},
        // Right  (x = +h)
        {v101, cm}, {v100, cm}, {v110, cm},
        {v101, cm}, {v110, cm}, {v111, cm},
        // Top    (y = +h)
        {v011, c},  {v111, c},  {v110, c},
        {v011, c},  {v110, c},  {v010, c},
        // Bottom (y = -h)
        {v000, cd}, {v100, cd}, {v101, cd},
        {v000, cd}, {v101, cd}, {v001, cd},
    };
}

// --- Pyramid (4 triangular sides + 2-triangle base = 18 vertices) ---
static std::vector<MyModel::Vertex> makePyramid(glm::vec3 col)
{
    glm::vec3 apex = { 0.0f, 1.0f,  0.0f };
    glm::vec3 fl   = {-0.5f, 0.0f,  0.5f }; // front-left  base corner
    glm::vec3 fr   = { 0.5f, 0.0f,  0.5f }; // front-right
    glm::vec3 br   = { 0.5f, 0.0f, -0.5f }; // back-right
    glm::vec3 bl   = {-0.5f, 0.0f, -0.5f }; // back-left

    glm::vec3 ct = col;
    glm::vec3 cb = col * 0.65f;

    return {
        // 4 triangular side faces
        {apex, ct}, {fl, cb}, {fr, cb},   // front
        {apex, ct}, {fr, cb}, {br, cb},   // right
        {apex, ct}, {br, cb}, {bl, cb},   // back
        {apex, ct}, {bl, cb}, {fl, cb},   // left
        // Base (2 triangles)
        {fl, cb}, {br, cb}, {fr, cb},
        {fl, cb}, {bl, cb}, {br, cb},
    };
}

// --- Diamond / Double-pyramid: two octants of an octahedron, squished flat ---
// Used for the satellite shape
static std::vector<MyModel::Vertex> makeDiamond(glm::vec3 col)
{
    glm::vec3 T = { 0.0f,  0.4f,  0.0f };
    glm::vec3 B = { 0.0f, -0.4f,  0.0f };
    glm::vec3 F = { 0.0f,  0.0f,  1.0f };
    glm::vec3 K = { 0.0f,  0.0f, -1.0f };
    glm::vec3 R = { 1.0f,  0.0f,  0.0f };
    glm::vec3 L = {-1.0f,  0.0f,  0.0f };

    glm::vec3 c0 = col;
    glm::vec3 c1 = col * 0.75f;

    return {
        {T, c0}, {F, c1}, {R, c1},
        {T, c0}, {R, c1}, {K, c1},
        {T, c0}, {K, c1}, {L, c1},
        {T, c0}, {L, c1}, {F, c1},
        {B, c0}, {R, c1}, {F, c1},
        {B, c0}, {K, c1}, {R, c1},
        {B, c0}, {L, c1}, {K, c1},
        {B, c0}, {F, c1}, {L, c1},
    };
}

// ============================================================
//  MyApplication
// ============================================================

MyApplication::MyApplication() :
    m_bMoveCamera(true),
    m_bPerspectiveProjection(true)
{
    _loadGameObjects();
}

void MyApplication::run() 
{
    _printControls();
    m_myWindow.bindMyApplication(this);
    MySimpleRenderFactory simpleRenderFactory{ m_myDevice, m_myRenderer.swapChainRenderPass() };
    MyCamera camera{};

    // Empty object used to drive the camera
    auto viewerObject = MyGameObject::createGameObject("CameraObject");

    // Initial camera position: pull back on -Z so all scene objects are visible
    viewerObject.transform.translation = glm::vec3(0.0f, -1.0f, 8.0f);

    MyKeyboardController cameraController{};

    auto currentTime = std::chrono::high_resolution_clock::now();

    while (!m_myWindow.shouldClose()) 
    {
        m_myWindow.pollEvents();

        auto newTime = std::chrono::high_resolution_clock::now();
        float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
        currentTime = newTime;

        if (m_bMoveCamera)
        {
            cameraController.moveInPlaneXZ(m_myWindow, frameTime, viewerObject);
            camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);
        }
        else
        {
            // Phase 4 + Phase 5: move / animate current scene graph node
            _moveGameObject(m_pCurrentSceneGraphNode);
        }

        float aspectRatio = m_myRenderer.aspectRatio();

        if (m_bPerspectiveProjection)
            camera.setPerspectiveProjection(glm::radians(50.f), aspectRatio, 0.1f, 100.f);
        else
            camera.setOrthographicProjection(
                -aspectRatio * 6.0f, aspectRatio * 6.0f,
                -6.0f, 6.0f,
                -20.0f, 20.0f);

        if (auto commandBuffer = m_myRenderer.beginFrame())
		{
            m_myRenderer.beginSwapChainRenderPass(commandBuffer);
            simpleRenderFactory.renderSceneGraph(commandBuffer, m_pMySeceneGraphRoot, camera);
            m_myRenderer.endSwapChainRenderPass(commandBuffer);
            m_myRenderer.endFrame();
        }
    }

    vkDeviceWaitIdle(m_myDevice.device());
}

// ============================================================
//  _loadGameObjects  (Phase 1 + Phase 2)
//
//  Scene graph layout:
//
//  Root
//    Group1 "SolarSystem"  (translated left)
//      Object1  "Sun"      - large yellow octahedron
//      Object2  "Mercury"  - small gray cube, offset right of Sun
//    Group2 "Architecture" (translated right)
//      Object3  "Pyramid"  - red/orange pyramid
//      Object4  "Obelisk"  - white tall cube (scaled thin+tall)
//    Group3 "Spacecraft"   (translated up, Phase 5 extra group)
//      Object5  "Rocket"   - cyan pyramid pointing up
//      Object6  "Satellite"- magenta flat diamond
// ============================================================
void MyApplication::_loadGameObjects()
{
    m_pMySeceneGraphRoot = std::make_shared<MySceneGraphNode>("Root", 0);

    // ---- Group nodes ----
    auto group1 = std::make_shared<MySceneGraphNode>("SolarSystem", 1);
    auto group2 = std::make_shared<MySceneGraphNode>("Architecture", 2);
    auto group3 = std::make_shared<MySceneGraphNode>("Spacecraft", 3); // Phase 5

    group1->transform.translation = glm::vec3(-4.0f, -0.5f, 0.0f);
    group2->transform.translation = glm::vec3( 4.0f, -0.5f, 0.0f);
    group3->transform.translation = glm::vec3( 0.0f,  3.5f, 0.0f); // Phase 5 group on top

    // ---- Leaf node: Object1 - Sun (yellow octahedron) ----
    auto obj1 = std::make_shared<MySceneGraphNode>("Sun", 4);
    obj1->transform.translation = glm::vec3(0.0f, 0.0f, 0.0f);
    obj1->transform.scale       = glm::vec3(0.9f, 0.9f, 0.9f);
    {
        auto verts = makeOctahedron({1.0f, 0.85f, 0.1f}); // warm yellow
        obj1->model = std::make_shared<MyModel>(m_myDevice, verts);
    }

    // ---- Leaf node: Object2 - Mercury (small gray cube) ----
    auto obj2 = std::make_shared<MySceneGraphNode>("Mercury", 5);
    obj2->transform.translation = glm::vec3(2.0f, 0.0f, 0.0f); // orbit distance from Sun
    obj2->transform.scale       = glm::vec3(0.28f, 0.28f, 0.28f);
    {
        auto verts = makeCube({0.65f, 0.65f, 0.65f}); // gray
        obj2->model = std::make_shared<MyModel>(m_myDevice, verts);
    }

    // ---- Leaf node: Object3 - Pyramid (red/orange) ----
    auto obj3 = std::make_shared<MySceneGraphNode>("Pyramid", 6);
    obj3->transform.translation = glm::vec3(-1.0f, -0.5f, 0.0f);
    obj3->transform.scale       = glm::vec3(0.9f, 1.1f, 0.9f);
    {
        auto verts = makePyramid({0.95f, 0.35f, 0.1f}); // deep orange-red
        obj3->model = std::make_shared<MyModel>(m_myDevice, verts);
    }

    // ---- Leaf node: Object4 - Obelisk (white, tall thin cube) ----
    auto obj4 = std::make_shared<MySceneGraphNode>("Obelisk", 7);
    obj4->transform.translation = glm::vec3( 1.2f, -0.5f, 0.0f);
    obj4->transform.scale       = glm::vec3(0.22f, 1.6f, 0.22f); // tall and thin
    {
        auto verts = makeCube({0.92f, 0.92f, 0.86f}); // off-white
        obj4->model = std::make_shared<MyModel>(m_myDevice, verts);
    }

    // ---- Leaf node: Object5 - Rocket (cyan pyramid, Phase 5) ----
    auto obj5 = std::make_shared<MySceneGraphNode>("Rocket", 8);
    obj5->transform.translation = glm::vec3(-1.5f, 0.0f, 0.0f);
    obj5->transform.scale       = glm::vec3(0.4f, 0.7f, 0.4f);
    {
        auto verts = makePyramid({0.2f, 0.9f, 0.95f}); // cyan
        obj5->model = std::make_shared<MyModel>(m_myDevice, verts);
    }

    // ---- Leaf node: Object6 - Satellite (magenta flat diamond, Phase 5) ----
    auto obj6 = std::make_shared<MySceneGraphNode>("Satellite", 9);
    obj6->transform.translation = glm::vec3( 1.5f, 0.0f, 0.0f);
    obj6->transform.scale       = glm::vec3(0.6f, 0.6f, 0.6f);
    {
        auto verts = makeDiamond({0.95f, 0.2f, 0.75f}); // magenta/pink
        obj6->model = std::make_shared<MyModel>(m_myDevice, verts);
    }

    // ---- Build the scene graph ----
    m_pMySeceneGraphRoot->addChild(group1);
    m_pMySeceneGraphRoot->addChild(group2);
    m_pMySeceneGraphRoot->addChild(group3);

    group1->addChild(obj1);
    group1->addChild(obj2);

    group2->addChild(obj3);
    group2->addChild(obj4);

    group3->addChild(obj5); // Phase 5
    group3->addChild(obj6); // Phase 5
}

// ============================================================
//  Interaction
// ============================================================

void MyApplication::_printControls()
{
    std::cout << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "  Assignment 3 - Scene Graph Controls   " << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "  CAMERA MODE (default)                 " << std::endl;
    std::cout << "    W / S        Move forward / backward" << std::endl;
    std::cout << "    A / D        Strafe left / right    " << std::endl;
    std::cout << "    E / Q        Move up / down         " << std::endl;
    std::cout << "    Arrow keys   Look around            " << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "  SCENE GRAPH                           " << std::endl;
    std::cout << "    N            Select next node       " << std::endl;
    std::cout << "    P            Print scene graph      " << std::endl;
    std::cout << "    SPACE        Toggle camera/node mode" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "  NODE MODE (after pressing SPACE)      " << std::endl;
    std::cout << "    Arrow keys   Move selected node     " << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "  OTHER                                 " << std::endl;
    std::cout << "    C            Toggle perspective/ortho" << std::endl;
    std::cout << "    ESC          Quit                   " << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
}

void MyApplication::switchProjectionMatrix()
{
    m_bPerspectiveProjection = !m_bPerspectiveProjection;

    if (m_bPerspectiveProjection)
        std::cout << "Switch to perspective camera mode" << std::endl;
    else
        std::cout << "Switch to orthographic camera mode" << std::endl;
}

void MyApplication::switchInteractionMode()
{
    m_bMoveCamera = !m_bMoveCamera;

    if (m_bMoveCamera)
        std::cout << "Switch to: Move Camera mode (WASD + arrow keys)" << std::endl;
    else
        std::cout << "Switch to: Move Scene Graph Node mode (arrow keys move current node)" << std::endl;
}

// ============================================================
//  handleMovement  (Phase 4)
//  Called from MyWindow::pollEvents() every frame a key is held.
//  Only moves the selected node when in object-move mode.
// ============================================================
void MyApplication::handleMovement(MyAppKeyMap key)
{
    if (m_bMoveCamera)
    {
        // Arrow keys are used by camera controller in camera mode - nothing extra needed here
        return;
    }

    if (m_pCurrentSceneGraphNode == nullptr)
    {
        std::cout << "[Move] No node selected. Press N to select a node first." << std::endl;
        return;
    }

    const float step = 0.04f; // translation per frame while key is held

    if      (key == KEY_LEFT)  m_pCurrentSceneGraphNode->transform.translation.x -= step;
    else if (key == KEY_RIGHT) m_pCurrentSceneGraphNode->transform.translation.x += step;
    else if (key == KEY_UP)    m_pCurrentSceneGraphNode->transform.translation.y += step; // Y is up
    else if (key == KEY_DOWN)  m_pCurrentSceneGraphNode->transform.translation.y -= step;
}

// ============================================================
//  _moveGameObject  (Phase 4 / Phase 5)
//  Called every frame when in object-move mode.
//
//  Phase 5 creativity:
//    The currently selected node continuously rotates around its Y-axis,
//    providing clear visual feedback about which node is "active".
//    If a group node is selected, the whole group (and all children) rotates.
// ============================================================
void MyApplication::_moveGameObject(MySceneGraphNode* pobj)
{
    if (pobj == nullptr) return;

    // Phase 5: spin the selected node around Y for visual indication
    pobj->transform.rotation.y = glm::mod(
        pobj->transform.rotation.y + 0.015f,
        glm::two_pi<float>());
}

// ============================================================
//  printSceneGraph  (Phase 2 + Phase 3)
// ============================================================
void MyApplication::printSceneGraph()
{
    std::cout << "\n--- Scene Graph ---" << std::endl;

    if (m_pMySeceneGraphRoot)
    {
        m_pMySeceneGraphRoot->printSceneGraph();
    }

    std::cout << "-------------------" << std::endl;
}

// ============================================================
//  traverseNext  (Phase 3)
// ============================================================
void MyApplication::traverseNext()
{
    if (m_pMySeceneGraphRoot)
    {
        m_pCurrentSceneGraphNode = m_pMySeceneGraphRoot->traverseNext(m_pCurrentSceneGraphNode);

        std::cout << "\n--- Scene Graph [current = "
                  << m_pCurrentSceneGraphNode->getName() << "] ---" << std::endl;
        m_pMySeceneGraphRoot->printSceneGraph();
        std::cout << "-------------------" << std::endl;
    }
}