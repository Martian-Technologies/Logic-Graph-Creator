#include "functions.h"

extern "C" {

const char* UUID = "8a92b940-456a-4d81-bc40-1f6e8bef4464";
const char* name = "And Gate";

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