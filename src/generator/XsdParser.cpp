#include "XsdParser.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>

namespace XsdGen {

XsdParser::XsdParser() : m_schema(QSharedPointer<XsdSchema>::create()) {
}

bool XsdParser::parseFile(const QString& filename, QString* errorMsg) {
    if (m_parsedFiles.contains(filename)) {
        return true; // Already parsed
    }
    
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (errorMsg) *errorMsg = QString("Cannot open file: %1").arg(filename);
        return false;
    }
    
    m_currentFile = filename;
    m_parsedFiles.append(filename);
    
    QXmlStreamReader reader(&file);
    bool result = parseSchema(reader);
    
    if (!result && errorMsg) {
        *errorMsg = QString("Parse error at line %1: %2")
            .arg(reader.lineNumber())
            .arg(reader.errorString());
    }
    
    return result;
}

bool XsdParser::parseString(const QString& xsdContent, QString* errorMsg) {
    QXmlStreamReader reader(xsdContent);
    bool result = parseSchema(reader);
    
    if (!result && errorMsg) {
        *errorMsg = QString("Parse error: %1").arg(reader.errorString());
    }
    
    return result;
}

bool XsdParser::parseSchema(QXmlStreamReader& reader) {
    while (!reader.atEnd()) {
        reader.readNext();
        
        if (reader.isStartElement()) {
            QString name = reader.name().toString();
            
            if (name == "schema") {
                m_schema->targetNamespace = readAttribute(reader, "targetNamespace");
                m_schema->elementFormDefault = readAttribute(reader, "elementFormDefault", "unqualified");
                m_schema->attributeFormDefault = readAttribute(reader, "attributeFormDefault", "unqualified");
            }
            else if (name == "element") {
                QSharedPointer<XsdElement> element(new XsdElement);
                if (parseElement(reader, element)) {
                    m_schema->elements[element->name] = element;
                }
            }
            else if (name == "complexType") {
                QSharedPointer<XsdType> type(new XsdType);
                type->kind = XsdTypeKind::ComplexType;
                if (parseComplexType(reader, type)) {
                    m_schema->types[type->name] = type;
                }
            }
            else if (name == "simpleType") {
                QSharedPointer<XsdType> type(new XsdType);
                type->kind = XsdTypeKind::SimpleType;
                if (parseSimpleType(reader, type)) {
                    m_schema->types[type->name] = type;
                }
            }
            else if (name == "include") {
                QString schemaLocation = readAttribute(reader, "schemaLocation");
                if (!schemaLocation.isEmpty()) {
                    m_schema->includes.append(schemaLocation);
                }
            }
            else if (name == "import") {
                QString schemaLocation = readAttribute(reader, "schemaLocation");
                if (!schemaLocation.isEmpty()) {
                    m_schema->imports.append(schemaLocation);
                }
            }
        }
    }
    
    buildSubstitutionGroups();
    return !reader.hasError();
}

bool XsdParser::parseElement(QXmlStreamReader& reader, QSharedPointer<XsdElement>& element) {
    element->name = readAttribute(reader, "name");
    element->typeName = readAttribute(reader, "type");
    element->substitutionGroup = readAttribute(reader, "substitutionGroup");
    element->minOccurs = readIntAttribute(reader, "minOccurs", 1);
    element->defaultValue = readAttribute(reader, "default");
    element->isNillable = readBoolAttribute(reader, "nillable", false);
    element->isAbstract = readBoolAttribute(reader, "abstract", false);
    
    QString maxOccursStr = readAttribute(reader, "maxOccurs", "1");
    if (maxOccursStr == "unbounded") {
        element->maxOccurs = -1;
    } else {
        element->maxOccurs = maxOccursStr.toInt();
    }
    
    // Check for inline type definition
    while (!reader.atEnd()) {
        reader.readNext();
        
        if (reader.isEndElement() && reader.name() == "element") {
            break;
        }
        
        if (reader.isStartElement()) {
            QString name = reader.name().toString();
            if (name == "complexType") {
                element->inlineType = QSharedPointer<XsdType>(new XsdType);
                element->inlineType->kind = XsdTypeKind::ComplexType;
                element->inlineType->name = element->name + "Type";
                parseComplexType(reader, element->inlineType);
            }
            else if (name == "simpleType") {
                element->inlineType = QSharedPointer<XsdType>(new XsdType);
                element->inlineType->kind = XsdTypeKind::SimpleType;
                element->inlineType->name = element->name + "Type";
                parseSimpleType(reader, element->inlineType);
            }
        }
    }
    
    return true;
}

bool XsdParser::parseComplexType(QXmlStreamReader& reader, QSharedPointer<XsdType>& type) {
    type->name = readAttribute(reader, "name");
    type->isAbstract = readBoolAttribute(reader, "abstract", false);
    
    while (!reader.atEnd()) {
        reader.readNext();
        
        if (reader.isEndElement() && reader.name() == "complexType") {
            break;
        }
        
        if (reader.isStartElement()) {
            QString name = reader.name().toString();
            
            if (name == "sequence") {
                type->compositor = "sequence";
                parseSequence(reader, type);
            }
            else if (name == "choice") {
                type->compositor = "choice";
                parseChoice(reader, type);
            }
            else if (name == "all") {
                type->compositor = "all";
                parseSequence(reader, type); // Same parsing logic
            }
            else if (name == "attribute") {
                QSharedPointer<XsdAttribute> attr(new XsdAttribute);
                if (parseAttribute(reader, attr)) {
                    type->attributes.append(attr);
                }
            }
            else if (name == "simpleContent") {
                type->contentType = ContentType::SimpleContent;
                // Continue parsing for extension/restriction
            }
            else if (name == "complexContent") {
                type->contentType = ContentType::ComplexContent;
                // Continue parsing for extension/restriction
            }
            else if (name == "extension") {
                parseExtension(reader, type);
            }
            else if (name == "restriction") {
                parseRestriction(reader, type);
            }
        }
    }
    
    return true;
}

bool XsdParser::parseSimpleType(QXmlStreamReader& reader, QSharedPointer<XsdType>& type) {
    type->name = readAttribute(reader, "name");
    
    while (!reader.atEnd()) {
        reader.readNext();
        
        if (reader.isEndElement() && reader.name() == "simpleType") {
            break;
        }
        
        if (reader.isStartElement()) {
            QString name = reader.name().toString();
            
            if (name == "restriction") {
                type->baseType = readAttribute(reader, "base");
                
                // Parse facets
                while (!reader.atEnd()) {
                    reader.readNext();
                    
                    if (reader.isEndElement() && reader.name() == "restriction") {
                        break;
                    }
                    
                    if (reader.isStartElement()) {
                        QString facetName = reader.name().toString();
                        QString value = readAttribute(reader, "value");
                        
                        if (facetName == "enumeration") {
                            type->enumValues.append(value);
                        }
                        else if (facetName == "pattern") {
                            type->pattern = value;
                        }
                        else if (facetName == "minLength") {
                            type->minLength = value.toInt();
                        }
                        else if (facetName == "maxLength") {
                            type->maxLength = value.toInt();
                        }
                    }
                }
            }
        }
    }
    
    return true;
}

bool XsdParser::parseSequence(QXmlStreamReader& reader, QSharedPointer<XsdType>& type) {
    while (!reader.atEnd()) {
        reader.readNext();
        
        if (reader.isEndElement() && (reader.name() == "sequence" || reader.name() == "all")) {
            break;
        }
        
        if (reader.isStartElement()) {
            QString name = reader.name().toString();
            
            if (name == "element") {
                QSharedPointer<XsdElement> element(new XsdElement);
                if (parseElement(reader, element)) {
                    type->elements.append(element);
                }
            }
        }
    }
    
    return true;
}

bool XsdParser::parseChoice(QXmlStreamReader& reader, QSharedPointer<XsdType>& type) {
    // Similar to sequence but marks it as choice
    return parseSequence(reader, type);
}

bool XsdParser::parseAttribute(QXmlStreamReader& reader, QSharedPointer<XsdAttribute>& attribute) {
    attribute->name = readAttribute(reader, "name");
    attribute->typeName = readAttribute(reader, "type");
    attribute->defaultValue = readAttribute(reader, "default");
    attribute->fixedValue = readAttribute(reader, "fixed");
    
    QString use = readAttribute(reader, "use", "optional");
    attribute->isRequired = (use == "required");
    
    skipUnknownElement(reader);
    return true;
}

bool XsdParser::parseExtension(QXmlStreamReader& reader, QSharedPointer<XsdType>& type) {
    type->baseTypeName = readAttribute(reader, "base");
    
    while (!reader.atEnd()) {
        reader.readNext();
        
        if (reader.isEndElement() && reader.name() == "extension") {
            break;
        }
        
        if (reader.isStartElement()) {
            QString name = reader.name().toString();
            
            if (name == "sequence") {
                parseSequence(reader, type);
            }
            else if (name == "choice") {
                parseChoice(reader, type);
            }
            else if (name == "attribute") {
                QSharedPointer<XsdAttribute> attr(new XsdAttribute);
                if (parseAttribute(reader, attr)) {
                    type->attributes.append(attr);
                }
            }
        }
    }
    
    return true;
}

bool XsdParser::parseRestriction(QXmlStreamReader& reader, QSharedPointer<XsdType>& type) {
    type->baseTypeName = readAttribute(reader, "base");
    
    // Parse restriction content (similar to extension)
    while (!reader.atEnd()) {
        reader.readNext();
        
        if (reader.isEndElement() && reader.name() == "restriction") {
            break;
        }
        
        if (reader.isStartElement()) {
            QString name = reader.name().toString();
            
            if (name == "sequence") {
                parseSequence(reader, type);
            }
            else if (name == "attribute") {
                QSharedPointer<XsdAttribute> attr(new XsdAttribute);
                if (parseAttribute(reader, attr)) {
                    type->attributes.append(attr);
                }
            }
        }
    }
    
    return true;
}

void XsdParser::buildSubstitutionGroups() {
    for (auto it = m_schema->elements.begin(); it != m_schema->elements.end(); ++it) {
        if (!it.value()->substitutionGroup.isEmpty()) {
            m_schema->substitutionGroups[it.value()->substitutionGroup].append(it.key());
        }
    }
}

bool XsdParser::resolveIncludes(const QString& basePath, QString* errorMsg) {
    QDir baseDir(basePath);
    
    for (const QString& include : m_schema->includes) {
        QString fullPath = baseDir.absoluteFilePath(include);
        if (!parseFile(fullPath, errorMsg)) {
            return false;
        }
    }
    
    for (const QString& import : m_schema->imports) {
        QString fullPath = baseDir.absoluteFilePath(import);
        if (!parseFile(fullPath, errorMsg)) {
            return false;
        }
    }
    
    return true;
}

QString XsdParser::readAttribute(QXmlStreamReader& reader, const QString& name, const QString& defaultValue) {
    QXmlStreamAttributes attrs = reader.attributes();
    if (attrs.hasAttribute(name)) {
        return attrs.value(name).toString();
    }
    return defaultValue;
}

int XsdParser::readIntAttribute(QXmlStreamReader& reader, const QString& name, int defaultValue) {
    QString value = readAttribute(reader, name);
    if (value.isEmpty()) {
        return defaultValue;
    }
    return value.toInt();
}

bool XsdParser::readBoolAttribute(QXmlStreamReader& reader, const QString& name, bool defaultValue) {
    QString value = readAttribute(reader, name).toLower();
    if (value.isEmpty()) {
        return defaultValue;
    }
    return (value == "true" || value == "1");
}

void XsdParser::skipUnknownElement(QXmlStreamReader& reader) {
    int depth = 1;
    while (depth > 0 && !reader.atEnd()) {
        reader.readNext();
        if (reader.isStartElement()) {
            depth++;
        } else if (reader.isEndElement()) {
            depth--;
        }
    }
}

} // namespace XsdGen
