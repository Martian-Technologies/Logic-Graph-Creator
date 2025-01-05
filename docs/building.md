# Building

## Prerequestites
QT6 is required to build this project. On MacOS or Linux you can get the libraries from your package manager.
On Windows you can either build QT yourself, or [download it](https://www.qt.io/download-qt-installer). Make sure that the QT compiler bin and lib directories are added to your system PATH variable [(guide)](https://youtu.be/rnogAji_I5E?si=yG-h8Z5A5h3anPI3&t=97).
This project also requires the [Vulkan SDK](https://www.lunarg.com/vulkan-sdk/) to build.


## CMake
This project uses the CMake build system. Make sure that you have CMake and a compiler installed. 
You can let your IDE manage CMake, or run the commands yourself:

For development - `cmake --workflow --preset debug`

For a "release" - `cmake --workflow --preset release`
> Tested for MacOS, Windows (MSVC), and Linux

Temp: for using XCode you should use the command `cmake -G Xcode <dir of CMakeLists.txt>`

## Setting up CMake in an IDE

## Notes
If your error highlighting or IDE integration is showing red, make sure you have already compiled the project (to generate QT moc header files) and the compile_commands.json in the build folder is being recognized (default for most lsp)
