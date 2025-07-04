import os

file_path = os.path.realpath(__file__)
os.chdir("/".join(file_path.split("/")[:-1]))

os.system("emcc andGate.cpp -Os -s STANDALONE_WASM -s EXPORTED_FUNCTIONS=\"['_generateCircuit', '_getUUID', '_getName']\" --no-entry -o andGate.wasm")

os.system("wasm-objdump -x andGate.wasm")
