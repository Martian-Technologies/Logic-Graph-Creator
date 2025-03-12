#ifndef selectionHelperTool_h
#define selectionHelperTool_h

#include "backend/selection.h"
#include "../circuitToolHelper.h"

class SelectionHelperTool : public CircuitToolHelper {
public:
	void activate() override { CircuitToolHelper::activate(); }
	inline bool isFinished() const { return (bool)createdSelection; }
	inline const SharedSelection getSelection() const { return createdSelection; }

protected:
	void reset() override { CircuitToolHelper::reset(); createdSelection = nullptr; }

	void finished(SharedSelection createdSelection) {
		this->createdSelection = createdSelection;
		toolStackInterface->popTool();
	}

private:
	SharedSelection createdSelection;
};

typedef std::shared_ptr<SelectionHelperTool> SharedSelectionHelperTool;

#endif /* selectionHelperTool_h */
