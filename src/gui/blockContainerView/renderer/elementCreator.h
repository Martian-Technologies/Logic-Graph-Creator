#ifndef elementCreator_h
#define elementCreator_h

#include <set>

#include "renderer.h"


class ElementCreator {
public:
    ElementCreator() : renderer(nullptr) {}
    ElementCreator(Renderer* renderer) : renderer(renderer), ids() { assert(renderer); }

    ~ElementCreator() { for (ElementID id : ids) renderer->removeElement(id); }

    void removeElement(ElementID id) {
        assert(renderer);
        auto iter = ids.find(id);
        if (iter == ids.end()) return;
        ids.erase(iter);
        renderer->removeElement(id);
    }

    inline void clear() { assert(renderer); for (ElementID id : ids) renderer->removeElement(id); ids.clear(); }

    inline bool hasElement(ElementID id) { return ids.find(id) != ids.end(); }

    inline ElementID addSelectionElement(Position position, bool inverted = false) {
        assert(renderer);
        ElementID id = renderer->addSelectionElement(position, position, inverted);
        ids.insert(id);
        return id;
    }

    inline ElementID addSelectionElement(Position positionA, Position positionB, bool inverted = false) {
        assert(renderer);
        ElementID id = renderer->addSelectionElement(positionA, positionB, inverted);
        ids.insert(id);
        return id;
    }

    ElementID addBlockPreview(Position position, Rotation rotation, Color modulate = Color(), float alpha = 1.f) {
        assert(renderer);
        ElementID id = renderer->addBlockPreview(position, rotation, modulate, alpha);
        ids.insert(id);
        return id;
    }

    ElementID addConnectionPreview(Position inputCellPos, Position outputCellPos, Color modulate = Color(), float alpha = 1.f) {
        assert(renderer);
        ElementID id = renderer->addConnectionPreview(inputCellPos, outputCellPos, modulate, alpha);
        ids.insert(id);
        return id;
    }

    void addConfetti(FPosition start) {
        assert(renderer);
        renderer->spawnConfetti(start);
    }

private:
    Renderer* renderer;
    std::set<ElementID> ids;
};

#endif /* elementCreator_h */
