#ifndef gridRenderer_h
#define gridRenderer_h

class LogicGridWindow;

class GridRenderer {
public:
    GridRenderer(LogicGridWindow* logicGridWindow) : logicGridWindow(logicGridWindow) {}

    void renderGrid() const;

private:
    LogicGridWindow* logicGridWindow;

};

#endif /* gridRenderer_h */