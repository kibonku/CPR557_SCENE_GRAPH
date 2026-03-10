#include "my_simple_render_factory.h"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <cassert>
#include <stdexcept>

struct MySimplePushConstantData
{
    glm::mat4 transform{ 1.f };
    alignas(16) glm::vec3 push_color;
};

MySimpleRenderFactory::MySimpleRenderFactory(MyDevice& device, VkRenderPass renderPass)
    : m_myDevice{ device } 
{
    _createPipelineLayout();
    _createPipeline(renderPass);
}

MySimpleRenderFactory::~MySimpleRenderFactory()
{
    vkDestroyPipelineLayout(m_myDevice.device(), m_vkPipelineLayout, nullptr);
}

void MySimpleRenderFactory::_createPipelineLayout() 
{
    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(MySimplePushConstantData);

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    if (vkCreatePipelineLayout(m_myDevice.device(), &pipelineLayoutInfo, nullptr, &m_vkPipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create pipeline layout!");
    }
}

void MySimpleRenderFactory::_createPipeline(VkRenderPass renderPass)
{
    assert(m_vkPipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

    PipelineConfigInfo pipelineConfig{};
    MyPipeline::defaultPipelineConfigInfo(pipelineConfig);
    pipelineConfig.renderPass = renderPass;
    pipelineConfig.pipelineLayout = m_vkPipelineLayout;

    m_pMyPipeline = std::make_unique<MyPipeline>(
        m_myDevice,
        "shaders/simple_shader.vert.spv",
        "shaders/simple_shader.frag.spv",
        pipelineConfig);
}

// -----------------------------------------------------------------------
// renderSceneGraph: public entry point for rendering the whole scene graph
// -----------------------------------------------------------------------
void MySimpleRenderFactory::renderSceneGraph(
    VkCommandBuffer commandBuffer,
    std::shared_ptr<MySceneGraphNode>& sceneGraph,
    const MyCamera& camera)
{
    if (!sceneGraph) return;

    m_pMyPipeline->bind(commandBuffer);

    // Projection * View matrix from camera
    glm::mat4 projView = camera.projectionMatrix() * camera.viewMatrix();

    // Start with root's own transform as the initial accumulated transform
    glm::mat4 rootTransform = sceneGraph->transform.mat4();
    bool rootSelected = sceneGraph->isCurrent();

    // Recurse through all children of root
    for (auto& child : sceneGraph->getChildren())
    {
        _renderSceneGraph(commandBuffer, child, projView, rootTransform, rootSelected);
    }
}

// -----------------------------------------------------------------------
// _renderSceneGraph: recursive traversal
//   - accumulates parent * node transforms
//   - renders leaf nodes (those with a model) using the accumulated transform
//   - propagates selection highlight down to children when a group is selected
// -----------------------------------------------------------------------
void MySimpleRenderFactory::_renderSceneGraph(
    VkCommandBuffer commandBuffer,
    const std::shared_ptr<MySceneGraphNode>& node,
    const glm::mat4& projView,
    glm::mat4 parentTransform,
    bool parentSelected)
{
    if (!node) return;

    // Accumulate this node's local transform on top of the parent's
    glm::mat4 accumulatedTransform = parentTransform * node->transform.mat4();

    // Phase 5 highlight: a node is highlighted if it is selected, OR if its
    // parent group is selected (so selecting a group lights up all its children)
    bool thisSelected = parentSelected || node->isCurrent();

    if (node->model)
    {
        // --- Leaf node: has geometry to render ---
        MySimplePushConstantData push{};

        // Final MVP transform
        push.transform = projView * accumulatedTransform;

        // Phase 5: pass a white tint into push_color when this subtree is selected
        // The vertex shader blends vertex color with push_color for a glow effect
        if (thisSelected)
            push.push_color = glm::vec3(0.9f, 0.9f, 0.9f); // bright highlight tint
        else
            push.push_color = glm::vec3(0.0f, 0.0f, 0.0f); // no tint (black = inactive)

        vkCmdPushConstants(
            commandBuffer,
            m_vkPipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(MySimplePushConstantData),
            &push);

        node->model->bind(commandBuffer);
        node->model->draw(commandBuffer);
    }
    else
    {
        // --- Interior / Group node: no geometry, just pass transform to children ---
        for (auto& child : node->getChildren())
        {
            _renderSceneGraph(commandBuffer, child, projView, accumulatedTransform, thisSelected);
        }
    }
}