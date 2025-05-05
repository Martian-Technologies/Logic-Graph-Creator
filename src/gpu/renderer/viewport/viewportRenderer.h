#ifndef viewportRenderer_h
#define viewportRenderer_h

#include "gpu/renderer/viewport/gridRenderer.h"
#include "gpu/renderer/viewport/chunking/chunkRenderer.h"
#include "gpu/renderer/viewportRenderInterface.h"
#include "gpu/renderer/frameManager.h"

class ViewportRenderer {
public:
	ViewportRenderer(VkRenderPass renderPass);

	void render(Frame& frame, ViewportRenderInterface* viewport);

private:
	GridRenderer gridRenderer;
	ChunkRenderer chunkRenderer;
};

#endif
