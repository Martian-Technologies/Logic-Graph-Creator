#ifndef vulkanWindow_h
#define vulkanWindow_h

#include <QWindow>

#include "gpu/vulkanRenderer.h"

class VulkanWindow : public QWindow {
	Q_OBJECT
public:
	VulkanWindow(VulkanRenderer* renderer);
	virtual ~VulkanWindow();

private:
	VulkanRenderer* renderer;
};

#endif
