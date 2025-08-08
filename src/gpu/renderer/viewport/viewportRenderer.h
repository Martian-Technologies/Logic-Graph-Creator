#ifndef viewportRenderer_h
#define viewportRenderer_h

#include "elements/elementRenderer.h"
#include "gpu/renderer/frameManager.h"
#include "grid/gridRenderer.h"
#include "logic/chunking/chunkRenderer.h"
#include "viewportRenderInterface.h"

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
