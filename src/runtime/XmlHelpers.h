#ifndef XMLHELPERS_H
#define XMLHELPERS_H

#include "XmlSerializable.h"
#include <QString>
#include <QDateTime>
#include <QDate>
#include <QTime>
#include <QList>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QSharedPointer>

namespace XsdQt {

class XmlHelpers {
public:
    // Read simple types
    static QString readElementText(QXmlStreamReader& reader);
    static int readInt(QXmlStreamReader& reader, bool* ok = nullptr);
    static double readDouble(QXmlStreamReader& reader, bool* ok = nullptr);
    static bool readBool(QXmlStreamReader& reader, bool* ok = nullptr);
    static QDateTime readDateTime(QXmlStreamReader& reader, bool* ok = nullptr);
    static QDate readDate(QXmlStreamReader& reader, bool* ok = nullptr);
    static QTime readTime(QXmlStreamReader& reader, bool* ok = nullptr);
    
    // Write simple types
    static void writeElement(QXmlStreamWriter& writer, const QString& name, const QString& value);
    static void writeElement(QXmlStreamWriter& writer, const QString& name, int value);
    static void writeElement(QXmlStreamWriter& writer, const QString& name, double value);
    static void writeElement(QXmlStreamWriter& writer, const QString& name, bool value);
    static void writeElement(QXmlStreamWriter& writer, const QString& name, const QDateTime& value);
    static void writeElement(QXmlStreamWriter& writer, const QString& name, const QDate& value);
    static void writeElement(QXmlStreamWriter& writer, const QString& name, const QTime& value);
    
    // Read attributes
    static QString readAttribute(QXmlStreamReader& reader, const QString& name, const QString& defaultValue = QString());
    static int readIntAttribute(QXmlStreamReader& reader, const QString& name, int defaultValue = 0, bool* ok = nullptr);
    static bool readBoolAttribute(QXmlStreamReader& reader, const QString& name, bool defaultValue = false, bool* ok = nullptr);
    
    // Write attributes
    static void writeAttribute(QXmlStreamWriter& writer, const QString& name, const QString& value);
    static void writeAttribute(QXmlStreamWriter& writer, const QString& name, int value);
    static void writeAttribute(QXmlStreamWriter& writer, const QString& name, bool value);
    
    // Handle polymorphic types
    static QSharedPointer<XmlSerializable> readPolymorphicElement(
        QXmlStreamReader& reader,
        const QString& expectedElement = QString()
    );
    
    static void writePolymorphicElement(
        QXmlStreamWriter& writer,
        const QSharedPointer<XmlSerializable>& obj,
        bool writeXsiType = false
    );
    
    // Skip unknown elements
    static void skipCurrentElement(QXmlStreamReader& reader);
    
    // Get xsi:type attribute
    static QString getXsiType(QXmlStreamReader& reader);
    
    // Namespace handling
    static void setupNamespaces(QXmlStreamWriter& writer);
};

} // namespace XsdQt

#endif // XMLHELPERS_H
