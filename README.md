# Python to C++ Decompiler

This is a basic Python to C++ decompiler that converts Python code to equivalent C++ code. The decompiler handles common Python constructs and converts them to their C++ counterparts.

## Features

- Converts basic Python syntax to C++
- Handles common control structures (if, else, elif, while, for)
- Converts Python functions to C++ functions
- Supports basic Python operators and their C++ equivalents
- Maintains code indentation
- Handles basic type conversions

## Supported Conversions

- Python `print()` → C++ `std::cout`
- Python `len()` → C++ `.size()`
- Python `not` → C++ `!`
- Python `and` → C++ `&&`
- Python `or` → C++ `||`
- Basic for loops with range
- Function definitions
- Basic type mappings (int, str, float, list, dict, bool)

## Building the Project

```bash
# Create a build directory
mkdir build
cd build

# Generate build files
cmake ..

# Build the project
cmake --build .
```

## Usage

```bash
./py2cpp <input_python_file> <output_cpp_file>
```

Example:
```bash
./py2cpp input.py output.cpp
```

## Limitations

1. This is a basic decompiler and doesn't support all Python features
2. Complex Python constructs may not be correctly converted
3. Some Python-specific features have no direct C++ equivalent
4. The generated C++ code may require manual adjustments
5. Python's dynamic typing is converted to `auto` where type cannot be determined
6. List comprehensions and lambda functions are not supported
7. Python's standard library functions may need manual conversion

## Example

Input Python code:
```python
def calculate_sum(a, b):
    if a > b:
        return a + b
    else:
        return b - a

for i in range(10):
    print(calculate_sum(i, 5))
```

Generated C++ code:
```cpp
#include <iostream>
#include <string>
#include <vector>
#include <map>

auto calculate_sum(int a, int b) {
    if (a > b) {
        return a + b;
    } else {
        return b - a;
    }
}

int main() {
    for(int i = 0; i < 10; i++) {
        std::cout << calculate_sum(i, 5) << std::endl;
    }
    return 0;
}
``` 
