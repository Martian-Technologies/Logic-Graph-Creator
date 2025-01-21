# Building

## Prerequestites
QT6 is required to build this project. On MacOS or Linux you can get the libraries from your package manager.
On Windows you can either build QT yourself, or [download it](https://www.qt.io/download-qt-installer). Make sure that the QT compiler bin and lib directories are added to your system PATH variable [(guide)](https://youtu.be/rnogAji_I5E?si=yG-h8Z5A5h3anPI3&t=97).
> On Windows: Gatality only builds with MSVC on Windows. Make sure you install QT with MSVC and not MinGW. Also make sure you have installed [Visual Studio Build Tools](https://visualstudio.microsoft.com/downloads/)

## Getting the source
You can start by cloning this repo. We use git submodules to keep all of the other dependencies within the project.

After cloning the main project, clone the submodules with these commands:
- `git submodule init`
- `git submodule update --init --recursive`
> If you have a client that can do this, or clone the repo with the submodules to begin with, that's fine too.

## CMake
This project uses the CMake build system. Make sure that you have CMake and a compiler installed. 
You can let your IDE manage CMake, or run the commands yourself:

1. Configure - `cmake --preset debug`
2. Then Build - `cmake --build --preset debug`

You can also build for release with `release` preset
> Tested for MacOS, Windows (MSVC), and Linux

## Setting up CMake in an IDE
TODO

## Notes
If your error highlighting or IDE integration is showing red, make sure you have already compiled the project (to generate QT moc header files) and the compile_commands.json in the build folder is being recognized (default for most lsp)
