#ifndef blockContainerManager_h
#define blockContainerManager_h

#include <memory>
#include <map>

#include "blockContainerWrapper.h"

class BlockContainerManager {
public:
    BlockContainerManager() : lastId(0), blockContainers() {}

    std::shared_ptr<BlockContainerWrapper> getContainer(block_container_wrapper_id_t id) {
        auto iter = blockContainers.find(id);
        if (iter == blockContainers.end()) return nullptr;
        return iter->second;
    }
    const std::shared_ptr<BlockContainerWrapper> getContainer(block_container_wrapper_id_t id) const {
        auto iter = blockContainers.find(id);
        if (iter == blockContainers.end()) return nullptr;
        return iter->second;
    }

    block_container_wrapper_id_t createNewContainer() {
        blockContainers.emplace(getNewContainerId(), std::make_shared<BlockContainerWrapper>(getLastCreatedContainerId()));
        return getLastCreatedContainerId();
    }

private:
    block_container_wrapper_id_t getNewContainerId() { return ++lastId; }
    block_container_wrapper_id_t getLastCreatedContainerId() { return lastId; }

    block_container_wrapper_id_t lastId;
    std::map<block_container_wrapper_id_t, std::shared_ptr<BlockContainerWrapper>> blockContainers;
};

#endif /* blockContainerManager_h */
