# Building
## Introduction
Gatality compiles on MacOS, Windows, and Linux. There are a few things you have to do in order to get it up and running:
1. Download the code
2. Install the dependencies (QT)
3. Set up the CMake build system
Things will generally run smoothly on MacOS and Linux, there are a few more steps on Windows.

## Getting the source
You can start by git cloning the repository. A git client or the command line should work.

We use git submodules so that we can keep the code of some of our smaller dependencies in the project, and so that they can be compiled by our build system.
After cloning the main project, clone the submodules with these commands:
- `git submodule init`
- `git submodule update --init --recursive`
> If you have a client that can do this, or clone the repo with the submodules to begin with, that's fine too.

## System dependencies
QT6 is required to build this project.
### MacOS and Linux
You can get the QT libraries from your package manager (brew on MacOS, distro dependent for Linux).
### Windows
You can either build QT yourself, or download and install it with QT's [installer](https://www.qt.io/download-qt-installer).
> Gatality only builds with MSVC on Windows. Make sure you install QT with MSVC and not MinGW.
Unfortunately QT's installer does not add its directories to your environment variables, and CMake will not be able to find it, so you need to add it youself.
- You need to add your QT version's lib and bin directories for MSVC to your path environment variable. This [guide](https://youtu.be/rnogAji_I5E?si=yG-h8Z5A5h3anPI3&t=97) will show you how.
- You also need to add a new environment variable called `QTDIR` which is the directory of your QT version and compiler.

For example:
- Your PATH should include `C:\Qt\6.8.1\msvc2022_64\bin` and `C:\Qt\6.8.1\msvc2022_64\lib`
- You should have an additional environment variable called `QTDIR` which is `C:\Qt\6.8.1\msvc2022_64\`
Either system or user environment variables will work.

## Setting up the CMake build system
You need the CMake build system and a C++ compiler to build this project.
### MacOS and Linux
You can get a compiler and CMake from your package manager. Any compiler should work.
### Windows
You can install CMake using the [CMake Binary Installer](https://cmake.org/download/).

You also need the MSVC compiler and its CMake build tools. You can download the installer from [Microsoft](https://visualstudio.microsoft.com/downloads/)
> In the Visual Studio installer, you probably want to do a custom installation, just make sure that you have C++ and CMake tools.
 
## Using CMake
Even if you are going to have your IDE manage CMake, it's a good idea to try running it from the terminal first.
> On Windows, Microsoft's build system will not be set as the CMake compiler by default. Either add MSBuild to your system variables, or use the "Developer Command Prompt for VS ..." application (which already has the variable set up) instead of your regular terminal. 

1. Configure - `cmake --preset debug`
2. Then Build - `cmake --build --preset debug`
3. Run the executable that was generated somewhere in the `build` directory. On Windows this is probably in the `Debug` subdirectory.

You can also build for release with `release` preset
> Works for MacOS, Windows (MSVC), and Linux

## Setting up CMake in an IDE
TODO

## Notes
If your error highlighting or IDE integration is showing red, make sure you have already compiled the project (to generate QT moc header files) and the compile_commands.json in the build folder is being recognized (default for most lsp)
