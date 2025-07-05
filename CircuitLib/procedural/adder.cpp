#include "functions.h"

extern "C" {

const char* UUID = "4c174b4c-584f-4480-9f29-97bab418d073";
const char* name = "Adder";
const char* defaultParameters = "(\"size\": 1)";

bool generateCircuit() {
	int size = getParameter("size");

	setSize(2, size*2);

	for (int i = 0; i < size; i++) {
		tryInsertBlock(-1, i*2, 0, BlockType::SWITCH); // A
		tryInsertBlock(-2, i*2, 0, BlockType::SWITCH); // B
		addConnectionInput(-1, i*2, 0, i);
		addConnectionInput(-2, i*2, 0, i+size);
		tryInsertBlock(0, i*2, 0, BlockType::XOR); // A ^ B
		tryCreateConnection(-1, i*2, 0, i*2);
		tryCreateConnection(-2, i*2, 0, i*2);
		tryInsertBlock(2, i*2, 0, BlockType::LIGHT); // out
		addConnectionOutput(2, i*2, 1, i);
		if (i == 0) {
			tryCreateConnection(0, i*2, 2, i*2); // A ^ B
		} else {
			tryInsertBlock(1, i*2, 0, BlockType::XOR); // A ^ B ^ C
			tryCreateConnection(0, i*2, 1, i*2);
			tryCreateConnection(2, i*2-1, 1, i*2);
			tryCreateConnection(1, i*2, 2, i*2);
		}
		if (i + 1 < size) {
			if (i == 0) {
				tryInsertBlock(2, i*2+1, 0, BlockType::AND); // A & B
				tryCreateConnection(-1, i*2, 2, i*2+1);
				tryCreateConnection(-2, i*2, 2, i*2+1);
			} else {
				tryInsertBlock(0, i*2+1, 0, BlockType::AND); // A & B
				tryCreateConnection(-1, i*2, 0, i*2+1);
				tryCreateConnection(-2, i*2, 0, i*2+1);
				
				tryInsertBlock(1, i*2+1, 0, BlockType::AND); // (A ^ B) & C
				tryCreateConnection(0, i*2, 1, i*2+1);
				tryCreateConnection(2, i*2-1, 1, i*2+1);
				tryInsertBlock(2, i*2+1, 0, BlockType::OR); // ((A ^ B) & C) || (A & B)
				tryCreateConnection(0, i*2+1, 2, i*2+1);
				tryCreateConnection(1, i*2+1, 2, i*2+1);
			}
		}
	}

	return true;
}

}