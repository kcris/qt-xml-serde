#ifndef XSDPARSER_H
#define XSDPARSER_H

#include <QString>
#include <QMap>
#include <QList>
#include <QSharedPointer>
#include <QXmlStreamReader>

namespace XsdGen {

enum class XsdTypeKind {
    SimpleType,
    ComplexType,
    Element,
    Attribute
};

enum class ContentType {
    Empty,
    SimpleContent,
    ComplexContent,
    Mixed
};

struct XsdType;
struct XsdElement;
struct XsdAttribute;

struct XsdType {
    QString name;
    QString namespaceName;
    XsdTypeKind kind;
    ContentType contentType = ContentType::ComplexContent;
    
    // For simple types
    QString baseType;
    QList<QString> enumValues;
    QString pattern;
    int minLength = -1;
    int maxLength = -1;
    
    // For complex types
    QString baseTypeName;  // For extension/restriction
    bool isAbstract = false;
    QList<QSharedPointer<XsdElement>> elements;
    QList<QSharedPointer<XsdAttribute>> attributes;
    
    // Sequence, choice, all
    QString compositor; // "sequence", "choice", "all"
};

struct XsdElement {
    QString name;
    QString typeName;
    QString substitutionGroup;
    int minOccurs = 1;
    int maxOccurs = 1;  // -1 for unbounded
    QString defaultValue;
    bool isNillable = false;
    bool isAbstract = false;
    
    // Inline type definition
    QSharedPointer<XsdType> inlineType;
};

struct XsdAttribute {
    QString name;
    QString typeName;
    QString defaultValue;
    QString fixedValue;
    bool isRequired = false;
};

struct XsdSchema {
    QString targetNamespace;
    QString elementFormDefault;
    QString attributeFormDefault;
    
    QMap<QString, QSharedPointer<XsdType>> types;
    QMap<QString, QSharedPointer<XsdElement>> elements;
    QList<QString> imports;
    QList<QString> includes;
    
    // Substitution groups map: group head -> list of substitutable elements
    QMap<QString, QList<QString>> substitutionGroups;
};

class XsdParser {
public:
    XsdParser();
    
    /**
     * Parse XSD file and all its includes/imports
     */
    bool parseFile(const QString& filename, QString* errorMsg = nullptr);
    
    /**
     * Parse XSD from string
     */
    bool parseString(const QString& xsdContent, QString* errorMsg = nullptr);
    
    /**
     * Get the parsed schema
     */
    QSharedPointer<XsdSchema> schema() const { return m_schema; }
    
    /**
     * Resolve all includes and imports
     */
    bool resolveIncludes(const QString& basePath, QString* errorMsg = nullptr);
    
private:
    bool parseSchema(QXmlStreamReader& reader);
    bool parseElement(QXmlStreamReader& reader, QSharedPointer<XsdElement>& element);
    bool parseComplexType(QXmlStreamReader& reader, QSharedPointer<XsdType>& type);
    bool parseSimpleType(QXmlStreamReader& reader, QSharedPointer<XsdType>& type);
    bool parseSequence(QXmlStreamReader& reader, QSharedPointer<XsdType>& type);
    bool parseChoice(QXmlStreamReader& reader, QSharedPointer<XsdType>& type);
    bool parseAttribute(QXmlStreamReader& reader, QSharedPointer<XsdAttribute>& attribute);
    bool parseExtension(QXmlStreamReader& reader, QSharedPointer<XsdType>& type);
    bool parseRestriction(QXmlStreamReader& reader, QSharedPointer<XsdType>& type);
    
    void buildSubstitutionGroups();
    
    QString readAttribute(QXmlStreamReader& reader, const QString& name, const QString& defaultValue = QString());
    int readIntAttribute(QXmlStreamReader& reader, const QString& name, int defaultValue);
    bool readBoolAttribute(QXmlStreamReader& reader, const QString& name, bool defaultValue = false);
    
    void skipUnknownElement(QXmlStreamReader& reader);
    
    QSharedPointer<XsdSchema> m_schema;
    QString m_currentFile;
    QList<QString> m_parsedFiles;
};

} // namespace XsdGen

#endif // XSDPARSER_H
