#ifndef elementCreator_h
#define elementCreator_h

#include <set>

#include "renderer.h"


class ElementCreator {
    friend class Renderer;
public:
    void removeElement(ElementID id) {
        auto iter = ids.find(id);
        if (iter == ids.end()) return;
        ids.erase(iter);
        renderer->removeElement(id);
    }

    inline ElementID addTint(Position position, Color color) {
        ElementID id = renderer->addTint(position, color);
        ids.insert(id);
        return id;
    }
    
    inline ElementID addTint(FPosition start, float width, float height, Color color) {
        ElementID id = renderer->addTint(start, width, height, color);
        ids.insert(id);
        return id;
    }

    ElementID addBlockPreview(Position position, Rotation rotation, Color modulate, float alpha) {
        ElementID id = renderer->addTint(position, rotation, modulate, alpha);
        ids.insert(id);
        return id;
    }

    ElementID addConnectionPreview(Position inputCellPos, Position outputCellPos, Color modulate, float alpha) {
        ElementID id = renderer->addTint(inputCellPos, outputCellPos, modulate, alpha);
        ids.insert(id);
        return id;
    }

    void addConfetti(FPosition start) {
        renderer->addConfetti(start);
    }

private:
    ElementCreator(Renderer* renderer) : renderer(renderer) { assert(renderer); }

    std::set<ElementID> ids;
    Renderer* renderer;
};

#endif /* elementCreator_h */
