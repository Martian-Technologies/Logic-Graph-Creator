#ifndef rendererTMP_h
#define rendererTMP_h

#include "renderer.h"

class RendererTMP : public Renderer {
public:
	// main flow
	void setCircuit(Circuit* circuit) override {};
	void setEvaluator(Evaluator* evaluator) override {};
	void setAddress(const Address& address) override {};

	void updateView(ViewManager* viewManager) override {};
	void updateCircuit(DifferenceSharedPtr diff) override {};

	float getLastFrameTimeMs() const override {};

private:
	// elements
	ElementID addSelectionElement(const SelectionObjectElement& selection) override {};
	ElementID addSelectionElement(const SelectionElement& selection) override {};
	void removeSelectionElement(ElementID selection) override {};

	ElementID addBlockPreview(const BlockPreview& blockPreview) override {};
	void removeBlockPreview(ElementID blockPreview) override {};

	ElementID addConnectionPreview(const ConnectionPreview& connectionPreview) override {};
	void removeConnectionPreview(ElementID connectionPreview) override {};

	ElementID addHalfConnectionPreview(const HalfConnectionPreview& halfConnectionPreview) override {};
	void removeHalfConnectionPreview(ElementID halfConnectionPreview) override {};

	void spawnConfetti(FPosition start) override {};
};

#endif /* rendererTMP_h */
