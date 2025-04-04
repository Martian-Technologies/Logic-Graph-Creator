#include "windowRenderer.h"

WindowRenderer::WindowRenderer(SdlWindow* sdlWindow)
	: sdlWindow(sdlWindow) {
	logInfo("Initializing window renderer...");

	surface = sdlWindow->createVkSurface(VulkanInstance::get().getInstance());
	VulkanInstance::get().ensureDeviceCreation(surface);
	device = VulkanInstance::get().getDevice();

	// set up swapchain
	swapchain = new Swapchain(surface);
	createRenderPass();
	swapchain->createFramebuffers(renderPass);

	// set up frames
	frames.resize(2);
}

WindowRenderer::~WindowRenderer() {
	vkDestroyRenderPass(device, renderPass, nullptr);
	delete swapchain;
}

void WindowRenderer::createRenderPass() {
	// render pass
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = swapchain->getVkbSwapchain().image_format;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	// subpass
	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	// subpass dependency
	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	// create pass
	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;
	
	if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
		throw std::runtime_error("failed to create render pass!");
	}
}

// -- Rml::RenderInterface --

Rml::CompiledGeometryHandle WindowRenderer::CompileGeometry(Rml::Span<const Rml::Vertex> vertices, Rml::Span<const int> indices) {
	return 1;
}
void WindowRenderer::ReleaseGeometry(Rml::CompiledGeometryHandle geometry) {
	
}
void WindowRenderer::RenderGeometry(Rml::CompiledGeometryHandle handle, Rml::Vector2f translation, Rml::TextureHandle texture) {
	
}

// Textures
Rml::TextureHandle WindowRenderer::LoadTexture(Rml::Vector2i& texture_dimensions, const Rml::String& source) {
	return 1;
}
Rml::TextureHandle WindowRenderer::GenerateTexture(Rml::Span<const Rml::byte> source, Rml::Vector2i source_dimensions) {
	return 1;
}
void WindowRenderer::ReleaseTexture(Rml::TextureHandle texture_handle) {
	
}

// Scissor
void WindowRenderer::EnableScissorRegion(bool enable) {
	
}
void WindowRenderer::SetScissorRegion(Rml::Rectanglei region) {
	
}
