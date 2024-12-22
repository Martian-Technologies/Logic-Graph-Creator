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

class QTRenderer : public Renderer
{
public:
    QTRenderer();
    
    // general flow
    void initialize(const std::string& filePath);
    void updateView(ViewMannager* viewManager, int w, int h) override;
    inline void takePainter(QPainter* painter) { this->painter = painter; }
    void render() override;

    // submission
    void resubmitBlockContainer(BlockContainer* blockContainer) override;
    void submitLine(const std::vector<FPosition>& line, float width) override;
    void submitSprite(BlockType type, const FPosition& position) override;
    void submitBlock(BlockType type, const Position& position) override;
    void submitTint(const Position& position, Color c, float a) override;
    
private:
    QPainter* painter;
    
    QPixmap tileMap;
    int w,h;
};                

#endif // QTRenderer_h
