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

// TODO
// - [x] QT renderer
// - [x] Coodinate system conversion (viewmanager handles conversions, gives renderer what it needs)
// - [x] viewmanager grid cell selection and input loop
// - [ ] blockContainer data to renderer (placeholder event and internal data structure)
// - [ ] write rendering logic
// - [ ] connect effects to renderer

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

    // effects
    ElementID addLine(const std::vector<FPosition>& positions, float width) override;
    void removeLine(ElementID id) override;

    ElementID addTint(Position position, Color color) override;
    ElementID addTint(FPosition start, float width, float height, Color color) override;
    void removeTint(ElementID id) override;
    
    ElementID addBlockPreview(Position position, Rotation rotation, Color modulate, float alpha) override;
    void removeBlockPreview(ElementID id) override;

    ElementID addConnectionPreview(std::pair<FPosition, FPosition> positions, Color modulate, float alpha) override;
    void removeConnectionPreview(ElementID id) override;

    void addConfetti(FPosition start) override;

private:
    int w, h;
    BlockContainerWrapper* blockContainer; // renderers should usually not retain a pointer to blockContainer
    ViewManager* viewManager; // or a viewManager
    QPixmap tileSet;
    std::unique_ptr<TileSet<BlockType>> tileSetInfo;

    QPointF gridToQt(FPosition position);
};

#endif /* QTRenderer_h */
