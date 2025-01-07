# Conventions and feature usage guidelines

### Includes
Usage
- Angle brackets <> when including from libraries.
- Quotations "" when including internal files.
- Use relative paths when it makes sense for files to be in the same directory (or child directories). If you need to include a file from a parent folder, it is almost always in a different "module" of the project and should be included as an "absolute" path (relative to src). `"util/fastMath.h"` instead of `"../../util/fastMath.h"`.

Include Order
- Include any external libraries first
- If the file is a .cpp file, include the corresponding .h before any other local .h files
- Add line breaks between the different kinds of includes if you want

Include Path

- all paths should be case sensitive `util/fastMath.h` not `util/fastmath.h`

QT Includes
- Do not use QT includes ending with .h, use the ones specified by classes in the [Qt documentation](https://doc.qt.io/qt-6/classes.html)

### Header Guards

Use **#ifndef**
The preporsseser var should be named the name of the file with a '_' instead of '.'

### Naming

**camalCase** for file names
".h" for headers and ".cpp" for sources

var and function names should be **camalCase**
const var and enum states names should be **MACRO_CASE**
class, enum, and struct names should **PascalCase**

### Using Raw and Smart Pointers

Dont use new except when creating windows for qt to own.
You should use smart pointers for ownership. These may be custom smart pointers.

You can pass raw pointers around but objects reciving raw pointers should not be responsible for deallocating the memory.

### Enums

When using enum use the name of the enum :: the state
example `BlockType::AND`

### Class Declaration Order
Info on where to put public/private variables and function, and in what order

Order should be:
- friends
- public
- protected functions
- protected vars
- private functions
- private vars

### Assertions

Use assertions when you think is necessary. You should not over do it.

### Returning Nothing

When functions should sometimes return values you should use `std::optional`

### Types and Sizes

Using regular c++ types when the size is not highly important. You should use cstdint types when you need an exact size

### Tabs vs. Spaces

use tabs please

### Small TODO comments

`// TODO: info`

### Curly Brackets

You should open them on the same line. Not a new one.
```cpp
// do
if () {
}
```
```cpp
// dont
if ()
{
}
```
Place a space before curley bracket set for while, for, and functions
Space after if, while, for loops

### Constructor initializer lists


