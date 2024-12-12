# Logic Graph Creator
 
## Building
### Prerequestites
QT6 is required to build this project. On MacOS or Linux you can get the libraries from your package manager.
On Windows you can either build QT yourself, or [download it](https://www.qt.io/download-qt-installer). Make sure that the QT compiler bin and lib directories are added to your system PATH variable [(guide)](https://youtu.be/rnogAji_I5E?si=yG-h8Z5A5h3anPI3&t=97).

### CMake
This project uses the CMake build system. Make sure that you have CMake and a compiler installed. 
You can let your IDE manage CMake, or run the commands yourself:

For development - `cmake -B build -DCMAKE_BUILD_TYPE=Debug`

For a "release" - `cmake -B build -DCMAKE_BUILD_TYPE=Release`
> **Windows**: If you don't want to use msvc, add the -G "Unix Makefiles" arguments to use GNU make and MinGW

Then (from the build folder) use the generated project files, or GNU make.
