#ifndef elementCreator_h
#define elementCreator_h

#include "gpu/mainRenderer.h"

class ElementCreator {
public:
	ElementCreator() : viewportID(0) { }
	ElementCreator(ViewportID viewportID) : viewportID(viewportID) { assert(viewportID); }
	ElementCreator(const ElementCreator& elementCreator) = delete;
	ElementCreator& operator=(const ElementCreator& elementCreator) = delete;

	~ElementCreator() { clear(); }

	void setup(ViewportID viewportID) { clear(); this->viewportID = viewportID; }
	bool isSetup() { return (bool)viewportID; }

	void removeElement(ElementID id) {
		assert(viewportID);
		auto iter = ids.find(id);
		if (iter == ids.end()) return;
		switch (iter->second) {
		case SelectionElement:
			MainRenderer::get().removeSelectionElement(viewportID, iter->first);
			break;
		case BlockPreview:
			MainRenderer::get().removeBlockPreview(viewportID, iter->first);
			break;
		case ConnectionPreview:
			MainRenderer::get().removeConnectionPreview(viewportID, iter->first);
			break;
		case HalfConnectionPreview:
			MainRenderer::get().removeHalfConnectionPreview(viewportID, iter->first);
			break;
		}
		ids.erase(iter);
	}

	inline void clear() {
		if (!viewportID) return;
		logInfo("Clearing: {}", "", ids.size());
		for (auto pair : ids) {
			switch (pair.second) {
			case SelectionElement:
				MainRenderer::get().removeSelectionElement(viewportID, pair.first);
				break;
			case BlockPreview:
				MainRenderer::get().removeBlockPreview(viewportID, pair.first);
				break;
			case ConnectionPreview:
				MainRenderer::get().removeConnectionPreview(viewportID, pair.first);
				break;
			case HalfConnectionPreview:
				MainRenderer::get().removeHalfConnectionPreview(viewportID, pair.first);
				break;
		}
		}
		ids.clear();
	}

	inline bool hasElement(ElementID id) { return ids.find(id) != ids.end(); }

	inline ElementID addSelectionElement(const SelectionObjectElement& selection) {
		assert(viewportID);
		ElementID id = MainRenderer::get().addSelectionObjectElement(viewportID, selection);
		ids[id] = ElementType::SelectionElement;
		return id;
	}

	inline ElementID addSelectionElement(const SelectionElement& selection) {
		assert(viewportID);
		ElementID id = MainRenderer::get().addSelectionElement(viewportID, selection);
		ids[id] = ElementType::SelectionElement;
		return id;
	}

	ElementID addBlockPreview(BlockPreview&& blockPreview) {
		assert(viewportID);
		ElementID id = MainRenderer::get().addBlockPreview(viewportID, std::move(blockPreview));
		ids[id] = ElementType::BlockPreview;
		return id;
	}

	void shiftBlockPreview(ElementID id, Vector shift) {
		assert(viewportID);
		if (ids.contains(id)) MainRenderer::get().shiftBlockPreview(viewportID, id, shift);
	}

	ElementID addConnectionPreview(const ConnectionPreview& connectionPreview) {
		assert(viewportID);
		ElementID id = MainRenderer::get().addConnectionPreview(viewportID, connectionPreview);
		ids[id] = ElementType::ConnectionPreview;
		return id;
	}

	ElementID addHalfConnectionPreview(const HalfConnectionPreview& halfConnectionPreview) {
		assert(viewportID);
		ElementID id = MainRenderer::get().addHalfConnectionPreview(viewportID, halfConnectionPreview);
		ids[id] = ElementType::HalfConnectionPreview;
		return id;
	}

private:
	enum ElementType {
		SelectionElement,
		ConnectionPreview,
		HalfConnectionPreview,
		BlockPreview,
	};

	ViewportID viewportID;
	std::map<ElementID, ElementType> ids;
};

#endif /* elementCreator_h */
