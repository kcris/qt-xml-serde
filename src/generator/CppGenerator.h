#ifndef CODEGENERATOR_H
#define CODEGENERATOR_H

#include "XsdParser.h"
#include <QString>
#include <QTextStream>

namespace XsdGen {

class CodeGenerator {
public:
    CodeGenerator(const QSharedPointer<XsdSchema>& schema);
    
    /**
     * Generate all code files to output directory
     */
    bool generate(const QString& outputDir, QString* errorMsg = nullptr);
    
    /**
     * Set namespace for generated code
     */
    void setNamespace(const QString& ns) { m_namespace = ns; }
    
private:
    QString toCppTypeName(const QString& xsdType);
    QString toCppClassName(const QString& name);
    QString toCppMemberName(const QString& name);
    QString getIncludeGuard(const QString& className);
    
    bool generateHeader(const QString& className, const QSharedPointer<XsdType>& type, const QString& outputDir);
    bool generateImplementation(const QString& className, const QSharedPointer<XsdType>& type, const QString& outputDir);
    
    void writeHeaderIncludes(QTextStream& out);
    void writeClassDeclaration(QTextStream& out, const QString& className, const QSharedPointer<XsdType>& type);
    void writeMemberVariables(QTextStream& out, const QSharedPointer<XsdType>& type);
    void writeGettersSetters(QTextStream& out, const QSharedPointer<XsdType>& type);
    void writeSerializationMethods(QTextStream& out, const QString& className);
    
    void writeImplementationIncludes(QTextStream& out, const QString& className);
    void writeConstructor(QTextStream& out, const QString& className, const QSharedPointer<XsdType>& type);
    void writeToXmlImplementation(QTextStream& out, const QString& className, const QSharedPointer<XsdType>& type);
    void writeFromXmlImplementation(QTextStream& out, const QString& className, const QSharedPointer<XsdType>& type);
    void writeRegistration(QTextStream& out, const QString& className, const QString& elementName, const QString& typeName);
    
    QString getBaseClassName(const QString& baseTypeName);
    QSharedPointer<XsdType> findType(const QString& typeName);
    
    QSharedPointer<XsdSchema> m_schema;
    QString m_namespace;
    QMap<QString, QString> m_typeMapping; // XSD type -> C++ type
};

} // namespace XsdGen

#endif // CODEGENERATOR_H
