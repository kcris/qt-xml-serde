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
