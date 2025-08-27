#include "mainRenderer.h"

MainRenderer& MainRenderer::get() {
	static MainRenderer singleton;
	return singleton;
}
