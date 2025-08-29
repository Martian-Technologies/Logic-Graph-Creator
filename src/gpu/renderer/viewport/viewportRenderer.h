#ifndef viewportRenderer_h
#define viewportRenderer_h

#include "logic/chunking/chunkRenderer.h"
#include "gpu/renderer/frameManager.h"
#include "elements/elementRenderer.h"
#include "viewportRenderInterface.h"
#include "grid/gridRenderer.h"

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
