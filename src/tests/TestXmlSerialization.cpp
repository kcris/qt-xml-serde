#include <QtTest>
#include <QDebug>
#include "XmlDocument.h"
#include "XmlHelpers.h"

// Mock generated classes for testing
class Vehicle : public XsdQt::XmlSerializable {
public:
    Vehicle() : m_year(0) {}
    virtual ~Vehicle() = default;
    
    QString getLicensePlate() const { return m_licensePlate; }
    void setLicensePlate(const QString& value) { m_licensePlate = value; }
    
    int getYear() const { return m_year; }
    void setYear(int value) { m_year = value; }
    
    QString getManufacturer() const { return m_manufacturer; }
    void setManufacturer(const QString& value) { m_manufacturer = value; }
    
    QString getId() const { return m_id; }
    void setId(const QString& value) { m_id = value; }
    
    void toXml(QXmlStreamWriter& writer) const override {
        XsdQt::XmlHelpers::writeAttribute(writer, "id", m_id);
        XsdQt::XmlHelpers::writeElement(writer, "licensePlate", m_licensePlate);
        XsdQt::XmlHelpers::writeElement(writer, "year", m_year);
        XsdQt::XmlHelpers::writeElement(writer, "manufacturer", m_manufacturer);
    }
    
    bool fromXml(QXmlStreamReader& reader) override {
        m_id = XsdQt::XmlHelpers::readAttribute(reader, "id");
        
        while (!reader.atEnd()) {
            reader.readNext();
            
            if (reader.isEndElement()) {
                break;
            }
            
            if (reader.isStartElement()) {
                QString name = reader.name().toString();
                
                if (name == "licensePlate") {
                    m_licensePlate = XsdQt::XmlHelpers::readElementText(reader);
                }
                else if (name == "year") {
                    m_year = XsdQt::XmlHelpers::readInt(reader);
                }
                else if (name == "manufacturer") {
                    m_manufacturer = XsdQt::XmlHelpers::readElementText(reader);
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
    QString m_id;
};

class Car : public Vehicle {
public:
    Car() : m_numDoors(0), m_trunkCapacity(0.0) {}
    
    int getNumDoors() const { return m_numDoors; }
    void setNumDoors(int value) { m_numDoors = value; }
    
    double getTrunkCapacity() const { return m_trunkCapacity; }
    void setTrunkCapacity(double value) { m_trunkCapacity = value; }
    
    void toXml(QXmlStreamWriter& writer) const override {
        Vehicle::toXml(writer);
        XsdQt::XmlHelpers::writeElement(writer, "numDoors", m_numDoors);
        XsdQt::XmlHelpers::writeElement(writer, "trunkCapacity", m_trunkCapacity);
    }
    
    bool fromXml(QXmlStreamReader& reader) override {
        m_id = XsdQt::XmlHelpers::readAttribute(reader, "id");
        
        while (!reader.atEnd()) {
            reader.readNext();
            
            if (reader.isEndElement()) {
                break;
            }
            
            if (reader.isStartElement()) {
                QString name = reader.name().toString();
                
                if (name == "licensePlate") {
                    m_licensePlate = XsdQt::XmlHelpers::readElementText(reader);
                }
                else if (name == "year") {
                    m_year = XsdQt::XmlHelpers::readInt(reader);
                }
                else if (name == "manufacturer") {
                    m_manufacturer = XsdQt::XmlHelpers::readElementText(reader);
                }
                else if (name == "numDoors") {
                    m_numDoors = XsdQt::XmlHelpers::readInt(reader);
                }
                else if (name == "trunkCapacity") {
                    m_trunkCapacity = XsdQt::XmlHelpers::readDouble(reader);
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

class Fleet : public XsdQt::XmlSerializable {
public:
    Fleet() {}
    
    QString getName() const { return m_name; }
    void setName(const QString& value) { m_name = value; }
    
    const QList<QSharedPointer<Vehicle>>& getVehicles() const { return m_vehicles; }
    void addVehicle(const QSharedPointer<Vehicle>& vehicle) { m_vehicles.append(vehicle); }
    
    void toXml(QXmlStreamWriter& writer) const override {
        XsdQt::XmlHelpers::writeElement(writer, "name", m_name);
        
        for (const auto& vehicle : m_vehicles) {
            if (vehicle) {
                XsdQt::XmlHelpers::writePolymorphicElement(writer, vehicle);
            }
        }
    }
    
    bool fromXml(QXmlStreamReader& reader) override {
        while (!reader.atEnd()) {
            reader.readNext();
            
            if (reader.isEndElement()) {
                break;
            }
            
            if (reader.isStartElement()) {
                QString name = reader.name().toString();
                
                if (name == "name") {
                    m_name = XsdQt::XmlHelpers::readElementText(reader);
                }
                else if (name == "vehicle" || name == "car" || name == "truck") {
                    auto vehicle = XsdQt::XmlHelpers::readPolymorphicElement(reader);
                    if (vehicle) {
                        m_vehicles.append(vehicle.dynamicCast<Vehicle>());
                    }
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

// Register types
static XsdQt::XmlTypeRegistrar<Vehicle> vehicleReg("vehicle", "VehicleType");
static XsdQt::XmlTypeRegistrar<Car> carReg("car", "CarType");
static XsdQt::XmlTypeRegistrar<Fleet> fleetReg("fleet", "FleetType");

class TestXmlSerialization : public QObject {
    Q_OBJECT
    
private slots:
    void testSimpleVehicle();
    void testCarInheritance();
    void testFleetWithMultipleVehicles();
    void testPolymorphicDeserialization();
    void testXmlDocumentSaveLoad();
    void testAttributes();
    void testTypeConversions();
};

void TestXmlSerialization::testSimpleVehicle() {
    Vehicle vehicle;
    vehicle.setId("V001");
    vehicle.setLicensePlate("ABC-123");
    vehicle.setYear(2020);
    vehicle.setManufacturer("TestMotors");
    
    XsdQt::XmlDocument<Vehicle> doc;
    doc.setRoot(QSharedPointer<Vehicle>::create(vehicle));
    
    QString xml = doc.saveToString();
    QVERIFY(!xml.isEmpty());
    QVERIFY(xml.contains("ABC-123"));
    QVERIFY(xml.contains("2020"));
    QVERIFY(xml.contains("TestMotors"));
    
    // Load it back
    XsdQt::XmlDocument<Vehicle> doc2;
    QString errorMsg;
    bool success = doc2.loadFromString(xml, &errorMsg);
    
    QVERIFY2(success, qPrintable(errorMsg));
    QVERIFY(doc2.root());
    QCOMPARE(doc2.root()->getLicensePlate(), QString("ABC-123"));
    QCOMPARE(doc2.root()->getYear(), 2020);
    QCOMPARE(doc2.root()->getManufacturer(), QString("TestMotors"));
}

void TestXmlSerialization::testCarInheritance() {
    QSharedPointer<Car> car = QSharedPointer<Car>::create();
    car->setId("C001");
    car->setLicensePlate("XYZ-789");
    car->setYear(2021);
    car->setManufacturer("CarCorp");
    car->setNumDoors(4);
    car->setTrunkCapacity(450.5);
    
    XsdQt::XmlDocument<Car> doc;
    doc.setRoot(car);
    
    QString xml = doc.saveToString();
    QVERIFY(xml.contains("XYZ-789"));
    QVERIFY(xml.contains("4"));
    QVERIFY(xml.contains("450.5"));
    
    // Load it back
    XsdQt::XmlDocument<Car> doc2;
    QString errorMsg;
    bool success = doc2.loadFromString(xml, &errorMsg);
    
    QVERIFY2(success, qPrintable(errorMsg));
    QCOMPARE(doc2.root()->getLicensePlate(), QString("XYZ-789"));
    QCOMPARE(doc2.root()->getNumDoors(), 4);
    QCOMPARE(doc2.root()->getTrunkCapacity(), 450.5);
}

void TestXmlSerialization::testFleetWithMultipleVehicles() {
    QSharedPointer<Fleet> fleet = QSharedPointer<Fleet>::create();
    fleet->setName("Test Fleet");
    
    QSharedPointer<Vehicle> vehicle1 = QSharedPointer<Vehicle>::create();
    vehicle1->setId("V001");
    vehicle1->setLicensePlate("AAA-111");
    vehicle1->setYear(2019);
    vehicle1->setManufacturer("Maker1");
    
    QSharedPointer<Car> car1 = QSharedPointer<Car>::create();
    car1->setId("C001");
    car1->setLicensePlate("BBB-222");
    car1->setYear(2020);
    car1->setManufacturer("Maker2");
    car1->setNumDoors(2);
    car1->setTrunkCapacity(300.0);
    
    fleet->addVehicle(vehicle1);
    fleet->addVehicle(car1);
    
    XsdQt::XmlDocument<Fleet> doc;
    doc.setRoot(fleet);
    
    QString xml = doc.saveToString();
    QVERIFY(xml.contains("Test Fleet"));
    QVERIFY(xml.contains("AAA-111"));
    QVERIFY(xml.contains("BBB-222"));
    
    // Load it back
    XsdQt::XmlDocument<Fleet> doc2;
    QString errorMsg;
    bool success = doc2.loadFromString(xml, &errorMsg);
    
    QVERIFY2(success, qPrintable(errorMsg));
    QCOMPARE(doc2.root()->getName(), QString("Test Fleet"));
    QCOMPARE(doc2.root()->getVehicles().size(), 2);
}

void TestXmlSerialization::testPolymorphicDeserialization() {
    QString xml = R"(<?xml version="1.0"?>
<fleet>
    <name>Mixed Fleet</name>
    <vehicle id="V001">
        <licensePlate>VVV-111</licensePlate>
        <year>2018</year>
        <manufacturer>Generic</manufacturer>
    </vehicle>
    <car id="C001">
        <licensePlate>CCC-222</licensePlate>
        <year>2020</year>
        <manufacturer>CarMaker</manufacturer>
        <numDoors>4</numDoors>
        <trunkCapacity>500.0</trunkCapacity>
    </car>
</fleet>)";
    
    XsdQt::XmlDocument<Fleet> doc;
    QString errorMsg;
    bool success = doc.loadFromString(xml, &errorMsg);
    
    QVERIFY2(success, qPrintable(errorMsg));
    QVERIFY(doc.root());
    QCOMPARE(doc.root()->getName(), QString("Mixed Fleet"));
    QCOMPARE(doc.root()->getVehicles().size(), 2);
    
    // First vehicle should be base type
    auto vehicle1 = doc.root()->getVehicles()[0];
    QVERIFY(vehicle1);
    QCOMPARE(vehicle1->getLicensePlate(), QString("VVV-111"));
    
    // Second vehicle should be Car
    auto car = doc.root()->getVehicles()[1].dynamicCast<Car>();
    QVERIFY(car);
    QCOMPARE(car->getLicensePlate(), QString("CCC-222"));
    QCOMPARE(car->getNumDoors(), 4);
}

void TestXmlSerialization::testXmlDocumentSaveLoad() {
    // Test save to file
    QSharedPointer<Vehicle> vehicle = QSharedPointer<Vehicle>::create();
    vehicle->setId("V123");
    vehicle->setLicensePlate("TEST-001");
    vehicle->setYear(2022);
    vehicle->setManufacturer("TestCo");
    
    XsdQt::XmlDocument<Vehicle> doc;
    doc.setRoot(vehicle);
    
    QString tempFile = QDir::temp().filePath("test_vehicle.xml");
    QString errorMsg;
    bool success = doc.saveToFile(tempFile, &errorMsg);
    QVERIFY2(success, qPrintable(errorMsg));
    
    // Load from file
    XsdQt::XmlDocument<Vehicle> doc2;
    success = doc2.loadFromFile(tempFile, &errorMsg);
    QVERIFY2(success, qPrintable(errorMsg));
    
    QCOMPARE(doc2.root()->getId(), QString("V123"));
    QCOMPARE(doc2.root()->getLicensePlate(), QString("TEST-001"));
    
    // Cleanup
    QFile::remove(tempFile);
}

void TestXmlSerialization::testAttributes() {
    QString xml = R"(<?xml version="1.0"?>
<vehicle id="ATTR-001">
    <licensePlate>ATTR-123</licensePlate>
    <year>2023</year>
    <manufacturer>AttrTest</manufacturer>
</vehicle>)";
    
    XsdQt::XmlDocument<Vehicle> doc;
    QString errorMsg;
    bool success = doc.loadFromString(xml, &errorMsg);
    
    QVERIFY2(success, qPrintable(errorMsg));
    QCOMPARE(doc.root()->getId(), QString("ATTR-001"));
    QCOMPARE(doc.root()->getLicensePlate(), QString("ATTR-123"));
}

void TestXmlSerialization::testTypeConversions() {
    // Test various XSD type conversions
    QString xml = R"(<?xml version="1.0"?>
<car id="TYPE-001">
    <licensePlate>TYPE-123</licensePlate>
    <year>2024</year>
    <manufacturer>TypeTest</manufacturer>
    <numDoors>5</numDoors>
    <trunkCapacity>678.25</trunkCapacity>
</car>)";
    
    XsdQt::XmlDocument<Car> doc;
    QString errorMsg;
    bool success = doc.loadFromString(xml, &errorMsg);
    
    QVERIFY2(success, qPrintable(errorMsg));
    
    // Verify integer conversion
    QCOMPARE(doc.root()->getYear(), 2024);
    QCOMPARE(doc.root()->getNumDoors(), 5);
    
    // Verify double conversion
    QCOMPARE(doc.root()->getTrunkCapacity(), 678.25);
    
    // Verify string conversion
    QCOMPARE(doc.root()->getManufacturer(), QString("TypeTest"));
}

QTEST_MAIN(TestXmlSerialization)
#include "TestXmlSerialization.moc"
