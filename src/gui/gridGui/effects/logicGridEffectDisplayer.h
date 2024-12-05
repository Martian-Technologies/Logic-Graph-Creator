#ifndef logicGridEffectDisplayer_h
#define logicGridEffectDisplayer_h

#include <vector>
#include <algorithm>
#include <QPainter>

#include "effect.h"
#include "../logicGridWindowData.h"

class LogicGridEffectDisplayer {
public:
    LogicGridEffectDisplayer() : sorted(false), effects() {}
    void display(QPainter& painter, const LogicGridWindowData& data) {sort(); for (Effect* effect : effects) {effect->display(painter, data);}}
    void sort() {if (!sorted) std::sort(effects.begin(), effects.end(), [](const Effect* a, const Effect* b) {return a->getId() < b->getId();}); sorted = true;}

    inline void clear() {effects.clear(); sorted = false;}
    inline void addEffect(Effect* effect) {effects.push_back(effect); sorted = false;}
    // this will delete the pointer.
    inline void removeEffect(int id) {sorted = false; for (Effect*& effect : effects) if (effect->getId() == id) deleteEffect(effect);}
    // this will not delete the pointer.
    inline void removeEffectNoFree(int id) {sorted = false; for (Effect*& effect : effects) if (effect->getId() == id) deleteEffectNoFree(effect);}

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
