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
    void display(QPainter& painter, const LogicGridWindow& gridWindow) {sort(); for (EffectUniquePtr& effect : effects) {effect->display(painter, gridWindow);}}
    // Sorts the effects by layer. Will automaticly get called by display().
    void sort() {if (!sorted) std::sort(effects.begin(), effects.end(), [](const EffectUniquePtr& a, const EffectUniquePtr& b) {return a->getLayer() < b->getLayer();}); sorted = true;}

    // Removes all of the effects.
    inline void clear() {effects.clear(); sorted = false;}
    // Adds a effects. (Will make a copy of the effect)
    inline void addEffect(const Effect& effect) {effects.push_back(std::move(effect.clone())); sorted = false;}
    // This will delete the pointer.
    inline void removeEffect(int id) {for (EffectUniquePtr& effect : effects) if (effect->getId() == id) removeEffect(effect);}

    inline const bool hasEffect(int id) {for (EffectUniquePtr& effect : effects) {if (effect->getId() == id) return true;} return false;}
    inline const EffectUniquePtr& getEffect(int id) {for (EffectUniquePtr& effect : effects) {if (effect->getId() == id) return effect;} assert(false);}
    inline const EffectUniquePtr& getEffect(int id) const {for (const EffectUniquePtr& effect : effects) {if (effect->getId() == id) return effect;} assert(false);}
    inline void changeEffectLayer(int id, int layer) {sorted = false; getEffect(id)->setLayer(layer);}
private:
    inline void removeEffect(EffectUniquePtr& effect) {sorted = false; effect = std::move(effects.back()); effects.pop_back();}

    bool sorted;
    std::vector<EffectUniquePtr> effects;
};

#endif /* logicGridEffectDisplayer_h */
