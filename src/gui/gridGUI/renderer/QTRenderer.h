#ifndef QTRenderer_h
#define QTRenderer_h

#include <QPainter>
#include <qpainter.h>
#include <qpixmap.h>

#include "renderer.h"

// TODO
// - [x] QT renderer
// - [ ] blockContainer data to renderer
// - [ ] Coodinate system conversion
// - [ ] Integration with rest of app

class QTRenderer : public Renderer
{
 public:
    QTRenderer();
    
	// general flow
	void initialize(const std::string& filePath);
	void render() override;
	inline void takePainter(QPainter* painter) { this->painter = painter; }
	inline void resize(int w, int h) override { this->w = w; this->h = h; }

	// submission
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
