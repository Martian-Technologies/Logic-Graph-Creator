#ifndef elementCreator_h
#define elementCreator_h

#include <set>

#include "renderer.h"
class Renderer;


class ElementCreator {
    friend class Renderer;
public:
    inline ElementID addLine(const std::vector<FPosition>& positions, float width) {
        ElementID id = renderer->addLine(positions, width);
        ids.insert(id);
        return id;
    }
    inline void updateLinePosition(ElementID line, int index, FPosition position);
    inline void updateLinePositions(ElementID line, std::vector<FPosition>& positions);
    inline void updateLineWidth(ElementID line, float width);
    inline void removeLine(ElementID line);

    inline ElementID addTint(Position position, Color color) {
        ElementID id = renderer->addTint(positions, width);
        ids.insert(id);
        return id;
    }
    inline ElementID addTint(FPosition start, float width, float height, Color color);
    inline void updateTintColor(ElementID tint, Color color);
    inline void updateTintRect(Position start, float width, float height);
    inline void removeTint(ElementID tint);

private:
    ElementCreator(Renderer* renderer) : renderer(renderer) { assert(renderer); }

    std::set<ElementID> ids;
    Renderer* renderer;
};

#endif /* elementCreator_h */
