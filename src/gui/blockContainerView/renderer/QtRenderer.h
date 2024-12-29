#ifndef QTRenderer_h
#define QTRenderer_h

#include <QPainter>
#include <memory>
#include <unordered_map>

#include "../viewManager/viewManager.h"
#include "backend/defs.h"
#include "renderer.h"
#include "tileSet.h"

struct SelectionElement
{
    Position topLeft;
    Position bottomRight;
    bool inverted;
};

struct ConnectionPreview
{
    Position a;
    Position b;
    Color modulate;
    float alpha;
};
    
class QtRenderer : public Renderer {
public:
    QtRenderer();

    // general flow
    void initializeTileSet(const std::string& filePath);
    void resize(int w, int h);
    void render(QPainter* painter);

    // updating
    void setBlockContainer(BlockContainerWrapper* blockContainer) override;
    // virtual void setSimulator(Simulator* simulator) override;

    void updateView(ViewManager* viewManager) override;
    // virtual void updateBlockContainer(Difference diff) override;

private:
    // elements
    ElementID addSelectionElement(Position topLeft, Position bottomRight, bool inverted) override;
    void removeSelectionElement(ElementID selection) override;
    
    ElementID addBlockPreview(Position position, Rotation rotation, Color modulate, float alpha) override;
    void removeBlockPreview(ElementID blockPreview) override;
    
    ElementID addConnectionPreview(Position input, Position output, Color modulate, float alpha) override;
    void removeConnectionPreview(ElementID connectionPreview) override;
    
    void spawnConfetti(FPosition start) override;

private:
    QPointF gridToQt(FPosition position);
    
    void setUpConnectionPainter(QPainter* painter);
    void renderConnection(QPainter* painter, const Block* a, Position aPos, const Block* b, Position bPos, bool setUpPainter = true);
    
    int w, h;
    ElementID currentID = 0;
    BlockContainerWrapper* blockContainer; // renderers should usually not retain a pointer to blockContainer
    ViewManager* viewManager; // or a viewManager
    QPixmap tileSet;
    std::unique_ptr<TileSet<BlockType>> tileSetInfo;

    std::unordered_map<ElementID, SelectionElement> selectionElements;
    std::unordered_map<ElementID, SelectionElement> invertedSelectionElements;
    std::unordered_map<ElementID, ConnectionPreview> connectionPreviews;
};

#endif /* QTRenderer_h */
