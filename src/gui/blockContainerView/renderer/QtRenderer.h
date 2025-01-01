#ifndef QTRenderer_h
#define QTRenderer_h

#include <QLineF>
#include <memory>
#include <unordered_map>
#include <QPainter>
#include <QColor>

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
    void setEvaluator(Evaluator* evaluator) override;

    void updateView(ViewManager* viewManager) override;
    virtual void updateBlockContainer(DifferenceSharedPtr diff) override;

    inline float getLastFrameTimeMs() const override { return lastFrameTime; }

private:
    // elements
    ElementID addSelectionElement(const SelectionObjectElement& selection) override;
    ElementID addSelectionElement(const SelectionElement& selection) override;
    void removeSelectionElement(ElementID selection) override;

    ElementID addBlockPreview(const BlockPreview& blockPreview) override;
    void removeBlockPreview(ElementID blockPreview) override;

    ElementID addConnectionPreview(const ConnectionPreview& connectionPreview) override;
    void removeConnectionPreview(ElementID connectionPreview) override;

    ElementID addHalfConnectionPreview(const HalfConnectionPreview& halfConnectionPreview) override;
    void removeHalfConnectionPreview(ElementID halfConnectionPreview) override;

    void spawnConfetti(FPosition start) override;

private:
    QPointF gridToQt(FPosition position);

    void renderSelection(QPainter* painter, const SharedSelection selection, SelectionObjectElement::RenderMode mode, unsigned int depth = 0);
    void renderBlock(QPainter* painter, BlockType type, Position position, Rotation rotation, bool state = false);
    void renderConnection(QPainter* painter, FPosition aPos, FPosition bPos, FPosition aControlOffset, FPosition bControlOffset, std::vector<QLineF>& lines);
    void renderConnection(QPainter* painter, Position aPos, const Block* a, Position bPos, const Block* b, std::vector<QLineF>& lines);
    void renderConnection(QPainter* painter, Position aPos, Position bPos, std::vector<QLineF>& lines);
    void renderConnection(QPainter* painter, Position aPos, FPosition bPos, std::vector<QLineF>& lines);

    void drawArrow(QPainter* painter, const QPointF& start, const QPointF& end, float scale, const QColor& color);

    int w, h;
    BlockContainerWrapper* blockContainer;
    Evaluator* evaluator;
    ViewManager* viewManager;
    QPixmap tileSet;
    std::unique_ptr<TileSetInfo> tileSetInfo;

    // Elements
    ElementID currentID = 0;
    std::unordered_map<ElementID, SelectionElement> selectionElements;
    std::unordered_map<ElementID, SelectionElement> invertedSelectionElements;
    std::unordered_map<ElementID, SelectionObjectElement> selectionObjectElements;
    std::unordered_map<ElementID, BlockPreview> blockPreviews;
    std::unordered_map<ElementID, ConnectionPreview> connectionPreviews;
    std::unordered_map<ElementID, HalfConnectionPreview> halfConnectionPreviews;

    unsigned int lineRendingLimit = 200000;

    // info
    float lastFrameTime = 0.0f;
};

#endif /* QTRenderer_h */
