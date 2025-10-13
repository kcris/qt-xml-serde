# Getting Started with XSD to C++ Mapping Library

This guide will walk you through creating your first XSD-based application.

## Quick Start (5 minutes)

### 1. Build the Library

```bash
chmod +x build.sh
./build.sh
```

### 2. Create Your XSD Schema

Create a file `person.xsd`:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<xs:schema xmlns:xs="http://www.w3.org/2001/XMLSchema"
           targetNamespace="http://example.com/person"
           xmlns:tns="http://example.com/person"
           elementFormDefault="qualified">

    <xs:complexType name="AddressType">
        <xs:sequence>
            <xs:element name="street" type="xs:string"/>
            <xs:element name="city" type="xs:string"/>
            <xs:element name="zipCode" type="xs:string"/>
        </xs:sequence>
    </xs:complexType>

    <xs:complexType name="PersonType">
        <xs:sequence>
            <xs:element name="firstName" type="xs:string"/>
            <xs:element name="lastName" type="xs:string"/>
            <xs:element name="age" type="xs:int"/>
            <xs:element name="email" type="xs:string" minOccurs="0"/>
            <xs:element name="address" type="tns:AddressType"/>
        </xs:sequence>
        <xs:attribute name="id" type="xs:string" use="required"/>
    </xs:complexType>

    <xs:element name="person" type="tns:PersonType"/>
    
</xs:schema>
```

### 3. Generate C++ Code

```bash
./build/xsd2cpp -i person.xsd -o generated/ -n MyApp
```

This creates:
- `generated/Person.h` / `generated/Person.cpp`
- `generated/Address.h` / `generated/Address.cpp`

### 4. Create Your Application

Create `myapp.pro`:

```qmake
QT += core xml
QT -= gui

TARGET = myapp
CONFIG += console c++14
CONFIG -= app_bundle

TEMPLATE = app

# Include paths
INCLUDEPATH += runtime generated

# Link runtime library
LIBS += -L. -lxsdqt-runtime

SOURCES += \
    main.cpp \
    generated/Person.cpp \
    generated/Address.cpp

HEADERS += \
    generated/Person.h \
    generated/Address.h
```

Create `main.cpp`:

```cpp
#include <QCoreApplication>
#include <QDebug>
#include "XmlDocument.h"
#include "Person.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    
    // Create a person
    QSharedPointer<MyApp::Person> person = 
        QSharedPointer<MyApp::Person>::create();
    
    person->setId("P001");
    person->setFirstName("John");
    person->setLastName("Doe");
    person->setAge(30);
    person->setEmail("john.doe@example.com");
    
    // Create address
    QSharedPointer<MyApp::Address> address = 
        QSharedPointer<MyApp::Address>::create();
    address->setStreet("123 Main St");
    address->setCity("Springfield");
    address->setZipCode("12345");
    
    person->setAddress(address);
    
    // Save to XML
    XsdQt::XmlDocument<MyApp::Person> doc;
    doc.setRoot(person);
    
    QString errorMsg;
    if (doc.saveToFile("person.xml", &errorMsg)) {
        qInfo() << "Saved successfully!";
    } else {
        qWarning() << "Error:" << errorMsg;
    }
    
    // Load back
    XsdQt::XmlDocument<MyApp::Person> doc2;
    if (doc2.loadFromFile("person.xml", &errorMsg)) {
        qInfo() << "Name:" 
                << doc2.root()->getFirstName() 
                << doc2.root()->getLastName();
        qInfo() << "Age:" << doc2.root()->getAge();
        qInfo() << "City:" 
                << doc2.root()->getAddress()->getCity();
    }
    
    return 0;
}
```

### 5. Build and Run

```bash
qmake myapp.pro
make
./myapp
```

Output:
```
Saved successfully!
Name: John Doe
Age: 30
City: Springfield
```

Generated `person.xml`:
```xml
<?xml version="1.0" encoding="UTF-8"?>
<person id="P001">
  <firstName>John</firstName>
  <lastName>Doe</lastName>
  <age>30</age>
  <email>john.doe@example.com</email>
  <address>
    <street>123 Main St</street>
    <city>Springfield</city>
    <zipCode>12345</zipCode>
  </address>
</person>
```

## Next Steps

### Working with Lists

XSD with `maxOccurs > 1`:

```xml
<xs:element name="phoneNumber" type="xs:string" 
            minOccurs="0" maxOccurs="unbounded"/>
```

Generated code provides list methods:

```cpp
person->addPhoneNumber("555-1234");
person->addPhoneNumber("555-5678");

const QList<QString>& phones = person->getPhoneNumbers();
for (const QString& phone : phones) {
    qDebug() << phone;
}
```

### Working with Inheritance

XSD with type extension:

```xml
<xs:complexType name="EmployeeType">
    <xs:complexContent>
        <xs:extension base="tns:PersonType">
            <xs:sequence>
                <xs:element name="employeeId" type="xs:string"/>
                <xs:element name="department" type="xs:string"/>
            </xs:sequence>
        </xs:extension>
    </xs:complexContent>
</xs:complexType>
```

Generated code uses inheritance:

```cpp
QSharedPointer<Employee> emp = QSharedPointer<Employee>::create();
emp->setFirstName("Jane");  // From Person base class
emp->setEmployeeId("E123"); // From Employee class
emp->setDepartment("IT");
```

### Working with Substitution Groups

XSD with substitution:

```xml
<xs:element name="person" type="tns:PersonType"/>
<xs:element name="employee" type="tns:EmployeeType" 
            substitutionGroup="tns:person"/>
```

Code can handle polymorphic lists:

```cpp
QList<QSharedPointer<Person>> people;

QSharedPointer<Person> p1 = QSharedPointer<Person>::create();
people.append(p1);

QSharedPointer<Employee> e1 = QSharedPointer<Employee>::create();
people.append(e1);  // Employee is-a Person

// Later, check concrete type
for (const auto& person : people) {
    auto employee = person.dynamicCast<Employee>();
    if (employee) {
        qDebug() << "Employee:" << employee->getEmployeeId();
    }
}
```

## Common Patterns

### Pattern 1: Configuration File

```cpp
// Load configuration
XsdQt::XmlDocument<Config> configDoc;
if (configDoc.loadFromFile("config.xml")) {
    applyConfig(configDoc.root());
}
```

### Pattern 2: Data Export/Import

```cpp
// Export data
XsdQt::XmlDocument<DataSet> exportDoc;
exportDoc.setRoot(collectData());
exportDoc.saveToFile("export.xml");

// Import data
XsdQt::XmlDocument<DataSet> importDoc;
if (importDoc.loadFromFile("import.xml")) {
    processData(importDoc.root());
}
```

### Pattern 3: API Request/Response

```cpp
// Create request
QSharedPointer<Request> req = createRequest();
XsdQt::XmlDocument<Request> reqDoc;
reqDoc.setRoot(req);
QString requestXml = reqDoc.saveToString();

// Send via HTTP/SOAP...
sendToServer(requestXml);

// Parse response
XsdQt::XmlDocument<Response> respDoc;
if (respDoc.loadFromString(responseXml)) {
    handleResponse(respDoc.root());
}
```

## Troubleshooting

### Issue: "Cannot open file: schema.xsd"

**Solution**: Use absolute path or ensure working directory is correct:
```bash
./xsd2cpp -i $(pwd)/schema.xsd -o generated/
```

### Issue: Generated code doesn't compile - "unknown type"

**Solution**: Ensure all includes are processed:
- Check that XSD includes/imports are resolved
- Verify the base path is correct for relative imports
- Make sure all referenced XSD files are accessible

### Issue: XML loading fails silently

**Solution**: Always check error messages:
```cpp
QString errorMsg;
if (!doc.loadFromFile("data.xml", &errorMsg)) {
    qCritical() << "Failed:" << errorMsg;
}
```

### Issue: Polymorphic types not deserializing correctly

**Solution**: Ensure types are registered. The generated code includes:
```cpp
static XsdQt::XmlTypeRegistrar<MyType> registrar_MyType("element", "TypeName");
```

This registration happens automatically when you link the generated .cpp files.

## Tips and Best Practices

1. **Keep XSD files organized**: Use folders for complex schemas
2. **Use meaningful namespaces**: Set `-n CompanyName::Module` for clarity
3. **Version your schemas**: Include version in namespace
4. **Test with real data**: Create sample XML files early
5. **Handle errors gracefully**: Always check return values
6. **Use smart pointers**: Let Qt manage memory automatically
7. **Document custom validations**: Override `fromXml()` with comments

## Advanced Topics

### Custom Validation

```cpp
bool Person::fromXml(QXmlStreamReader& reader) {
    if (!BaseClass::fromXml(reader)) {
        return false;
    }
    
    // Custom validation
    if (m_age < 0 || m_age > 150) {
        qWarning() << "Invalid age:" << m_age;
        return false;
    }
    
    if (!m_email.contains('@')) {
        qWarning() << "Invalid email format";
        return false;
    }
    
    return true;
}
```

### Performance Optimization

For large XML files:
- Use `QIODevice::ReadOnly | QIODevice::Unbuffered` for streaming
- Process elements incrementally instead of loading entire document
- Consider using `QXmlStreamReader` directly for read-only access

### Thread Safety

The generated classes are not thread-safe by default. For multi-threaded use:
- Use separate instances per thread
- Protect shared instances with `QMutex`
- Consider using `QSharedPointer` with atomic operations

## Resources

- Full API documentation: See header files
- Example schemas: `examples/` directory
- Unit tests: `tests/` directory for usage patterns
- Qt XML documentation: https://doc.qt.io/qt-5/qtxml-index.html

## Need Help?

Common solutions:
1. Check the README.md for detailed feature list
2. Look at test files for usage examples
3. Examine generated code to understand API
4. Review XSD specification for schema questions

Happy coding! 🚀
