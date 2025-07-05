#include "functions.h"

extern "C" {

const char* UUID = "c23a393f-3965-446c-99af-471db04ce50e";
const char* name = "Box";
const char* defaultParameters = "(\"w\": 1, \"h\": 1)";

bool generateCircuit() {
	int w = getParameter("w");
	int h = getParameter("h");

	setSize(w, h);

	return true;
}

}