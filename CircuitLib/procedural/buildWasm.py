import os
import pathlib

file_path = pathlib.Path(__file__).resolve()
os.chdir(file_path.parent)

file = "adder.cpp"

os.system("emcc " + file + " -Os -s STANDALONE_WASM -s EXPORTED_FUNCTIONS=\"['_generateCircuit', '_getUUID', '_getName', '_getDefaultParameters']\" --no-entry -o " + file[:file.rfind(".")] + ".wasm")

os.system("wasm-objdump -x " + file[:file.rfind(".")] + ".wasm")
