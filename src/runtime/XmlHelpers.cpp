#include "XmlHelpers.h"

namespace XsdQt {

QString XmlHelpers::readElementText(QXmlStreamReader& reader) {
    return reader.readElementText();
}

int XmlHelpers::readInt(QXmlStreamReader& reader, bool* ok) {
    QString text = reader.readElementText();
    return text.toInt(ok);
}

double XmlHelpers::readDouble(QXmlStreamReader& reader, bool* ok) {
    QString text = reader.readElementText();
    return text.toDouble(ok);
}

bool XmlHelpers::readBool(QXmlStreamReader& reader, bool* ok) {
    QString text = reader.readElementText().toLower();
    if (ok) *ok = true;
    
    if (text == "true" || text == "1") {
        return true;
    } else if (text == "false" || text == "0") {
        return false;
    }
    
    if (ok) *ok = false;
    return false;
}

QDateTime XmlHelpers::readDateTime(QXmlStreamReader& reader, bool* ok) {
    QString text = reader.readElementText();
    QDateTime dt = QDateTime::fromString(text, Qt::ISODate);
    if (ok) *ok = dt.isValid();
    return dt;
}

QDate XmlHelpers::readDate(QXmlStreamReader& reader, bool* ok) {
    QString text = reader.readElementText();
    QDate date = QDate::fromString(text, Qt::ISODate);
    if (ok) *ok = date.isValid();
    return date;
}

QTime XmlHelpers::readTime(QXmlStreamReader& reader, bool* ok) {
    QString text = reader.readElementText();
    QTime time = QTime::fromString(text, Qt::ISODate);
    if (ok) *ok = time.isValid();
    return time;
}

void XmlHelpers::writeElement(QXmlStreamWriter& writer, const QString& name, const QString& value) {
    writer.writeTextElement(name, value);
}

void XmlHelpers::writeElement(QXmlStreamWriter& writer, const QString& name, int value) {
    writer.writeTextElement(name, QString::number(value));
}

void XmlHelpers::writeElement(QXmlStreamWriter& writer, const QString& name, double value) {
    writer.writeTextElement(name, QString::number(value, 'g', 15));
}

void XmlHelpers::writeElement(QXmlStreamWriter& writer, const QString& name, bool value) {
    writer.writeTextElement(name, value ? "true" : "false");
}

void XmlHelpers::writeElement(QXmlStreamWriter& writer, const QString& name, const QDateTime& value) {
    writer.writeTextElement(name, value.toString(Qt::ISODate));
}

void XmlHelpers::writeElement(QXmlStreamWriter& writer, const QString& name, const QDate& value) {
    writer.writeTextElement(name, value.toString(Qt::ISODate));
}

void XmlHelpers::writeElement(QXmlStreamWriter& writer, const QString& name, const QTime& value) {
    writer.writeTextElement(name, value.toString(Qt::ISODate));
}

QString XmlHelpers::readAttribute(QXmlStreamReader& reader, const QString& name, const QString& defaultValue) {
    QXmlStreamAttributes attrs = reader.attributes();
    if (attrs.hasAttribute(name)) {
        return attrs.value(name).toString();
    }
    return defaultValue;
}

int XmlHelpers::readIntAttribute(QXmlStreamReader& reader, const QString& name, int defaultValue, bool* ok) {
    QString value = readAttribute(reader, name);
    if (value.isEmpty()) {
        if (ok) *ok = false;
        return defaultValue;
    }
    return value.toInt(ok);
}

bool XmlHelpers::readBoolAttribute(QXmlStreamReader& reader, const QString& name, bool defaultValue, bool* ok) {
    QString value = readAttribute(reader, name).toLower();
    if (value.isEmpty()) {
        if (ok) *ok = false;
        return defaultValue;
    }
    
    if (ok) *ok = true;
    if (value == "true" || value == "1") {
        return true;
    } else if (value == "false" || value == "0") {
        return false;
    }
    
    if (ok) *ok = false;
    return defaultValue;
}

void XmlHelpers::writeAttribute(QXmlStreamWriter& writer, const QString& name, const QString& value) {
    writer.writeAttribute(name, value);
}

void XmlHelpers::writeAttribute(QXmlStreamWriter& writer, const QString& name, int value) {
    writer.writeAttribute(name, QString::number(value));
}

void XmlHelpers::writeAttribute(QXmlStreamWriter& writer, const QString& name, bool value) {
    writer.writeAttribute(name, value ? "true" : "false");
}

QSharedPointer<XmlSerializable> XmlHelpers::readPolymorphicElement(
    QXmlStreamReader& reader,
    const QString& expectedElement
) {
    if (!reader.isStartElement()) {
        return nullptr;
    }
    
    QString elementName = reader.name().toString();
    QString xsiType = getXsiType(reader);
    
    QSharedPointer<XmlSerializable> obj;
    
    // Try to create by xsi:type first
    if (!xsiType.isEmpty()) {
        obj = XmlTypeFactory::instance().createByType(xsiType);
    }
    
    // Fall back to element name
    if (!obj) {
        obj = XmlTypeFactory::instance().createByElement(elementName);
    }
    
    if (obj && obj->fromXml(reader)) {
        return obj;
    }
    
    return nullptr;
}

void XmlHelpers::writePolymorphicElement(
    QXmlStreamWriter& writer,
    const QSharedPointer<XmlSerializable>& obj,
    bool writeXsiType
) {
    if (!obj) {
        return;
    }
    
    writer.writeStartElement(obj->xmlElementName());
    
    if (writeXsiType) {
        writer.writeAttribute("xsi:type", obj->xsdTypeName());
    }
    
    obj->toXml(writer);
    writer.writeEndElement();
}

void XmlHelpers::skipCurrentElement(QXmlStreamReader& reader) {
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

QString XmlHelpers::getXsiType(QXmlStreamReader& reader) {
    QXmlStreamAttributes attrs = reader.attributes();
    
    // Check for xsi:type with namespace
    for (const QXmlStreamAttribute& attr : attrs) {
        if (attr.name().toString() == "type" && 
            attr.namespaceUri().toString().contains("XMLSchema-instance")) {
            return attr.value().toString();
        }
    }
    
    // Check without namespace prefix
    if (attrs.hasAttribute("xsi:type")) {
        return attrs.value("xsi:type").toString();
    }
    
    return QString();
}

void XmlHelpers::setupNamespaces(QXmlStreamWriter& writer) {
    writer.writeNamespace("http://www.w3.org/2001/XMLSchema-instance", "xsi");
}

} // namespace XsdQt
