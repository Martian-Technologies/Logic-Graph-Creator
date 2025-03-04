#ifndef circuitBlockData_h
#define circuitBlockData_h

#include "backend/container/block/blockDefs.h"

class CircuitBlockData {
public:
	inline block_size_t getWidth() const { return width; }
	inline block_size_t getHeight() const { return height; }

private:
	bool isPlaceable = true;

	block_size_t width;
	block_size_t height;
};

#endif /* circuitBlockData_h */