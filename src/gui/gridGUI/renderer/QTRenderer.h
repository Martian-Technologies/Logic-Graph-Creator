#ifndef QTRenderer_h
#define QTRenderer_h

#include <QPainter>
#include <qpainter.h>
#include <qpixmap.h>

#include "renderer.h"

// TODO
// - [x] QT renderer
// - [x] Coodinate system conversion (viewmanager handles conversions, gives renderer what it needs)
// - [x] viewmanager grid cell selection and input loop
// - [ ] blockContainer data to renderer (placeholder event and internal data structure)
// - [ ] write rendering logic
// - [ ] connect effects to renderer

// This QT renderer is not designed well. The renderer interface is designed to be implemented 

class QTRenderer : public Renderer
{
public:
    QTRenderer();
    
    // general flow
    void initialize(const std::string& filePath);
    void resize(int w, int h);
    void render(QPainter* painter);

    // updating
    void setBlockContainer(BlockContainer* blockContainer) override;
    // virtual void setSimulator(Simulator* simulator) override;
    
    void updateView(ViewMannager* viewManager) override;
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

    BlockContainer* blockContainer; // renderers should usually not retain a pointer to block
    QPixmap tileMap;
};                

#endif // QTRenderer_h
