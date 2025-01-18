#if defined(__clang__) || defined(__GNUC__)
  #define UNREACHABLE() __builtin_unreachable()
#elif defined(_MSC_VER)
  #define UNREACHABLE() __assume(0)
#else
  #include <stdlib.h>
  #define UNREACHABLE() abort()  // Fallback that terminates the program.
#endif