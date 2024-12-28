#ifndef elementCreator_h
#define elementCreator_h

#include <set>

#include "renderer.h"


class ElementCreator {
public:
    ElementCreator(Renderer* renderer) : renderer(renderer), ids() { assert(renderer); }

    ~ElementCreator() { for (ElementID id : ids) renderer->removeElement(id); }

    void removeElement(ElementID id) {
        auto iter = ids.find(id);
        if (iter == ids.end()) return;
        ids.erase(iter);
        renderer->removeElement(id);
    }

    inline void clear() { for (ElementID id : ids) renderer->removeElement(id); ids.clear(); }

    inline bool hasElement(ElementID id) { return ids.find(id) != ids.end(); }

    inline ElementID addSelectionElement(Position position) {
        ElementID id = renderer->addSelectionElement(position, position);
        ids.insert(id);
        return id;
    }

    inline ElementID addSelectionElement(Position positionA, Position positionB) {
        ElementID id = renderer->addSelectionElement(positionA, positionB);
        ids.insert(id);
        return id;
    }

    ElementID addBlockPreview(Position position, Rotation rotation, Color modulate, float alpha) {
        ElementID id = renderer->addBlockPreview(position, rotation, modulate, alpha);
        ids.insert(id);
        return id;
    }

    ElementID addConnectionPreview(Position inputCellPos, Position outputCellPos, Color modulate, float alpha) {
        ElementID id = renderer->addConnectionPreview(inputCellPos, outputCellPos, modulate, alpha);
        ids.insert(id);
        return id;
    }

    void addConfetti(FPosition start) {
        renderer->addConfetti(start);
    }

private:
    Renderer* renderer;
    std::set<ElementID> ids;
};

#endif /* elementCreator_h */
