#include "functions.h"

extern "C" {

bool generateCircuit() {
	setSize(2, 2);

	tryInsertBlock(-1, 1, 0, 11);
	addConnectionInput(-1, 1, 0, 0);

	tryInsertBlock(-1, -1, 0, 11);
	addConnectionInput(-1, -1, 0, 1);

	tryInsertBlock(1, 0, 0, 13);
	addConnectionOutput(1, 0, 1, 0);

	tryInsertBlock(0, 0, 0, 1);

	tryCreateConnection(-1, 1, 0, 0);
	tryCreateConnection(-1, -1, 0, 0);
	tryCreateConnection(0, 0, 1, 0);

	return true;
}

}