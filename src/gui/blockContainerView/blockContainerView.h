#ifndef blockContainerView_h
#define blockContainerView_h

#include <type_traits>
#include <memory>

#include "middleEnd/blockContainerWrapper.h"
#include "backend/evaluator/evaluatorStateInterface.h"
#include "backend/evaluator/evaluator.h"
#include "events/eventRegister.h"
#include "tools/toolManager.h"
#include "renderer/renderer.h"
#include "viewManager/viewManager.h"

template <class RENDERER_TYPE>
// typename std::enable_if<std::is_base_of<Renderer, RENDERER_TYPE>::value, void>::type // idk if we can get this working
class BlockContainerView {
public:
    BlockContainerView() : blockContainerWrapper(), evaluator(), evaluatorStateInterface(), eventRegister(), viewManager(), renderer(), toolManager(&eventRegister, &renderer) {
        viewManager.initialize(eventRegister);
        viewManager.connectViewChanged(std::bind(&BlockContainerView::viewChanged, this));
    }

    inline void setEvaluator(std::shared_ptr<Evaluator> evaluator) {
        renderer.setEvaluator(evaluator.get());
        evaluatorStateInterface = EvaluatorStateInterface(evaluator.get());
        toolManager.setEvaluatorStateInterface(&evaluatorStateInterface);
    }

    inline void setBlockContainer(std::shared_ptr<BlockContainerWrapper> blockContainerWrapper) {
        this->blockContainerWrapper = blockContainerWrapper;
        toolManager.setBlockContainer(blockContainerWrapper.get());
        renderer.setBlockContainer(blockContainerWrapper.get());
    }

    void viewChanged() {
        renderer.updateView(&viewManager);
        eventRegister.doEvent(PositionEvent("pointer move", viewManager.getPointerPosition()));
    }

    // --------------- Gettters ---------------

    inline BlockContainerWrapper* getBlockContainer() { return blockContainerWrapper.get(); }
    inline const BlockContainerWrapper* getBlockContainer() const { return blockContainerWrapper.get(); }

    inline EventRegister& getEventRegister() { return eventRegister; }
    inline const EventRegister& getEventRegister() const { return eventRegister; }

    inline ToolManager& getToolManager() { return toolManager; }
    inline const ToolManager& getToolManager() const { return toolManager; }

    inline ViewManager& getViewManager() { return viewManager; }
    inline const ViewManager& getViewManager() const { return viewManager; }

    inline RENDERER_TYPE& getRenderer() { return renderer; }
    inline const RENDERER_TYPE& getRenderer() const { return renderer; }

private:
    std::shared_ptr<BlockContainerWrapper> blockContainerWrapper;
    std::shared_ptr<Evaluator> evaluator;
    EvaluatorStateInterface evaluatorStateInterface;
    EventRegister eventRegister;
    ViewManager viewManager;
    RENDERER_TYPE renderer;
    ToolManager toolManager;
};

#endif /* blockContainerView_h */
