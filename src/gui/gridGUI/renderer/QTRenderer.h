#ifndef QTRenderer_h
#define QTRenderer_h

#include <QPainter>
#include <qpainter.h>

#include "renderer.h"

// TODO
// - [x] QT renderer
// - [ ] blockContainer data to renderer
// - [ ] Coodinate system conversion
// - [ ] Integration with rest of app

class QTRenderer : Renderer
{
 public:
	// general flow
	void initialize(const std::string& tileMap);
	inline void takePainter(QPainter* painter) { this->painter = painter; }
	void render() override;
	void resize(int w, int h) override;

	// submission
	void submitLine(const std::vector<FPosition>& line, float width) override;
	void submitSprite(BlockType type, const FPosition& position) override;
	void submitBlock(BlockType type, const Position& position) override;
    void submitTint(const Position& position, Color c, float a) override;
	
 private:
	QPainter* painter;
};                

#endif // QTRenderer_h
