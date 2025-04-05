#include "rmlRenderer.h"

#include "gpu/vulkanInstance.h"

RmlGeometryAllocation::RmlGeometryAllocation(Rml::Span<const Rml::Vertex> vertices, Rml::Span<const int> indices) {
	size_t vertexBufferSize = vertices.size() * sizeof(RmlVertex);
	size_t indexBufferSize = indices.size() * sizeof(int);
	numIndices = indices.size();
;
	vertexBuffer = createBuffer(vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO);
	indexBuffer = createBuffer(indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO);

	vmaCopyMemoryToAllocation(VulkanInstance::get().getAllocator(), vertices.data(), vertexBuffer.allocation, 0, vertexBufferSize);
	vmaCopyMemoryToAllocation(VulkanInstance::get().getAllocator(), indices.data(), indexBuffer.allocation, 0, indexBufferSize);
}

RmlGeometryAllocation::~RmlGeometryAllocation() {
	destroyBuffer(indexBuffer);
	destroyBuffer(vertexBuffer);
}
