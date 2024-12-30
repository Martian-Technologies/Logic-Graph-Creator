### Includes
Usage
- Angle brackets <> when including from libraries.
- Quotations "" when including internal files.
- Use relative paths when it makes sense for files to be in the same directory (or child directories). If you need to include a file from a parent folder, it is almost always in a different "module" of the project and should be included as an "absolute" path (relative to src). `"util/fastMath.h"` instead of `"../../util/fastMath.h"`.

Include Order
- Include any external libraries first
- If the file is a .cpp file, include the corresponding .h before any other local .h files
- Add line breaks between the different kinds of includes if you want

QT Includes
- Do not use QT includes ending with .h, use the ones specified by classes in the Qt documentation

Case Sensitive Includes
- 

### Naming

### Pointers

### Enums

### Class Declaration Order

### Assertions and Returning "Failed"

### Types and Sizes
