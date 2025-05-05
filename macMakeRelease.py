import os

os.system("/opt/homebrew/bin/cmake -DCMAKE_BUILD_TYPE=Release -DGATALITY_BUILD_APP=ON -G Ninja -S./ -B./build-release")
os.system("/opt/homebrew/bin/cmake --build /Users/ben/Documents/GitHub/Logic-Graph-Creator/build-release --parallel 8 --config Release --target Gatality --")

try:
	os.makedirs("./releaseFiles/mac")
except:
	pass

import shutil


shutil.copytree("./build-release/Gatality.app", "./releaseFiles/mac/Gatality.app", dirs_exist_ok=True)

shutil.copytree("./build-release/resources", "./releaseFiles/mac/Gatality.app/Contents/MacOS/resources", dirs_exist_ok=True)

import subprocess
import shutil

APP_PATH = "releaseFiles/mac/Gatality.app"
EXECUTABLE_NAME = "Gatality"

FRAMEWORKS_DIR = os.path.join(APP_PATH, 'Contents', 'Frameworks')
MACOS_DIR = os.path.join(APP_PATH, 'Contents', 'MacOS')
EXECUTABLE_PATH = os.path.join(MACOS_DIR, EXECUTABLE_NAME)

os.makedirs(FRAMEWORKS_DIR, exist_ok=True)

def get_linked_libraries(binary_path):
    output = subprocess.check_output(['otool', '-L', binary_path], text=True)
    lines = output.splitlines()[1:]  # Skip the first line (binary name)
    libraries = [line.strip().split(' ')[0] for line in lines if '/usr/local' in line or '/opt/' in line or '/Library/Frameworks' in line]
    return libraries

def copy_and_relink_lib(lib_path):
    lib_name = os.path.basename(lib_path)
    dest_path = os.path.join(FRAMEWORKS_DIR, lib_name)
    
    if not os.path.exists(dest_path):
        print(f'Copying {lib_path} -> {dest_path}')
        shutil.copy(lib_path, dest_path)

    # Make the binary refer to the internal Frameworks path
    new_id = f'@executable_path/../Frameworks/{lib_name}'
    print(f'Updating link for {lib_path} to {new_id}')
    subprocess.run(['install_name_tool', '-change', lib_path, new_id, EXECUTABLE_PATH], check=True)

print(f'Analyzing: {EXECUTABLE_PATH}')
libs = get_linked_libraries(EXECUTABLE_PATH)

print(libs)

for lib in libs:
	if os.path.exists(lib):
		print(lib)
		copy_and_relink_lib(lib)
	else:
		print(f'⚠️ Library not found: {lib}')

os.system("cd ./releaseFiles/mac; zip -9 -y -r -q ./Gatality.app.zip ./Gatality.app")

