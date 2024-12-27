#ifndef blockContainerView_h
#define blockContainerView_h

#include <memory>

#include "middleEnd/blockContainerWrapper.h"
#include "events/eventRegister.h"
#include "tools/toolManager.h"

class BlockContainerView {
public:
    BlockContainerView() : blockContainerWrapper(), eventRegister(), toolManager(&eventRegister) {}

    inline void setBlockContainer(std::shared_ptr<BlockContainerWrapper> blockContainerWrapper) {
        this->blockContainerWrapper = blockContainerWrapper;
        toolManager.setBlockContainer(blockContainerWrapper.get());
    }

    inline BlockContainerWrapper* getBlockContainer() { return blockContainerWrapper.get(); }
    inline const BlockContainerWrapper* getBlockContainer() const { return blockContainerWrapper.get(); }

    ToolManager& getToolManager() { return toolManager; }
    EventRegister& getEventRegister() { return eventRegister; }

private:
    std::shared_ptr<BlockContainerWrapper>  blockContainerWrapper;
    EventRegister eventRegister;
    ToolManager toolManager;
};

#endif /* blockContainerView_h */
