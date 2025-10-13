#include "XsdParser.h"
#include "CodeGenerator.h"
#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QFileInfo>

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("xsd2cpp");
    QCoreApplication::setApplicationVersion("1.0");
    
    QCommandLineParser parser;
    parser.setApplicationDescription("XSD to C++ code generator for Qt5");
    parser.addHelpOption();
    parser.addVersionOption();
    
    parser.addPositionalArgument("input", "Input XSD file");
    
    QCommandLineOption outputOption(QStringList() << "o" << "output",
        "Output directory for generated files (default: current directory)",
        "directory",
        ".");
    parser.addOption(outputOption);
    
    QCommandLineOption namespaceOption(QStringList() << "n" << "namespace",
        "C++ namespace for generated code (default: Generated)",
        "namespace",
        "Generated");
    parser.addOption(namespaceOption);
    
    parser.process(app);
    
    const QStringList args = parser.positionalArguments();
    if (args.isEmpty()) {
        qCritical() << "Error: No input file specified";
        parser.showHelp(1);
        return 1;
    }
    
    QString inputFile = args.at(0);
    QString outputDir = parser.value(outputOption);
    QString ns = parser.value(namespaceOption);
    
    qInfo() << "Parsing XSD file:" << inputFile;
    
    // Parse XSD
    XsdGen::XsdParser xsdParser;
    QString errorMsg;
    
    if (!xsdParser.parseFile(inputFile, &errorMsg)) {
        qCritical() << "Failed to parse XSD:" << errorMsg;
        return 1;
    }
    
    qInfo() << "Resolving includes and imports...";
    
    QFileInfo fileInfo(inputFile);
    QString basePath = fileInfo.absolutePath();
    
    if (!xsdParser.resolveIncludes(basePath, &errorMsg)) {
        qCritical() << "Failed to resolve includes:" << errorMsg;
        return 1;
    }
    
    qInfo() << "Generating C++ code...";
    
    // Generate code
    XsdGen::CodeGenerator generator(xsdParser.schema());
    generator.setNamespace(ns);
    
    if (!generator.generate(outputDir, &errorMsg)) {
        qCritical() << "Failed to generate code:" << errorMsg;
        return 1;
    }
    
    qInfo() << "Code generation completed successfully!";
    qInfo() << "Output directory:" << outputDir;
    
    return 0;
}
