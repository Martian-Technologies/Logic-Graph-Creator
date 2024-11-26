#ifndef blockContainerTools_h
#define blockContainerTools_h

#include <vector>
#include <string>

#include "blockContainer.h"

class BlockContainerTools {
private:
    struct Action;
public:
    enum ToolTypes {
        SELECT, // We will want to be able to interact will blocks. Maybe some kind of ui on the side.
        SINGLE_PLACE, // Place tools will also be able to delete blocks
        AREA_PLACE,
    };

    inline BlockContainerTools() : blockContainer(nullptr), selectedBlock(NONE), tool(SINGLE_PLACE), actionStack() {}

    inline void setBlockContainer(BlockContainer* blockContainer) {this->blockContainer = blockContainer; clearActions();}
    inline void selectBlock(BlockType selectedBlock) {this->selectedBlock = selectedBlock;}
    inline void selectTool(ToolTypes tool) {this->tool = tool; clearActions();}

    bool leftPress(Position pos);
    bool rightPress(Position pos);
    bool leftRelease(Position pos);
    bool rightRelease(Position pos);
    bool mouseMove(Position pos);
    bool keyPress(int keyId);
    bool keyRelease(int keyId);

private:
    inline void clearActions() {actionStack.clear();}
    inline void addAction(std::string name, Position position = Position()) {actionStack.emplace_back(name, position);}

    struct Action {
        inline Action(std::string name, Position position) : name(name), position(position) {}
        std::string name;
        Position position;
    };

    BlockContainer* blockContainer;
    BlockType selectedBlock;
    ToolTypes tool;
    std::vector<Action> actionStack;
};

#endif /* blockContainerTools_h */