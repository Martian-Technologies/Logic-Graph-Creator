#include "functions.h"

extern "C" {

const char* UUID = "8a92b940-456a-4d81-bc40-1f6e8bef4464";
const char* name = "And Gate";
const char* defaultParameters = "(\"size\": 1)";

bool generateCircuit() {
	int size = getParameter("size");
	logInfo(size)

	setSize(2, size);

	tryInsertBlock(1, 0, 0, 13);
	addConnectionOutput(1, 0, 1, 0);

	tryInsertBlock(0, 0, 0, 1);

	tryCreateConnection(0, 0, 1, 0);

	for (int i = 0; i < size; i++) {
		tryInsertBlock(-1, i, 0, 11);
		addConnectionInput(-1, i, 0, i);
		tryCreateConnection(-1, i, 0, 0);
	}

	return true;
}

}