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

class QTRenderer : public Renderer
{
public:
    QTRenderer();
    
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
    void updateLinePosition(LineID line, int index, FPosition position) override;
    void updateLinePositions(LineID line, std::vector<FPosition>& positions) override;
    void updateLineWidth(LineID line, float width) override;
    void removeLine(LineID line) override;

    TintID addTint(Position position, Color color) override;
    TintID addTint(FPosition start, float width, float height, Color color) override;
    void updateTintColor(TintID tint, Color color) override;
    void updateTintRect(Position start, float width, float height) override;
    void removeTint(TintID tint) override;

    void addConfetti(FPosition start) override;
    
private:
    int w,h;

    BlockContainerWrapper* blockContainer; // renderers should usually not retain a pointer to blockContainer
    ViewManager* viewManager; // or viewManager
    
    QPixmap tileSet;
    std::unique_ptr<TileSet<BlockType>> tileSetInfo;
};                

#endif // QTRenderer_h
