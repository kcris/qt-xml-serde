# Project Structure

Complete directory layout for the XSD to C++ Mapping Library.

```
xsdqt/
│
├── README.md                      # Main documentation
├── GETTING_STARTED.md             # Quick start guide
├── PROJECT_STRUCTURE.md           # This file
├── LICENSE                        # MIT License
│
├── build.sh                       # Build script
├── xsdqt.pro                      # Root qmake project file
│
├── runtime/                       # Runtime library (required at runtime)
│   ├── xsdqt-runtime.pro         # Runtime library project file
│   ├── XmlSerializable.h         # Base interface for all generated classes
│   ├── XmlHelpers.h              # Serialization helper functions
│   ├── XmlHelpers.cpp            # Implementation
│   └── XmlDocument.h             # Template for document-level I/O
│
├── generator/                     # Code generator (build-time only)
│   ├── xsd2cpp.pro               # Generator project file
│   ├── main.cpp                  # CLI entry point
│   ├── XsdParser.h               # XSD schema parser
│   ├── XsdParser.cpp             # Implementation
│   ├── CodeGenerator.h           # C++ code generation
│   └── CodeGenerator.cpp         # Implementation
│
├── tests/                         # Unit tests
│   ├── tests.pro                 # Test project file
│   ├── TestXmlSerialization.cpp  # Main test suite
│   ├── vehicle.xsd               # Sample schema for testing
│   └── fleet_sample.xml          # Sample XML document
│
├── examples/                      # Example applications
│   ├── person.xsd                # Simple person schema
│   ├── example.cpp               # Example usage code
│   └── example.pro               # Example project file
│
├── generated/                     # Generated code goes here (by convention)
│   └── .gitkeep                  # (created during code generation)
│
├── build/                         # Build output directory
│   ├── libxsdqt-runtime.a        # Built runtime library
│   ├── xsd2cpp                   # Built generator executable
│   └── test_xsdqt                # Built test executable
│
└── docs/                          # Additional documentation
    ├── API.md                    # API reference
    ├── XSD_SUPPORT.md            # Supported XSD features
    └── EXAMPLES.md               # More examples
```

## Directory Purposes

### `/runtime`
**Purpose**: Core library that must be linked with generated code and applications.

**Files**:
- `XmlSerializable.h` - Abstract base class and factory pattern for polymorphism
- `XmlHelpers.h/.cpp` - Utility functions for reading/writing XML elements
- `XmlDocument.h` - Template class for document-level operations

**Dependencies**: Qt5 Core, Qt5 XML

**Build output**: `libxsdqt-runtime.a` (static library)

### `/generator`
**Purpose**: Command-line tool to parse XSD and generate C++ code.

**Files**:
- `main.cpp` - Argument parsing and workflow orchestration
- `XsdParser.h/.cpp` - XSD file parsing, include resolution
- `CodeGenerator.h/.cpp` - C++ header/implementation generation

**Dependencies**: Qt5 Core, Qt5 XML

**Build output**: `xsd2cpp` executable

### `/tests`
**Purpose**: Comprehensive test suite validating all features.

**Coverage**:
- Basic serialization/deserialization
- Type inheritance and polymorphism
- Collections (lists)
- Attributes
- Type conversions
- File I/O
- Error handling

**Framework**: Qt Test

**Build output**: `test_xsdqt` executable

### `/examples`
**Purpose**: Sample applications demonstrating library usage.

**Includes**:
- Simple schemas (person, address)
- Complex schemas (vehicles with inheritance)
- Complete applications with build files

### `/generated`
**Purpose**: Default output directory for generated code (by convention).

Users can specify any output directory with `-o` flag, but this is the recommended location for examples and tests.

### `/build`
**Purpose**: Build artifacts and temporary files.

This directory is created by the build script and contains:
- Compiled libraries
- Executables
- Object files
- Makefiles

Should be in `.gitignore`.

## File Naming Conventions

### XSD Files
- Use lowercase with underscores: `vehicle_fleet.xsd`
- Match namespace to filename when possible
- Keep related schemas in same directory

### Generated Files
- Header: `{ClassName}.h` (e.g., `Vehicle.h`)
- Implementation: `{ClassName}.cpp` (e.g., `Vehicle.cpp`)
- Class names use PascalCase: `VehicleType` → `Vehicle`
- Member variables use camelCase with `m_` prefix: `m_licensePlate`

### Project Files
- Runtime library: `xsdqt-runtime.pro`
- Generator: `xsd2cpp.pro`
- Tests: `tests.pro`
- Examples: descriptive name like `person-example.pro`

## Build Outputs

### Static Library: `libxsdqt-runtime.a`
**Location**: `build/libxsdqt-runtime.a`

**Purpose**: Must be linked with:
- Generated code
- User applications using generated code

**Usage**:
```qmake
LIBS += -L$$PWD/build -lxsdqt-runtime
```

### Generator Executable: `xsd2cpp`
**Location**: `build/xsd2cpp`

**Purpose**: Generates C++ code from XSD schemas

**Usage**:
```bash
./build/xsd2cpp -i schema.xsd -o generated/ -n MyNamespace
```

## Integration Patterns

### Pattern 1: Single Project
```
myproject/
├── myproject.pro
├── main.cpp
├── schemas/
│   └── myschema.xsd
├── generated/           # Generated code here
│   ├── MyType.h
│   └── MyType.cpp
└── xsdqt/              # Library as subdirectory
    ├── runtime/
    └── ...
```

Project file:
```qmake
include(xsdqt/runtime/xsdqt-runtime.pro)
SOURCES += generated/*.cpp
HEADERS += generated/*.h
```

### Pattern 2: Separate Library Project
```
workspace/
├── xsdqt/              # Library project
├── mylib/              # Your library using XSD
│   ├── schemas/
│   └── generated/
└── myapp/              # Application using your library
```

### Pattern 3: System-Wide Installation
```bash
cd xsdqt
./build.sh install      # Installs to /usr/local
```

Then in any project:
```qmake
LIBS += -lxsdqt-runtime
INCLUDEPATH += /usr/local/include/xsdqt
```

## Development Workflow

### Typical Development Cycle

1. **Design XSD Schema**
   ```bash
   vim schemas/mydata.xsd
   ```

2. **Generate Code**
   ```bash
   ./xsd2cpp -i schemas/mydata.xsd -o generated/ -n MyApp
   ```

3. **Add to Project**
   ```qmake
   SOURCES += generated/*.cpp
   HEADERS += generated/*.h
   ```

4. **Use in Application**
   ```cpp
   #include "generated/MyData.h"
   // Use MyApp::MyData class
   ```

5. **Build and Test**
   ```bash
   qmake && make && ./myapp
   ```

6. **Iterate**: Modify XSD, regenerate, rebuild

### Regenerating Code

When XSD changes:
```bash
# Regenerate
./xsd2cpp -i schemas/mydata.xsd -o generated/ -n MyApp

# Rebuild
make clean
qmake && make
```

**Note**: Generated code is overwritten. Don't manually edit generated files.

## Customization Points

### Where to Add Custom Code

1. **Validation**: Override `fromXml()` in derived class
2. **Business Logic**: Create separate utility classes
3. **UI Adapters**: Create wrapper classes
4. **Converters**: Extend `XmlHelpers` in separate file

### Example: Custom Validation Layer
```
myproject/
├── generated/          # Don't edit these
│   ├── Person.h
│   └── Person.cpp
└── validation/         # Custom code here
    ├── PersonValidator.h
    └── PersonValidator.cpp
```

## Dependencies

### Build Time
- Qt 5.x development files
- C++14/17 compiler (g++, clang, MSVC)
- qmake
- make

### Runtime
- Qt 5.x libraries (Core, XML)
- C++14/17 standard library

### Optional
- Doxygen (for documentation)
- Qt Creator (for development)

## Platform-Specific Notes

### Linux
```bash
# Install dependencies (Ubuntu/Debian)
sudo apt-get install qt5-default qt5-qmake

# Build
./build.sh
```

### macOS
```bash
# Install Qt via Homebrew
brew install qt5

# Add Qt to PATH
export PATH="/usr/local/opt/qt/bin:$PATH"

# Build
./build.sh
```

### Windows
```powershell
# Use Qt installer from qt.io
# Open Qt Command Prompt

# Build
qmake xsdqt.pro
nmake  # or jom for parallel builds
```

## Size and Performance

### Library Size
- Runtime library: ~100-200 KB (static)
- Generator: ~500 KB - 1 MB
- Generated code: ~5-20 KB per complex type

### Performance
- Parsing: Handles multi-MB XML files efficiently
- Generation: Processes complex schemas in seconds
- Memory: Minimal overhead beyond Qt's XML classes

## Maintenance

### Adding New XSD Features

1. Update `XsdParser` to recognize new elements
2. Extend `CodeGenerator` to emit appropriate C++
3. Add tests in `TestXmlSerialization`
4. Update documentation

### Version Control

**Include in repository**:
- All source files
- XSD schemas
- Project files (.pro)
- Documentation
- Build scripts

**Exclude from repository** (.gitignore):
```
build/
generated/
*.o
*.a
Makefile*
*.user
```

## Summary

This structure separates concerns:
- **Runtime**: Pure library code, stable API
- **Generator**: Build tool, rarely changes
- **Generated**: Output, never manually edited
- **Tests**: Validation and examples
- **Examples**: User documentation

The design allows:
- Easy updates to XSD support
- Stable runtime API
- Clean separation of generated/manual code
- Simple integration into user projects
