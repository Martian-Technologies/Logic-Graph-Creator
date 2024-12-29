#ifndef QTRenderer_h
#define QTRenderer_h

#include <QPainter>
#include <memory>
#include <unordered_map>

#include "../viewManager/viewManager.h"
#include "backend/defs.h"
#include "renderer.h"
#include "tileSet.h"
    
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
    ElementID addSelectionElement(const SelectionElement& selection) override;
    void removeSelectionElement(ElementID selection) override;
    
    ElementID addBlockPreview(const BlockPreview& blockPreview) override;
    void removeBlockPreview(ElementID blockPreview) override;
    
    ElementID addConnectionPreview(const ConnectionPreview& connectionPreview) override;
    void removeConnectionPreview(ElementID connectionPreview) override;
    
    void spawnConfetti(FPosition start) override;

private:
    QPointF gridToQt(FPosition position);
    
    void renderBlock(QPainter* painter, BlockType type, Position position, Rotation rotation);
    void setUpConnectionPainter(QPainter* painter);
    void renderConnection(QPainter* painter, const Block* a, Position aPos, const Block* b, Position bPos, bool setUpPainter = true);
    
    int w, h;
    BlockContainerWrapper* blockContainer;
    ViewManager* viewManager;
    QPixmap tileSet;
    std::unique_ptr<TileSet<BlockType>> tileSetInfo;

    // Elements
    ElementID currentID = 0;
    std::unordered_map<ElementID, SelectionElement> selectionElements;
    std::unordered_map<ElementID, SelectionElement> invertedSelectionElements;
    std::unordered_map<ElementID, BlockPreview> blockPreviews;
    std::unordered_map<ElementID, ConnectionPreview> connectionPreviews;
};

#endif /* QTRenderer_h */
