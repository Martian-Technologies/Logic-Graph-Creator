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
    LineID addLine(const std::vector<FPosition>& positions, float width) override;
    void updateLinePosition(LineID id, int index, FPosition position) override;
    void updateLinePositions(LineID id, std::vector<FPosition>& positions) override;
    void updateLineWidth(LineID id, float width) override;
    void removeLine(LineID id) override;

    TintID addTint(Position position, Color color) override;
    TintID addTint(FPosition start, float width, float height, Color color) override;
    void updateTintColor(TintID id, Color color) override;
    void updateTintRect(TintID id, FPosition start, float width, float height) override;
    void removeTint(TintID id) override;
    
    BlockPreviewID addBlockPreview(Position position, Rotation rotation, Color modulate, float alpha) override;
    void updateBlockPreviewColor(BlockPreviewID id, Color modulate, float alpha) override;
    void removeBlockPreview(BlockPreviewID id) override;

    ConnectionPreviewID addConnectionPreview(std::pair<FPosition, FPosition> positions, Color modulate, float alpha) override;
    void updateConnectionPreviewPositions(ConnectionPreviewID id, std::pair<FPosition, FPosition> positions) override;
    void updateConnectionPreviewColor(ConnectionPreviewID id, Color modulate, float alpha) override;
    void removeConnectionPreview(ConnectionPreviewID id) override;

    void addConfetti(FPosition start) override;

private:
    int w, h;
    BlockContainerWrapper* blockContainer; // renderers should usually not retain a pointer to blockContainer
    ViewManager* viewManager; // or a viewManager
    QPixmap tileSet;
    std::unique_ptr<TileSet<BlockType>> tileSetInfo;

    QPointF gridToQt(FPosition position);
};

#endif // QTRenderer_h
