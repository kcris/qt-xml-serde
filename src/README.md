# XSD to C++ Mapping Library for Qt5

A comprehensive library for generating C++ classes from XSD schemas with full XML serialization/deserialization support using Qt5.

## Features

- **Complete XSD Support**
  - Complex types with sequences, choices, and all
  - Simple types with restrictions and enumerations
  - Type extensions (inheritance)
  - Element substitution groups
  - Attributes (required/optional)
  - Recursive includes and imports
  - Namespace handling

- **C++ Code Generation**
  - Clean, readable C++ classes
  - Type-safe getters and setters
  - Automatic XML serialization/deserialization
  - Polymorphic type support using Qt smart pointers
  - Support for lists/collections (minOccurs/maxOccurs)

- **Runtime Library**
  - Base classes for XML serialization
  - Type factory for polymorphic instantiation
  - Helper functions for XSD built-in types
  - Document-level read/write API

## Project Structure

```
xsdqt/
├── runtime/                  # Runtime library
│   ├── XmlSerializable.h    # Base serialization interface
│   ├── XmlHelpers.h/.cpp    # Serialization utilities
│   └── XmlDocument.h        # Document-level API
├── generator/               # Code generator
│   ├── main.cpp            # CLI application
│   ├── XsdParser.h/.cpp    # XSD parser
│   └── CodeGenerator.h/.cpp # C++ code generator
├── tests/                   # Unit tests
│   ├── TestXmlSerialization.cpp
│   └── vehicle.xsd         # Sample schema
├── xsdqt.pro               # Root project file
├── xsdqt-runtime.pro       # Runtime library project
├── xsd2cpp.pro             # Generator project
└── tests.pro               # Tests project
```

## Building

### Prerequisites
- Qt 5.x (5.9 or later recommended)
- C++14 or C++17 compiler
- qmake

### Build Steps

```bash
# Clone or extract the project
cd xsdqt

# Build everything
qmake xsdqt.pro
make

# Install (optional)
sudo make install
```

This will build:
- `libxsdqt-runtime.a` - Runtime library
- `xsd2cpp` - Code generator executable
- `test_xsdqt` - Unit tests

## Usage

### 1. Generate C++ Code from XSD

```bash
./xsd2cpp -i schema.xsd -o generated/ -n MyNamespace
```

Options:
- `-i, --input <file>` - Input XSD file (required)
- `-o, --output <dir>` - Output directory (default: current directory)
- `-n, --namespace <name>` - C++ namespace (default: Generated)

### 2. Use Generated Code

```cpp
#include "XmlDocument.h"
#include "Fleet.h"  // Generated from XSD

// Create objects
QSharedPointer<Car> car = QSharedPointer<Car>::create();
car->setLicensePlate("ABC-123");
car->setYear(2024);
car->setManufacturer("TestMotors");
car->setNumDoors(4);
car->setTrunkCapacity(450.5);

// Save to XML
XsdQt::XmlDocument<Car> doc;
doc.setRoot(car);

QString errorMsg;
if (!doc.saveToFile("car.xml", &errorMsg)) {
    qWarning() << "Save failed:" << errorMsg;
}

// Load from XML
XsdQt::XmlDocument<Car> doc2;
if (doc2.loadFromFile("car.xml", &errorMsg)) {
    qDebug() << "License:" << doc2.root()->getLicensePlate();
    qDebug() << "Doors:" << doc2.root()->getNumDoors();
} else {
    qWarning() << "Load failed:" << errorMsg;
}
```

### 3. Polymorphic Types

For schemas with type inheritance and substitution groups:

```cpp
QSharedPointer<Fleet> fleet = QSharedPointer<Fleet>::create();
fleet->setName("My Fleet");

// Add different vehicle types
QSharedPointer<Car> car = QSharedPointer<Car>::create();
car->setLicensePlate("CAR-001");
fleet->addVehicle(car);

QSharedPointer<Truck> truck = QSharedPointer<Truck>::create();
truck->setLicensePlate("TRUCK-001");
fleet->addVehicle(truck);

// Save - polymorphic types are automatically handled
XsdQt::XmlDocument<Fleet> doc;
doc.setRoot(fleet);
doc.saveToFile("fleet.xml");
```

The generated XML will correctly use substitution group elements:

```xml
<fleet>
  <name>My Fleet</name>
  <car id="...">
    <licensePlate>CAR-001</licensePlate>
    ...
  </car>
  <truck id="...">
    <licensePlate>TRUCK-001</licensePlate>
    ...
  </truck>
</fleet>
```

## Example: Vehicle Schema

The included `vehicle.xsd` demonstrates:

1. **Base Type** (VehicleType)
   - Common fields: licensePlate, year, manufacturer
   - Required attribute: id

2. **Extended Types**
   - CarType extends VehicleType (adds numDoors, trunkCapacity)
   - TruckType extends VehicleType (adds payloadCapacity, numAxles)
   - MotorcycleType extends VehicleType (adds engineCC, hasSidecar)

3. **Substitution Groups**
   - Elements car, truck, motorcycle can substitute for vehicle element

4. **Collection Type**
   - FleetType contains multiple vehicles (polymorphic list)

## Generated Code Example

For a simple XSD type:

```xml
<xs:complexType name="PersonType">
  <xs:sequence>
    <xs:element name="firstName" type="xs:string"/>
    <xs:element name="lastName" type="xs:string"/>
    <xs:element name="age" type="xs:int"/>
  </xs:sequence>
  <xs:attribute name="id" type="xs:string" use="required"/>
</xs:complexType>
```

Generates:

```cpp
class Person : public XsdQt::XmlSerializable {
public:
    Person();
    
    QString getFirstName() const;
    void setFirstName(const QString& value);
    
    QString getLastName() const;
    void setLastName(const QString& value);
    
    int getAge() const;
    void setAge(int value);
    
    QString getId() const;
    void setId(const QString& value);
    
    void toXml(QXmlStreamWriter& writer) const override;
    bool fromXml(QXmlStreamReader& reader) override;
    QString xmlElementName() const override;
    QString xsdTypeName() const override;
    
private:
    QString m_firstName;
    QString m_lastName;
    int m_age;
    QString m_id;
};
```

## XSD Type Mapping

| XSD Type | C++ Type |
|----------|----------|
| xs:string | QString |
| xs:int, xs:integer | int |
| xs:long | qint64 |
| xs:short | qint16 |
| xs:byte | qint8 |
| xs:unsignedInt | quint32 |
| xs:unsignedLong | quint64 |
| xs:double | double |
| xs:float | float |
| xs:boolean | bool |
| xs:dateTime | QDateTime |
| xs:date | QDate |
| xs:time | QTime |
| Custom complex types | Generated C++ class |
| maxOccurs > 1 | QList<T> |
| Complex type with maxOccurs > 1 | QList<QSharedPointer<T>> |

## Running Tests

```bash
cd tests
./test_xsdqt
```

Tests cover:
- Simple type serialization
- Inheritance/polymorphism
- Collections
- Attributes
- Type conversions
- File I/O

## Advanced Features

### Custom Validation

Override `fromXml` to add custom validation:

```cpp
bool MyType::fromXml(QXmlStreamReader& reader) {
    if (!BaseType::fromXml(reader)) {
        return false;
    }
    
    // Custom validation
    if (m_age < 0 || m_age > 150) {
        return false;
    }
    
    return true;
}
```

### Namespace Handling

The generator preserves XML namespaces from XSD schemas. Generated code automatically handles namespace prefixes during serialization.

### Error Handling

All XML operations return bool and provide optional error messages:

```cpp
QString errorMsg;
if (!doc.loadFromFile("data.xml", &errorMsg)) {
    qCritical() << "Failed to load:" << errorMsg;
}
```

## Limitations

- Mixed content (text + elements) is not fully supported
- Some advanced XSD features (unions, anyType) have limited support
- Circular type references require careful handling

## Contributing

Contributions welcome! Areas for improvement:
- Additional XSD features
- Code generation optimizations
- More comprehensive tests
- CMake build support

## License

MIT License - feel free to use in commercial and open-source projects.

## Support

For issues, questions, or feature requests, please refer to the project documentation or submit an issue.
