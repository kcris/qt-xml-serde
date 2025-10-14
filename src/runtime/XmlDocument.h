#ifndef XMLDOCUMENT_H
#define XMLDOCUMENT_H

#include "XmlSerializable.h"
#include <QString>
#include <QSharedPointer>
#include <QFile>

namespace XsdQt {

/**
 * Template class for reading/writing complete XML documents
 * with a root element of type T
 */
template<typename T>
class XmlDocument {
public:
    XmlDocument() : m_root(QSharedPointer<T>::create()) {}
    explicit XmlDocument(const QSharedPointer<T>& root) : m_root(root) {}
    
    /**
     * Get the root element
     */
    QSharedPointer<T> root() const { return m_root; }
    
    /**
     * Set the root element
     */
    void setRoot(const QSharedPointer<T>& root) { m_root = root; }
    
    /**
     * Load XML from file
     */
    bool loadFromFile(const QString& filename, QString* errorMsg = nullptr) {
        QFile file(filename);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            if (errorMsg) *errorMsg = QString("Cannot open file: %1").arg(filename);
            return false;
        }
        
        return loadFromDevice(&file, errorMsg);
    }
    
    /**
     * Load XML from device
     */
    bool loadFromDevice(QIODevice* device, QString* errorMsg = nullptr) {
        QXmlStreamReader reader(device);
        
        // Find root element
        while (!reader.atEnd() && !reader.hasError()) {
            reader.readNext();
            
            if (reader.isStartElement()) {
                if (!m_root) {
                    m_root = QSharedPointer<T>::create();
                }
                
                if (m_root->fromXml(reader)) {
                    return true;
                } else {
                    if (errorMsg) *errorMsg = "Failed to parse root element";
                    return false;
                }
            }
        }
        
        if (reader.hasError()) {
            if (errorMsg) *errorMsg = reader.errorString();
            return false;
        }
        
        if (errorMsg) *errorMsg = "No root element found";
        return false;
    }
    
    /**
     * Load XML from string
     */
    bool loadFromString(const QString& xml, QString* errorMsg = nullptr) {
        QByteArray data = xml.toUtf8();
        QBuffer buffer(&data);
        buffer.open(QIODevice::ReadOnly);
        return loadFromDevice(&buffer, errorMsg);
    }
    
    /**
     * Save XML to file
     */
    bool saveToFile(const QString& filename, QString* errorMsg = nullptr) const {
        QFile file(filename);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            if (errorMsg) *errorMsg = QString("Cannot open file for writing: %1").arg(filename);
            return false;
        }
        
        return saveToDevice(&file, errorMsg);
    }
    
    /**
     * Save XML to device
     */
    bool saveToDevice(QIODevice* device, QString* errorMsg = nullptr) const {
        if (!m_root) {
            if (errorMsg) *errorMsg = "No root element to save";
            return false;
        }
        
        QXmlStreamWriter writer(device);
        writer.setAutoFormatting(true);
        writer.setAutoFormattingIndent(2);
        
        writer.writeStartDocument();
        XmlHelpers::setupNamespaces(writer);
        
        writer.writeStartElement(m_root->xmlElementName());
        m_root->toXml(writer);
        writer.writeEndElement();
        
        writer.writeEndDocument();
        
        if (writer.hasError()) {
            if (errorMsg) *errorMsg = "Error writing XML";
            return false;
        }
        
        return true;
    }
    
    /**
     * Save XML to string
     */
    QString saveToString(QString* errorMsg = nullptr) const {
        QByteArray data;
        QBuffer buffer(&data);
        buffer.open(QIODevice::WriteOnly);
        
        if (saveToDevice(&buffer, errorMsg)) {
            return QString::fromUtf8(data);
        }
        
        return QString();
    }
    
private:
    QSharedPointer<T> m_root;
};

} // namespace XsdQt

#endif // XMLDOCUMENT_H
