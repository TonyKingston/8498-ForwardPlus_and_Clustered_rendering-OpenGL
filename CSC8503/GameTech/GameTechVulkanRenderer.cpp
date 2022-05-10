#include "GameTechVulkanRenderer.h"
using namespace NCL;
using namespace Rendering;
using namespace CSC8503;

GameTechVulkanRenderer::GameTechVulkanRenderer(GameWorld& world) : VulkanRenderer(*Window::GetWindow()), gameWorld(world) {
}

GameTechVulkanRenderer::~GameTechVulkanRenderer() {
}
