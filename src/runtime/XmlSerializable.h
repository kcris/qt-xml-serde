#ifndef XMLSERIALIZABLE_H
#define XMLSERIALIZABLE_H

#include <QString>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QSharedPointer>
#include <QMap>
#include <functional>

namespace XsdQt {

/**
 * Base class for all generated XML serializable types
 */
class XmlSerializable {
public:
    virtual ~XmlSerializable() = default;
    
    /**
     * Serialize this object to XML
     */
    virtual void toXml(QXmlStreamWriter& writer) const = 0;
    
    /**
     * Deserialize this object from XML
     * Returns true if successful
     */
    virtual bool fromXml(QXmlStreamReader& reader) = 0;
    
    /**
     * Get the XML element name for this type
     */
    virtual QString xmlElementName() const = 0;
    
    /**
     * Get the XSD type name (for xsi:type)
     */
    virtual QString xsdTypeName() const = 0;
};

/**
 * Factory for creating polymorphic types based on element name or xsi:type
 */
class XmlTypeFactory {
public:
    using Creator = std::function<QSharedPointer<XmlSerializable>()>;
    
    static XmlTypeFactory& instance() {
        static XmlTypeFactory factory;
        return factory;
    }
    
    /**
     * Register a type creator
     * @param elementName The XML element name
     * @param typeName The XSD type name
     * @param creator Factory function to create instances
     */
    void registerType(const QString& elementName, const QString& typeName, Creator creator) {
        m_elementCreators[elementName] = creator;
        m_typeCreators[typeName] = creator;
        m_elementToType[elementName] = typeName;
    }
    
    /**
     * Create instance by element name (for substitution groups)
     */
    QSharedPointer<XmlSerializable> createByElement(const QString& elementName) const {
        auto it = m_elementCreators.find(elementName);
        if (it != m_elementCreators.end()) {
            return it.value()();
        }
        return nullptr;
    }
    
    /**
     * Create instance by type name (for xsi:type)
     */
    QSharedPointer<XmlSerializable> createByType(const QString& typeName) const {
        auto it = m_typeCreators.find(typeName);
        if (it != m_typeCreators.end()) {
            return it.value()();
        }
        return nullptr;
    }
    
    /**
     * Get type name for element name
     */
    QString getTypeForElement(const QString& elementName) const {
        return m_elementToType.value(elementName);
    }
    
private:
    XmlTypeFactory() = default;
    
    QMap<QString, Creator> m_elementCreators;
    QMap<QString, Creator> m_typeCreators;
    QMap<QString, QString> m_elementToType;
};

/**
 * Helper for automatic type registration
 */
template<typename T>
class XmlTypeRegistrar {
public:
    XmlTypeRegistrar(const QString& elementName, const QString& typeName) {
        XmlTypeFactory::instance().registerType(
            elementName, 
            typeName,
            []() -> QSharedPointer<XmlSerializable> {
                return QSharedPointer<T>::create();
            }
        );
    }
};

} // namespace XsdQt

#endif // XMLSERIALIZABLE_H
