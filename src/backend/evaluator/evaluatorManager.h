#ifndef evaluatorManager_h
#define evaluatorManager_h

#include "evaluator.h"

class EvaluatorManager {
public:
	inline SharedEvaluator getEvaluator(evaluator_id_t id) {
		auto iter = evaluators.find(id);
		if (iter == evaluators.end()) return nullptr;
		return iter->second;
	}
	inline const SharedEvaluator getEvaluator(evaluator_id_t id) const {
		auto iter = evaluators.find(id);
		if (iter == evaluators.end()) return nullptr;
		return iter->second;
	}

	inline evaluator_id_t createNewEvaluator(SharedCircuit circuit) {
		evaluators.emplace(getNewEvaluatorId(), std::make_shared<Evaluator>(getLastCreatedEvaluatorId(), circuit));
		return getLastCreatedEvaluatorId();
	}
	inline void destroyEvaluator(evaluator_id_t id) {
		auto iter = evaluators.find(id);
		if (iter != evaluators.end()) {
			evaluators.erase(iter);
		}
	}

private:
	evaluator_id_t getNewEvaluatorId() { return ++lastId; }
	evaluator_id_t getLastCreatedEvaluatorId() { return lastId; }

	evaluator_id_t lastId = 0;
	std::map<evaluator_id_t, SharedEvaluator> evaluators;
};

#endif /* evaluatorManager_h */
