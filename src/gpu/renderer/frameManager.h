#ifndef frameManager_h
#define frameManager_h

#include <volk.h>
#include <chrono>

#include "util/lifetimeExtender.h"

constexpr unsigned int FRAMES_IN_FLIGHT = 1;

struct Frame {
public:
	Frame();
	~Frame();

public:
	VkCommandPool commandPool;
	VkCommandBuffer mainCommandBuffer;
	VkSemaphore swapchainSemaphore, renderSemaphore;
	VkFence renderFence;
	std::chrono::time_point<std::chrono::system_clock> lastStartTime;

	LifetimeExtender lifetime;
};

class FrameManager {
public:
	void incrementFrame();
	float waitForCurrentFrameCompletion();
	void startCurrentFrame();

	inline Frame& getCurrentFrame() { return frames[frameIndex]; }
	
private:
	std::array<Frame, FRAMES_IN_FLIGHT> frames;
	uint32_t frameNumber = 0;
	uint32_t frameIndex = 0;
};

#endif
