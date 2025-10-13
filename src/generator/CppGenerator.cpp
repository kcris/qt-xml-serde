#include "CodeGenerator.h"
#include <QFile>
#include <QDir>
#include <QTextStream>

namespace XsdGen {

CodeGenerator::CodeGenerator(const QSharedPointer<XsdSchema>& schema)
    : m_schema(schema), m_namespace("Generated")
{
    // Initialize XSD to C++ type mapping
    m_typeMapping["xs:string"] = "QString";
    m_typeMapping["xs:int"] = "int";
    m_typeMapping["xs:integer"] = "int";
    m_typeMapping["xs:long"] = "qint64";
    m_typeMapping["xs:short"] = "qint16";
    m_typeMapping["xs:byte"] = "qint8";
    m_typeMapping["xs:unsignedInt"] = "quint32";
    m_typeMapping["xs:unsignedLong"] = "quint64";
    m_typeMapping["xs:unsignedShort"] = "quint16";
    m_typeMapping["xs:unsignedByte"] = "quint8";
    m_typeMapping["xs:double"] = "double";
    m_typeMapping["xs:float"] = "float";
    m_typeMapping["xs:boolean"] = "bool";
    m_typeMapping["xs:dateTime"] = "QDateTime";
    m_typeMapping["xs:date"] = "QDate";
    m_typeMapping["xs:time"] = "QTime";
    m_typeMapping["xs:decimal"] = "double";
    
    // Also support without xs: prefix
    m_typeMapping["string"] = "QString";
    m_typeMapping["int"] = "int";
    m_typeMapping["integer"] = "int";
    m_typeMapping["long"] = "qint64";
    m_typeMapping["boolean"] = "bool";
    m_typeMapping["dateTime"] = "QDateTime";
    m_typeMapping["date"] = "QDate";
    m_typeMapping["time"] = "QTime";
    m_typeMapping["double"] = "double";
    m_typeMapping["float"] = "float";
}

bool CodeGenerator::generate(const QString& outputDir, QString* errorMsg) {
    QDir dir;
    if (!dir.exists(outputDir)) {
        dir.mkpath(outputDir);
    }
    
    // Generate code for each complex type
    for (auto it = m_schema->types.begin(); it != m_schema->types.end(); ++it) {
        if (it.value()->kind == XsdTypeKind::ComplexType) {
            QString className = toCppClassName(it.key());
            
            if (!generateHeader(className, it.value(), outputDir)) {
                if (errorMsg) *errorMsg = QString("Failed to generate header for %1").arg(className);
                return false;
            }
            
            if (!generateImplementation(className, it.value(), outputDir)) {
                if (errorMsg) *errorMsg = QString("Failed to generate implementation for %1").arg(className);
                return false;
            }
        }
    }
    
    // Generate code for global elements with inline types
    for (auto it = m_schema->elements.begin(); it != m_schema->elements.end(); ++it) {
        if (it.value()->inlineType) {
            QString className = toCppClassName(it.key());
            
            if (!generateHeader(className, it.value()->inlineType, outputDir)) {
                if (errorMsg) *errorMsg = QString("Failed to generate header for element %1").arg(className);
                return false;
            }
            
            if (!generateImplementation(className, it.value()->inlineType, outputDir)) {
                if (errorMsg) *errorMsg = QString("Failed to generate implementation for element %1").arg(className);
                return false;
            }
        }
    }
    
    return true;
}

QString CodeGenerator::toCppTypeName(const QString& xsdType) {
    // Remove namespace prefix if present
    QString cleanType = xsdType;
    int colonPos = xsdType.indexOf(':');
    if (colonPos >= 0) {
        cleanType = xsdType.mid(colonPos + 1);
    }
    
    // Check if it's a built-in type
    if (m_typeMapping.contains(xsdType)) {
        return m_typeMapping[xsdType];
    }
    if (m_typeMapping.contains(cleanType)) {
        return m_typeMapping[cleanType];
    }
    
    // Otherwise it's a user-defined type
    return toCppClassName(cleanType);
}

QString CodeGenerator::toCppClassName(const QString& name) {
    QString result = name;
    
    // Capitalize first letter
    if (!result.isEmpty()) {
        result[0] = result[0].toUpper();
    }
    
    // Remove hyphens and underscores, capitalize following letter
    for (int i = 0; i < result.length(); ++i) {
        if (result[i] == '-' || result[i] == '_') {
            result.remove(i, 1);
            if (i < result.length()) {
                result[i] = result[i].toUpper();
            }
        }
    }
    
    return result;
}

QString CodeGenerator::toCppMemberName(const QString& name) {
    QString result = name;
    
    // Start with lowercase
    if (!result.isEmpty()) {
        result[0] = result[0].toLower();
    }
    
    // Remove hyphens, capitalize following letter
    for (int i = 0; i < result.length(); ++i) {
        if (result[i] == '-') {
            result.remove(i, 1);
            if (i < result.length()) {
                result[i] = result[i].toUpper();
            }
        }
    }
    
    return "m_" + result;
}

QString CodeGenerator::getIncludeGuard(const QString& className) {
    return className.toUpper() + "_H";
}

bool CodeGenerator::generateHeader(const QString& className, const QSharedPointer<XsdType>& type, const QString& outputDir) {
    QString filename = outputDir + "/" + className + ".h";
    QFile file(filename);
    
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream out(&file);
    
    QString guard = getIncludeGuard(className);
    out << "#ifndef " << guard << "\n";
    out << "#define " << guard << "\n\n";
    
    writeHeaderIncludes(out);
    
    out << "namespace " << m_namespace << " {\n\n";
    
    writeClassDeclaration(out, className, type);
    
    out << "} // namespace " << m_namespace << "\n\n";
    out << "#endif // " << guard << "\n";
    
    return true;
}

void CodeGenerator::writeHeaderIncludes(QTextStream& out) {
    out << "#include \"XmlSerializable.h\"\n";
    out << "#include \"XmlHelpers.h\"\n";
    out << "#include <QString>\n";
    out << "#include <QDateTime>\n";
    out << "#include <QList>\n";
    out << "#include <QSharedPointer>\n\n";
}

void CodeGenerator::writeClassDeclaration(QTextStream& out, const QString& className, const QSharedPointer<XsdType>& type) {
    // Forward declarations for cyclic dependencies
    out << "// Forward declarations\n";
    for (const auto& elem : type->elements) {
        QString elemType = toCppTypeName(elem->typeName);
        if (!m_typeMapping.contains(elem->typeName) && !elem->typeName.isEmpty()) {
            out << "class " << elemType << ";\n";
        }
    }
    out << "\n";
    
    // Class declaration
    QString baseClass = "XsdQt::XmlSerializable";
    if (!type->baseTypeName.isEmpty()) {
        baseClass = getBaseClassName(type->baseTypeName);
    }
    
    out << "class " << className << " : public " << baseClass << " {\n";
    out << "public:\n";
    out << "    " << className << "();\n";
    
    if (!type->baseTypeName.isEmpty()) {
        out << "    virtual ~" << className << "() = default;\n\n";
    }
    
    writeGettersSetters(out, type);
    writeSerializationMethods(out, className);
    
    out << "\nprivate:\n";
    writeMemberVariables(out, type);
    
    out << "};\n\n";
}

void CodeGenerator::writeMemberVariables(QTextStream& out, const QSharedPointer<XsdType>& type) {
    // Write member variables for elements
    for (const auto& elem : type->elements) {
        QString cppType = toCppTypeName(elem->typeName);
        QString memberName = toCppMemberName(elem->name);
        
        if (elem->maxOccurs == -1 || elem->maxOccurs > 1) {
            // List/array type
            if (m_typeMapping.contains(elem->typeName)) {
                out << "    QList<" << cppType << "> " << memberName << ";\n";
            } else {
                out << "    QList<QSharedPointer<" << cppType << ">> " << memberName << ";\n";
            }
        } else {
            // Single value
            if (m_typeMapping.contains(elem->typeName)) {
                out << "    " << cppType << " " << memberName << ";\n";
            } else {
                out << "    QSharedPointer<" << cppType << "> " << memberName << ";\n";
            }
        }
    }
    
    // Write member variables for attributes
    for (const auto& attr : type->attributes) {
        QString cppType = toCppTypeName(attr->typeName);
        QString memberName = toCppMemberName(attr->name);
        out << "    " << cppType << " " << memberName << ";\n";
    }
}

void CodeGenerator::writeGettersSetters(QTextStream& out, const QSharedPointer<XsdType>& type) {
    // Getters and setters for elements
    for (const auto& elem : type->elements) {
        QString cppType = toCppTypeName(elem->typeName);
        QString memberName = toCppMemberName(elem->name);
        QString propertyName = elem->name;
        propertyName[0] = propertyName[0].toUpper();
        
        if (elem->maxOccurs == -1 || elem->maxOccurs > 1) {
            // List type
            if (m_typeMapping.contains(elem->typeName)) {
                out << "    const QList<" << cppType << ">& get" << propertyName << "() const { return " << memberName << "; }\n";
                out << "    void set" << propertyName << "(const QList<" << cppType << ">& value) { " << memberName << " = value; }\n";
                out << "    void add" << propertyName << "(const " << cppType << "& value) { " << memberName << ".append(value); }\n";
            } else {
                out << "    const QList<QSharedPointer<" << cppType << ">>& get" << propertyName << "() const { return " << memberName << "; }\n";
                out << "    void set" << propertyName << "(const QList<QSharedPointer<" << cppType << ">>& value) { " << memberName << " = value; }\n";
                out << "    void add" << propertyName << "(const QSharedPointer<" << cppType << ">& value) { " << memberName << ".append(value); }\n";
            }
        } else {
            // Single value
            if (m_typeMapping.contains(elem->typeName)) {
                out << "    " << cppType << " get" << propertyName << "() const { return " << memberName << "; }\n";
                out << "    void set" << propertyName << "(const " << cppType << "& value) { " << memberName << " = value; }\n";
            } else {
                out << "    QSharedPointer<" << cppType << "> get" << propertyName << "() const { return " << memberName << "; }\n";
                out << "    void set" << propertyName << "(const QSharedPointer<" << cppType << ">& value) { " << memberName << " = value; }\n";
            }
        }
        out << "\n";
    }
    
    // Getters and setters for attributes
    for (const auto& attr : type->attributes) {
        QString cppType = toCppTypeName(attr->typeName);
        QString memberName = toCppMemberName(attr->name);
        QString propertyName = attr->name;
        propertyName[0] = propertyName[0].toUpper();
        
        out << "    " << cppType << " get" << propertyName << "() const { return " << memberName << "; }\n";
        out << "    void set" << propertyName << "(const " << cppType << "& value) { " << memberName << " = value; }\n\n";
    }
}

void CodeGenerator::writeSerializationMethods(QTextStream& out, const QString& className) {
    out << "    // Serialization\n";
    out << "    void toXml(QXmlStreamWriter& writer) const override;\n";
    out << "    bool fromXml(QXmlStreamReader& reader) override;\n";
    out << "    QString xmlElementName() const override;\n";
    out << "    QString xsdTypeName() const override;\n";
}

QString CodeGenerator::getBaseClassName(const QString& baseTypeName) {
    return toCppClassName(baseTypeName);
}

QSharedPointer<XsdType> CodeGenerator::findType(const QString& typeName) {
    return m_schema->types.value(typeName);
}

bool CodeGenerator::generateImplementation(const QString& className, const QSharedPointer<XsdType>& type, const QString& outputDir) {
    QString filename = outputDir + "/" + className + ".cpp";
    QFile file(filename);
    
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream out(&file);
    
    writeImplementationIncludes(out, className);
    
    out << "namespace " << m_namespace << " {\n\n";
    
    writeConstructor(out, className, type);
    writeToXmlImplementation(out, className, type);
    writeFromXmlImplementation(out, className, type);
    
    // Find element name for this type
    QString elementName = className;
    elementName[0] = elementName[0].toLower();
    for (auto it = m_schema->elements.begin(); it != m_schema->elements.end(); ++it) {
        if (toCppClassName(it.key()) == className) {
            elementName = it.key();
            break;
        }
    }
    
    writeRegistration(out, className, elementName, type->name);
    
    out << "} // namespace " << m_namespace << "\n";
    
    return true;
}

void CodeGenerator::writeImplementationIncludes(QTextStream& out, const QString& className) {
    out << "#include \"" << className << ".h\"\n\n";
}

void CodeGenerator::writeConstructor(QTextStream& out, const QString& className, const QSharedPointer<XsdType>& type) {
    out << className << "::" << className << "() {\n";
    
    // Initialize simple type members with defaults
    for (const auto& elem : type->elements) {
        if (m_typeMapping.contains(elem->typeName) && !elem->defaultValue.isEmpty()) {
            QString memberName = toCppMemberName(elem->name);
            QString cppType = toCppTypeName(elem->typeName);
            
            if (cppType == "QString") {
                out << "    " << memberName << " = \"" << elem->defaultValue << "\";\n";
            } else if (cppType == "int" || cppType.contains("int")) {
                out << "    " << memberName << " = " << elem->defaultValue << ";\n";
            } else if (cppType == "bool") {
                out << "    " << memberName << " = " << (elem->defaultValue == "true" ? "true" : "false") << ";\n";
            }
        }
    }
    
    out << "}\n\n";
}

void CodeGenerator::writeToXmlImplementation(QTextStream& out, const QString& className, const QSharedPointer<XsdType>& type) {
    out << "void " << className << "::toXml(QXmlStreamWriter& writer) const {\n";
    
    // Call base class if there's inheritance
    if (!type->baseTypeName.isEmpty()) {
        QString baseClass = getBaseClassName(type->baseTypeName);
        out << "    " << baseClass << "::toXml(writer);\n\n";
    }
    
    // Write attributes
    for (const auto& attr : type->attributes) {
        QString memberName = toCppMemberName(attr->name);
        out << "    XsdQt::XmlHelpers::writeAttribute(writer, \"" << attr->name << "\", " << memberName << ");\n";
    }
    
    if (!type->attributes.isEmpty()) {
        out << "\n";
    }
    
    // Write elements
    for (const auto& elem : type->elements) {
        QString memberName = toCppMemberName(elem->name);
        QString cppType = toCppTypeName(elem->typeName);
        
        if (elem->maxOccurs == -1 || elem->maxOccurs > 1) {
            // List type
            out << "    for (const auto& item : " << memberName << ") {\n";
            if (m_typeMapping.contains(elem->typeName)) {
                out << "        XsdQt::XmlHelpers::writeElement(writer, \"" << elem->name << "\", item);\n";
            } else {
                out << "        if (item) {\n";
                out << "            XsdQt::XmlHelpers::writePolymorphicElement(writer, item);\n";
                out << "        }\n";
            }
            out << "    }\n";
        } else {
            // Single value
            if (m_typeMapping.contains(elem->typeName)) {
                out << "    XsdQt::XmlHelpers::writeElement(writer, \"" << elem->name << "\", " << memberName << ");\n";
            } else {
                out << "    if (" << memberName << ") {\n";
                out << "        XsdQt::XmlHelpers::writePolymorphicElement(writer, " << memberName << ");\n";
                out << "    }\n";
            }
        }
    }
    
    out << "}\n\n";
}

void CodeGenerator::writeFromXmlImplementation(QTextStream& out, const QString& className, const QSharedPointer<XsdType>& type) {
    out << "bool " << className << "::fromXml(QXmlStreamReader& reader) {\n";
    
    // Read attributes first
    if (!type->attributes.isEmpty()) {
        out << "    // Read attributes\n";
        for (const auto& attr : type->attributes) {
            QString memberName = toCppMemberName(attr->name);
            QString cppType = toCppTypeName(attr->typeName);
            
            if (cppType == "QString") {
                out << "    " << memberName << " = XsdQt::XmlHelpers::readAttribute(reader, \"" << attr->name << "\"";
                if (!attr->defaultValue.isEmpty()) {
                    out << ", \"" << attr->defaultValue << "\"";
                }
                out << ");\n";
            } else if (cppType == "int" || cppType.contains("int")) {
                out << "    " << memberName << " = XsdQt::XmlHelpers::readIntAttribute(reader, \"" << attr->name << "\"";
                if (!attr->defaultValue.isEmpty()) {
                    out << ", " << attr->defaultValue;
                }
                out << ");\n";
            } else if (cppType == "bool") {
                out << "    " << memberName << " = XsdQt::XmlHelpers::readBoolAttribute(reader, \"" << attr->name << "\"";
                if (!attr->defaultValue.isEmpty()) {
                    out << ", " << (attr->defaultValue == "true" ? "true" : "false");
                }
                out << ");\n";
            }
        }
        out << "\n";
    }
    
    // Read child elements
    out << "    // Read child elements\n";
    out << "    while (!reader.atEnd()) {\n";
    out << "        reader.readNext();\n\n";
    out << "        if (reader.isEndElement()) {\n";
    out << "            break;\n";
    out << "        }\n\n";
    out << "        if (reader.isStartElement()) {\n";
    out << "            QString name = reader.name().toString();\n\n";
    
    // Generate if-else chain for each element
    bool first = true;
    for (const auto& elem : type->elements) {
        QString memberName = toCppMemberName(elem->name);
        QString cppType = toCppTypeName(elem->typeName);
        
        out << "            ";
        if (!first) out << "else ";
        out << "if (name == \"" << elem->name << "\") {\n";
        
        if (elem->maxOccurs == -1 || elem->maxOccurs > 1) {
            // List type
            if (m_typeMapping.contains(elem->typeName)) {
                if (cppType == "QString") {
                    out << "                " << memberName << ".append(XsdQt::XmlHelpers::readElementText(reader));\n";
                } else if (cppType == "int" || cppType.contains("int")) {
                    out << "                " << memberName << ".append(XsdQt::XmlHelpers::readInt(reader));\n";
                } else if (cppType == "double" || cppType == "float") {
                    out << "                " << memberName << ".append(XsdQt::XmlHelpers::readDouble(reader));\n";
                } else if (cppType == "bool") {
                    out << "                " << memberName << ".append(XsdQt::XmlHelpers::readBool(reader));\n";
                } else if (cppType == "QDateTime") {
                    out << "                " << memberName << ".append(XsdQt::XmlHelpers::readDateTime(reader));\n";
                } else if (cppType == "QDate") {
                    out << "                " << memberName << ".append(XsdQt::XmlHelpers::readDate(reader));\n";
                } else if (cppType == "QTime") {
                    out << "                " << memberName << ".append(XsdQt::XmlHelpers::readTime(reader));\n";
                }
            } else {
                out << "                auto item = XsdQt::XmlHelpers::readPolymorphicElement(reader, \"" << elem->name << "\");\n";
                out << "                if (item) {\n";
                out << "                    " << memberName << ".append(item.dynamicCast<" << cppType << ">());\n";
                out << "                }\n";
            }
        } else {
            // Single value
            if (m_typeMapping.contains(elem->typeName)) {
                if (cppType == "QString") {
                    out << "                " << memberName << " = XsdQt::XmlHelpers::readElementText(reader);\n";
                } else if (cppType == "int" || cppType.contains("int")) {
                    out << "                " << memberName << " = XsdQt::XmlHelpers::readInt(reader);\n";
                } else if (cppType == "double" || cppType == "float") {
                    out << "                " << memberName << " = XsdQt::XmlHelpers::readDouble(reader);\n";
                } else if (cppType == "bool") {
                    out << "                " << memberName << " = XsdQt::XmlHelpers::readBool(reader);\n";
                } else if (cppType == "QDateTime") {
                    out << "                " << memberName << " = XsdQt::XmlHelpers::readDateTime(reader);\n";
                } else if (cppType == "QDate") {
                    out << "                " << memberName << " = XsdQt::XmlHelpers::readDate(reader);\n";
                } else if (cppType == "QTime") {
                    out << "                " << memberName << " = XsdQt::XmlHelpers::readTime(reader);\n";
                }
            } else {
                out << "                " << memberName << " = XsdQt::XmlHelpers::readPolymorphicElement(reader, \"" << elem->name << "\").dynamicCast<" << cppType << ">();\n";
            }
        }
        
        out << "            }\n";
        first = false;
    }
    
    if (!type->elements.isEmpty()) {
        out << "            else {\n";
        out << "                XsdQt::XmlHelpers::skipCurrentElement(reader);\n";
        out << "            }\n";
    }
    
    out << "        }\n";
    out << "    }\n\n";
    out << "    return true;\n";
    out << "}\n\n";
}

void CodeGenerator::writeRegistration(QTextStream& out, const QString& className, const QString& elementName, const QString& typeName) {
    out << "QString " << className << "::xmlElementName() const {\n";
    out << "    return \"" << elementName << "\";\n";
    out << "}\n\n";
    
    out << "QString " << className << "::xsdTypeName() const {\n";
    out << "    return \"" << typeName << "\";\n";
    out << "}\n\n";
    
    out << "// Static registration\n";
    out << "static XsdQt::XmlTypeRegistrar<" << className << "> registrar_" << className << "(\"" << elementName << "\", \"" << typeName << "\");\n\n";
}

} // namespace XsdGen
