#ifndef blockContainerTools_h
#define blockContainerTools_h

#include <vector>
#include <string>

#include "blockContainer.h"

class BlockContainerTools {
private:
    struct Action;
    struct KeyAction;
    struct MouseAction;
public:
    enum ToolTypes {
        SELECT, // We will want to be able to interact will blocks. Maybe some kind of ui on the side.
        SINGLE_PLACE, // Place tools will also be able to delete blocks
        AREA_PLACE,
    };

    inline BlockContainerTools() : blockContainer(nullptr), selectedBlock(NONE), tool(SINGLE_PLACE), mouseActionStack() {}

    inline void setBlockContainer(BlockContainer* blockContainer) {this->blockContainer = blockContainer; clearAllActions();}
    inline void selectBlock(BlockType selectedBlock) {this->selectedBlock = selectedBlock;}
    inline void selectTool(ToolTypes tool) {this->tool = tool; clearAllActions();}

    bool leftPress(Position pos);
    bool rightPress(Position pos);
    bool leftRelease(Position pos);
    bool rightRelease(Position pos);
    bool mouseMove(Position pos);
    bool keyPress(int keyId);
    bool keyRelease(int keyId);

private:
    inline void clearAllActions() {actionStack.clear(); keyActionStack.clear(); mouseActionStack.clear();}
    inline void clearActions() {actionStack.clear();}
    inline void clearKeyActions() {keyActionStack.clear();}
    inline void clearMouseActions() {mouseActionStack.clear();}
    inline void addAction(std::string name) {actionStack.emplace_back(name);}
    inline void addKeyAction(std::string name, int key) {keyActionStack.emplace_back(name, key);}
    inline void addMouseAction(std::string name, Position position) {mouseActionStack.emplace_back(name, position);}

    struct Action {
        inline Action(std::string name) : name(name) {}
        std::string name;
    };

    struct KeyAction {
        inline KeyAction(std::string name, int key) : name(name), key(key) {}
        std::string name;
        int key;
    };

    struct MouseAction {
        inline MouseAction(std::string name, Position position) : name(name), position(position) {}
        std::string name;
        Position position;
    };

    BlockContainer* blockContainer;
    BlockType selectedBlock;
    ToolTypes tool;
    std::vector<Action> actionStack;
    std::vector<KeyAction> keyActionStack;
    std::vector<MouseAction> mouseActionStack;
};

#endif /* blockContainerTools_h */