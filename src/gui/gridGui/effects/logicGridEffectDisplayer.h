#ifndef logicGridEffectDisplayer_h
#define logicGridEffectDisplayer_h

#include <algorithm>
#include <QPainter>
#include <vector>

class LogicGridWindow;
#include "effect.h"

class LogicGridEffectDisplayer {
public:
    LogicGridEffectDisplayer() : sorted(false), effects() {}
    // Displays the effects onto the gridWindow.
    void display(QPainter& painter, const LogicGridWindow& gridWindow) {sort(); for (Effect* effect : effects) {effect->display(painter, gridWindow);}}
    // Sorts the effects by layer. Will automaticly get called by display().
    void sort() {if (!sorted) std::sort(effects.begin(), effects.end(), [](const Effect* a, const Effect* b) {return a->getLayer() < b->getLayer();}); sorted = true;}

    // Removes all of the effects.
    inline void clear() {effects.clear(); sorted = false;}
    // Adds a effects. (Will make a copy of the effect)
    inline void addEffect(Effect* effect) {effects.push_back(effect); sorted = false;}
    // This will delete the pointer.
    inline void removeEffect(int id) {sorted = false; for (Effect*& effect : effects) if (effect->getId() == id) deleteEffect(effect);}

    inline Effect* getEffect(int id) {for (Effect* effect : effects) {if (effect->getId() == id) return effect;} return nullptr;}
    inline const Effect* getEffect(int id) const {for (Effect* effect : effects) {if (effect->getId() == id) return effect;} return nullptr;}
    inline void changeEffectLayer(int id, int layer) {sorted = false; }
private:
    inline void deleteEffect(Effect*& effect) {delete effect; effect = effects.back(); effects.pop_back();}
    inline void deleteEffectNoFree(Effect*& effect) {effect = effects.back(); effects.pop_back();}

    bool sorted;
    std::vector<Effect*> effects;
};

#endif /* logicGridEffectDisplayer_h */
