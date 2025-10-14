QT += testlib xml
QT -= gui

TARGET = test_xsdqt
CONFIG += qt console warn_on depend_includepath testcase c++14
CONFIG -= app_bundle

TEMPLATE = app

# Include runtime library headers
INCLUDEPATH += ../runtime

# Link against runtime library
LIBS += -L.. -lxsdqt-runtime

SOURCES += \
    TestXmlSerialization.cpp

HEADERS +=
