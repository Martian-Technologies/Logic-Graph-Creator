import os

file_path = os.path.realpath(__file__)
os.chdir("/".join(file_path.split("/")[:-1]))

file = "edgeDetector.cpp"

os.system("emcc " + file + " -Os -s STANDALONE_WASM -s EXPORTED_FUNCTIONS=\"['_generateCircuit', '_getUUID', '_getName', '_getDefaultParameters']\" --no-entry -o " + file[:file.rfind(".")] + ".wasm")

os.system("wasm-objdump -x " + file[:file.rfind(".")] + ".wasm")
