#ifndef QTRenderer_h
#define QTRenderer_h

#include <QPainter>
#include <memory>
#include <qpainter.h>
#include <qpixmap.h>

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
    ElementID addSelectionElement(Position topLeft, Position bottomRight, bool inverted) override;
    void removeSelectionElement(ElementID selection) override;
    
    ElementID addBlockPreview(Position position, Rotation rotation, Color modulate, float alpha) override;
    void removeBlockPreview(ElementID blockPreview) override;
    
    ElementID addConnectionPreview(Position input, Position output, Color modulate, float alpha) override;
    void removeConnectionPreview(ElementID connectionPreview) override;
    
    void spawnConfetti(FPosition start) override;

private:
    int w, h;
    BlockContainerWrapper* blockContainer; // renderers should usually not retain a pointer to blockContainer
    ViewManager* viewManager; // or a viewManager
    QPixmap tileSet;
    std::unique_ptr<TileSet<BlockType>> tileSetInfo;

    QPointF gridToQt(FPosition position);
};

#endif /* QTRenderer_h */
