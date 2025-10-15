/**
 * Complete examples of polymorphic XML handling
 * Demonstrates both type extension and element substitution
 */

#include <QCoreApplication>
#include <QDebug>
#include "XmlDocument.h"
#include "XmlHelpers.h"

// Mock classes (would be generated from vehicle.xsd)

//=============================================================================
// EXAMPLE 1: TYPE EXTENSION (inheritance-based polymorphism)
//=============================================================================

// Base type
class Vehicle : public XsdQt::XmlSerializable {
public:
    Vehicle() : m_year(0) {}
    virtual ~Vehicle() = default;
    
    QString getLicensePlate() const { return m_licensePlate; }
    void setLicensePlate(const QString& v) { m_licensePlate = v; }
    
    int getYear() const { return m_year; }
    void setYear(int v) { m_year = v; }
    
    QString getManufacturer() const { return m_manufacturer; }
    void setManufacturer(const QString& v) { m_manufacturer = v; }
    
    void toXml(QXmlStreamWriter& writer) const override {
        XsdQt::XmlHelpers::writeElement(writer, "licensePlate", m_licensePlate);
        XsdQt::XmlHelpers::writeElement(writer, "year", m_year);
        XsdQt::XmlHelpers::writeElement(writer, "manufacturer", m_manufacturer);
    }
    
    bool fromXml(QXmlStreamReader& reader) override {
        while (!reader.atEnd()) {
            reader.readNext();
            if (reader.isEndElement()) break;
            
            if (reader.isStartElement()) {
                QString name = reader.name().toString();
                if (name == "licensePlate") {
                    m_licensePlate = XsdQt::XmlHelpers::readElementText(reader);
                } else if (name == "year") {
                    m_year = XsdQt::XmlHelpers::readInt(reader);
                } else if (name == "manufacturer") {
                    m_manufacturer = XsdQt::XmlHelpers::readElementText(reader);
                } else {
                    XsdQt::XmlHelpers::skipCurrentElement(reader);
                }
            }
        }
        return true;
    }
    
    QString xmlElementName() const override { return "vehicle"; }
    QString xsdTypeName() const override { return "VehicleType"; }
    
protected:
    QString m_licensePlate;
    int m_year;
    QString m_manufacturer;
};

// Extended type: Car extends Vehicle
class Car : public Vehicle {
public:
    Car() : m_numDoors(0), m_trunkCapacity(0.0) {}
    
    int getNumDoors() const { return m_numDoors; }
    void setNumDoors(int v) { m_numDoors = v; }
    
    double getTrunkCapacity() const { return m_trunkCapacity; }
    void setTrunkCapacity(double v) { m_trunkCapacity = v; }
    
    // Override: call base class then add own fields
    void toXml(QXmlStreamWriter& writer) const override {
        Vehicle::toXml(writer);  // Serialize base class fields
        XsdQt::XmlHelpers::writeElement(writer, "numDoors", m_numDoors);
        XsdQt::XmlHelpers::writeElement(writer, "trunkCapacity", m_trunkCapacity);
    }
    
    bool fromXml(QXmlStreamReader& reader) override {
        while (!reader.atEnd()) {
            reader.readNext();
            if (reader.isEndElement()) break;
            
            if (reader.isStartElement()) {
                QString name = reader.name().toString();
                // Handle base class fields
                if (name == "licensePlate") {
                    m_licensePlate = XsdQt::XmlHelpers::readElementText(reader);
                } else if (name == "year") {
                    m_year = XsdQt::XmlHelpers::readInt(reader);
                } else if (name == "manufacturer") {
                    m_manufacturer = XsdQt::XmlHelpers::readElementText(reader);
                }
                // Handle extended fields
                else if (name == "numDoors") {
                    m_numDoors = XsdQt::XmlHelpers::readInt(reader);
                } else if (name == "trunkCapacity") {
                    m_trunkCapacity = XsdQt::XmlHelpers::readDouble(reader);
                } else {
                    XsdQt::XmlHelpers::skipCurrentElement(reader);
                }
            }
        }
        return true;
    }
    
    QString xmlElementName() const override { return "car"; }
    QString xsdTypeName() const override { return "CarType"; }
    
private:
    int m_numDoors;
    double m_trunkCapacity;
};

// Another extended type: Truck extends Vehicle
class Truck : public Vehicle {
public:
    Truck() : m_payloadCapacity(0.0), m_numAxles(0) {}
    
    double getPayloadCapacity() const { return m_payloadCapacity; }
    void setPayloadCapacity(double v) { m_payloadCapacity = v; }
    
    int getNumAxles() const { return m_numAxles; }
    void setNumAxles(int v) { m_numAxles = v; }
    
    void toXml(QXmlStreamWriter& writer) const override {
        Vehicle::toXml(writer);
        XsdQt::XmlHelpers::writeElement(writer, "payloadCapacity", m_payloadCapacity);
        XsdQt::XmlHelpers::writeElement(writer, "numAxles", m_numAxles);
    }
    
    bool fromXml(QXmlStreamReader& reader) override {
        while (!reader.atEnd()) {
            reader.readNext();
            if (reader.isEndElement()) break;
            
            if (reader.isStartElement()) {
                QString name = reader.name().toString();
                if (name == "licensePlate") {
                    m_licensePlate = XsdQt::XmlHelpers::readElementText(reader);
                } else if (name == "year") {
                    m_year = XsdQt::XmlHelpers::readInt(reader);
                } else if (name == "manufacturer") {
                    m_manufacturer = XsdQt::XmlHelpers::readElementText(reader);
                } else if (name == "payloadCapacity") {
                    m_payloadCapacity = XsdQt::XmlHelpers::readDouble(reader);
                } else if (name == "numAxles") {
                    m_numAxles = XsdQt::XmlHelpers::readInt(reader);
                } else {
                    XsdQt::XmlHelpers::skipCurrentElement(reader);
                }
            }
        }
        return true;
    }
    
    QString xmlElementName() const override { return "truck"; }
    QString xsdTypeName() const override { return "TruckType"; }
    
private:
    double m_payloadCapacity;
    int m_numAxles;
};

//=============================================================================
// EXAMPLE 2: ELEMENT SUBSTITUTION (substitution group)
//=============================================================================

// Container with polymorphic list
class Fleet : public XsdQt::XmlSerializable {
public:
    QString getName() const { return m_name; }
    void setName(const QString& v) { m_name = v; }
    
    const QList<QSharedPointer<Vehicle>>& getVehicles() const { return m_vehicles; }
    void addVehicle(const QSharedPointer<Vehicle>& v) { m_vehicles.append(v); }
    
    void toXml(QXmlStreamWriter& writer) const override {
        XsdQt::XmlHelpers::writeElement(writer, "name", m_name);
        
        // Write polymorphic elements - uses actual runtime type
        for (const auto& vehicle : m_vehicles) {
            if (vehicle) {
                // writePolymorphicElement uses vehicle->xmlElementName()
                // So it writes <car>, <truck>, or <vehicle> based on actual type
                XsdQt::XmlHelpers::writePolymorphicElement(writer, vehicle);
            }
        }
    }
    
    bool fromXml(QXmlStreamReader& reader) override {
        while (!reader.atEnd()) {
            reader.readNext();
            if (reader.isEndElement()) break;
            
            if (reader.isStartElement()) {
                QString name = reader.name().toString();
                
                if (name == "name") {
                    m_name = XsdQt::XmlHelpers::readElementText(reader);
                }
                // Handle ANY element in substitution group
                else if (name == "vehicle" || name == "car" || name == "truck") {
                    // readPolymorphicElement uses factory to create correct type
                    auto vehicle = XsdQt::XmlHelpers::readPolymorphicElement(reader);
                    if (vehicle) {
                        m_vehicles.append(vehicle.dynamicCast<Vehicle>());
                    }
                }
                else {
                    XsdQt::XmlHelpers::skipCurrentElement(reader);
                }
            }
        }
        return true;
    }
    
    QString xmlElementName() const override { return "fleet"; }
    QString xsdTypeName() const override { return "FleetType"; }
    
private:
    QString m_name;
    QList<QSharedPointer<Vehicle>> m_vehicles;
};

// Register all types with factory
static XsdQt::XmlTypeRegistrar<Vehicle> regVehicle("vehicle", "VehicleType");
static XsdQt::XmlTypeRegistrar<Car> regCar("car", "CarType");
static XsdQt::XmlTypeRegistrar<Truck> regTruck("truck", "TruckType");
static XsdQt::XmlTypeRegistrar<Fleet> regFleet("fleet", "FleetType");

//=============================================================================
// EXAMPLE USAGE
//=============================================================================

void exampleTypeExtension() {
    qInfo() << "\n=== EXAMPLE 1: Type Extension ===\n";
    
    // Create a Car (extends Vehicle)
    QSharedPointer<Car> car = QSharedPointer<Car>::create();
    car->setLicensePlate("ABC-123");
    car->setYear(2024);
    car->setManufacturer("CarCorp");
    car->setNumDoors(4);
    car->setTrunkCapacity(500.0);
    
    // Serialize
    XsdQt::XmlDocument<Car> doc;
    doc.setRoot(car);
    QString xml = doc.saveToString();
    
    qInfo() << "Generated XML:";
    qInfo() << xml;
    
    // Deserialize
    XsdQt::XmlDocument<Car> doc2;
    QString error;
    if (doc2.loadFromString(xml, &error)) {
        qInfo() << "\nDeserialized successfully:";
        qInfo() << "  License:" << doc2.root()->getLicensePlate();
        qInfo() << "  Year:" << doc2.root()->getYear();
        qInfo() << "  Doors:" << doc2.root()->getNumDoors();
        qInfo() << "  Trunk:" << doc2.root()->getTrunkCapacity() << "L";
    } else {
        qWarning() << "Error:" << error;
    }
}

void exampleElementSubstitution() {
    qInfo() << "\n=== EXAMPLE 2: Element Substitution ===\n";
    
    // Create fleet with mixed vehicle types
    QSharedPointer<Fleet> fleet = QSharedPointer<Fleet>::create();
    fleet->setName("Corporate Fleet");
    
    // Add base Vehicle
    QSharedPointer<Vehicle> v1 = QSharedPointer<Vehicle>::create();
    v1->setLicensePlate("V-001");
    v1->setYear(2020);
    v1->setManufacturer("GenericMotors");
    fleet->addVehicle(v1);
    
    // Add Car (substitutes for vehicle)
    QSharedPointer<Car> c1 = QSharedPointer<Car>::create();
    c1->setLicensePlate("C-001");
    c1->setYear(2023);
    c1->setManufacturer("LuxuryCars");
    c1->setNumDoors(2);
    c1->setTrunkCapacity(300.0);
    fleet->addVehicle(c1);
    
    // Add Truck (substitutes for vehicle)
    QSharedPointer<Truck> t1 = QSharedPointer<Truck>::create();
    t1->setLicensePlate("T-001");
    t1->setYear(2022);
    t1->setManufacturer("HeavyDuty");
    t1->setPayloadCapacity(10000.0);
    t1->setNumAxles(3);
    fleet->addVehicle(t1);
    
    // Serialize - notice different element names
    XsdQt::XmlDocument<Fleet> doc;
    doc.setRoot(fleet);
    QString xml = doc.saveToString();
    
    qInfo() << "Generated XML with substitution:";
    qInfo() << xml;
    
    // Deserialize - factory creates correct types
    XsdQt::XmlDocument<Fleet> doc2;
    QString error;
    if (doc2.loadFromString(xml, &error)) {
        qInfo() << "\nDeserialized fleet:";
        qInfo() << "  Name:" << doc2.root()->getName();
        qInfo() << "  Vehicles:" << doc2.root()->getVehicles().size();
        
        for (int i = 0; i < doc2.root()->getVehicles().size(); ++i) {
            auto v = doc2.root()->getVehicles()[i];
            qInfo() << "\n  Vehicle" << (i+1) << ":";
            qInfo() << "    License:" << v->getLicensePlate();
            qInfo() << "    Year:" << v->getYear();
            
            // Check actual runtime type
            if (auto car = v.dynamicCast<Car>()) {
                qInfo() << "    Type: Car";
                qInfo() << "    Doors:" << car->getNumDoors();
            } else if (auto truck = v.dynamicCast<Truck>()) {
                qInfo() << "    Type: Truck";
                qInfo() << "    Payload:" << truck->getPayloadCapacity() << "kg";
            } else {
                qInfo() << "    Type: Base Vehicle";
            }
        }
    } else {
        qWarning() << "Error:" << error;
    }
}

void examplePolymorphicUpcast() {
    qInfo() << "\n=== EXAMPLE 3: Polymorphic Storage ===\n";
    
    // Store different types in base class pointer
    QList<QSharedPointer<Vehicle>> vehicles;
    
    vehicles.append(QSharedPointer<Vehicle>::create());
    vehicles.append(QSharedPointer<Car>::create());
    vehicles.append(QSharedPointer<Truck>::create());
    
    // Initialize
    vehicles[0]->setLicensePlate("V-BASE");
    vehicles[0]->setYear(2020);
    
    auto car = vehicles[1].dynamicCast<Car>();
    car->setLicensePlate("C-EXTEND");
    car->setYear(2021);
    car->setNumDoors(4);
    
    auto truck = vehicles[2].dynamicCast<Truck>();
    truck->setLicensePlate("T-EXTEND");
    truck->setYear(2022);
    truck->setNumAxles(2);
    
    qInfo() << "Polymorphic list:";
    for (const auto& v : vehicles) {
        qInfo() << "  -" << v->getLicensePlate() 
                << "Type:" << v->xsdTypeName();
    }
}

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    
    qInfo() << "===================================";
    qInfo() << "Polymorphic XML Examples";
    qInfo() << "===================================";
    
    exampleTypeExtension();
    exampleElementSubstitution();
    examplePolymorphicUpcast();
    
    qInfo() << "\n===================================";
    qInfo() << "Examples complete!";
    qInfo() << "===================================\n";
    
    return 0;
}
