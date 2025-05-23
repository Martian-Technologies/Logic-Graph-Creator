#ifndef viewportRenderer_h
#define viewportRenderer_h

#include "gpu/renderer/viewport/gridRenderer.h"
#include "gpu/renderer/viewport/chunking/chunkRenderer.h"
#include "gpu/renderer/viewport/elements/elementRenderer.h"
#include "gpu/renderer/viewportRenderInterface.h"
#include "gpu/renderer/frameManager.h"

class ViewportRenderer {
public:
	void init(VulkanDevice* device, VkRenderPass renderPass);
	void cleanup();

	void render(Frame& frame, ViewportRenderInterface* viewport);

private:
	GridRenderer gridRenderer;
	ChunkRenderer chunkRenderer;
	ElementRenderer elementRenderer;
};

#endif
