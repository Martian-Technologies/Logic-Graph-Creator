#ifndef blockContainerView_h
#define blockContainerView_h

#include "middleEnd/blockContainerWrapper.h"
#include "../events/eventRegister.h"
#include "tools/toolManager.h"

class BlockContainerView {
public:
    BlockContainerView() : eventRegister(), toolManager(&eventRegister) {

    }

    inline void setBlockContainer(BlockContainerWrapper* blockContainerWrapper) {
        this->blockContainerWrapper = blockContainerWrapper;
        toolManager.setBlockContainer(blockContainerWrapper);
    }

    ToolManager& getToolManager() { return toolManager; }
    EventRegister& getEventRegister() { return eventRegister; }

private:
    BlockContainerWrapper* blockContainerWrapper;

    EventRegister eventRegister;
    ToolManager toolManager;
};

#endif /* blockContainerView_h */
