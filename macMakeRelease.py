import os

os.system("/opt/homebrew/bin/cmake -DCMAKE_BUILD_TYPE=Release -DGATALITY_BUILD_APP=ON -G Ninja -S./ -B./build-release")
os.system("/opt/homebrew/bin/cmake --build /Users/ben/Documents/GitHub/Logic-Graph-Creator/build-release --parallel 8 --config Release --target Gatality --")

os.system("cd ./build-release; cpack; cd ../")
